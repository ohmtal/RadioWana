//-----------------------------------------------------------------------------
// Copyright (c) 2012 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once
#include <curl/curl.h>
#include "errorlog.h"
#include "CurlGlue.h"
#include "StreamInfo.h"

#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <functional>
#include <map>



namespace RadioWana {


    class HttpStream {
    private:
        std::string mUrl;
        std::atomic<bool> mStopRequested{false}; // remember : .store .load  :P
        std::thread mThread;
        std::atomic<bool> mRunning = false;
        StreamInfo mStreamInfo;

        std::string mFullHeader;
        std::mutex mHeaderMutex;


        enum class StreamState { AUDIO, META_LENGTH, META_DATA };
        uint32_t mMetaInt = 0;
        uint32_t mMetaCounter = 0;
        std::string mMetaString = "";
        std::string mSongTitle = "";
        StreamState mState = StreamState::AUDIO;
        size_t mBytesToRead = mMetaInt;
        size_t mCurrentMetaSize = 0;

        CURL* mCurlHandle = nullptr;


    public:
        std::function<void()> OnConnected = nullptr;
        std::function<void()> onDisConnected = nullptr;

        std::function<void(std::string)> OnSongUpdate = nullptr;
        std::function<void(const void*, size_t)> OnAudioChunk = nullptr;



        HttpStream() = default;
        ~HttpStream() {
            stop();
            if (mThread.joinable()) mThread.join();
        }

        bool isRunning() {return mRunning.load();}
        StreamInfo* getStreamInfo() { return &mStreamInfo; };
        void dumpInfo() { mStreamInfo.dump(); }



        void stop() {
            mStopRequested.store( true ) ;
        };


    private:
        static size_t HeaderCallback(char* buffer, size_t size, size_t nitems, void* userdata) {
            size_t totalSize = size * nitems;
            auto* self = static_cast<HttpStream*>(userdata);
            if (!self) {
                Log("[error]HeaderCallback self in nil!!! ");
                return totalSize;
            }

            std::string header(buffer, totalSize);
            {
                std::lock_guard<std::mutex> lock(self->mHeaderMutex);
                self->mFullHeader += header;
            }

            if (header.find("icy-metaint:") == 0) {
                size_t pos = header.find(":");
                self->mMetaInt = std::atol(header.substr(pos + 1).c_str());
                // self->mBytesToRead = self->mMetaInt;
                dLog("[info] https got header: %s (metaint:%d)", header.c_str(), self->mMetaInt);

            } else if (header == "\r\n" || header == "\n") {

                long http_code = 0;
                if (self->mCurlHandle) {
                    curl_easy_getinfo(self->mCurlHandle, CURLINFO_RESPONSE_CODE, &http_code);
                    if (http_code == 200) {
                        self->mState = StreamState::AUDIO;
                        self->mBytesToRead = self->mMetaInt;

                        if (self->OnConnected) self->OnConnected();
                    }
                }
                dLog("[warn] https got header: %s (metaint:%d) HTTP CODE: %d", header.c_str(), self->mMetaInt, http_code);

            } else {
                dLog("[info] https got header: %s", header.c_str());
                self->mStreamInfo.parseHeader(header);

            }


            return totalSize;
        }

        void parseMeta() {
            if (mMetaString.empty()) return;

            // dLog("[warn] META DATA: %s", mMetaString.c_str());

            size_t start = mMetaString.find("StreamTitle='");
            if (start != std::string::npos) {
                start += 13;
                size_t end = mMetaString.find("';", start);
                if (end != std::string::npos) {
                    mSongTitle = mMetaString.substr(start, end - start);
                    Log("[info] New Song: %s", mSongTitle.c_str());
                    if (OnSongUpdate) OnSongUpdate( mSongTitle);
                }
            }

        }



        static size_t WriteCallback(void* buffer, size_t size, size_t nmemb, void* userdata) {
            auto* self = static_cast<HttpStream*>(userdata);
            size_t totalSize = size * nmemb;

            const uint8_t* data = static_cast<const uint8_t*>(buffer);
            size_t handled = 0;

            while (handled < totalSize) {
                if (self->mState == StreamState::AUDIO) {
                    size_t canRead = std::min(self->mBytesToRead, totalSize - handled);

                    //AUDIO CALLBACK .....
                    if ( self->OnAudioChunk ) self->OnAudioChunk(data + handled, canRead);

                    self->mBytesToRead -= canRead;
                    handled += canRead;
                    if (self->mBytesToRead == 0) {
                        self->mMetaString.clear();
                        self->mState = StreamState::META_LENGTH;
                    }
                }
                else if (self->mState == StreamState::META_LENGTH) {
                    // Log("Switching to META_LENGTH at total handled: %llu", self->mTotalBytesProcessed);

                    self->mCurrentMetaSize = data[handled] * 16;
                    handled++;
                    if (self->mCurrentMetaSize > 0) {
                        self->mState = StreamState::META_DATA;
                        self->mBytesToRead = self->mCurrentMetaSize;
                    } else {
                        self->mState = StreamState::AUDIO; // no meta data avail
                        self->mBytesToRead = self->mMetaInt;
                    }
                }
                else if (self->mState == StreamState::META_DATA) {
                    size_t canRead = std::min(self->mBytesToRead, totalSize - handled);
                    self->mMetaString.append((char*)data + handled, canRead);

                    self->mBytesToRead -= canRead;
                    handled += canRead;
                    if (self->mBytesToRead == 0) {

                        // METADATA (JSON/Text)
                        self->parseMeta();

                        self->mState = StreamState::AUDIO;
                        self->mBytesToRead = self->mMetaInt;
                    }
                }
            }
            return totalSize;
        }


        static int ProgressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
            std::atomic<bool>* stopRequested = static_cast<std::atomic<bool>*>(clientp);
            if (stopRequested && stopRequested->load()) {
                return 1;
            }
            return 0; // 0 ==> continue
        }

        void reset() {
            mMetaInt  = 0;
            mStreamInfo = StreamInfo();
            mCurlHandle = nullptr;
            mFullHeader = "";
        }

    public:
        void Execute(std::string url = "") {
            if (mRunning.load()) {
                Log("Request is running - stop first ! ");
            }
            reset();

            if (mThread.joinable()) {
                mStopRequested.store(true);
                mThread.join();
            }
            if (!url.empty()) mUrl = url; //overwrite by parameter
            mStopRequested.store(false);
            initCurl();

            mThread = std::thread([this]() {
                mRunning.store(true);
                struct curl_slist *headers = NULL;
                headers = curl_slist_append(headers, "Icy-MetaData: 1");

                CURLcode res;
                mCurlHandle = curl_easy_init();
                if(mCurlHandle) {

                    curl_easy_setopt(mCurlHandle, CURLOPT_URL, mUrl.c_str());
                    curl_easy_setopt(mCurlHandle, CURLOPT_USERAGENT, "RadioWana/2.0");
                    curl_easy_setopt(mCurlHandle, CURLOPT_HTTPHEADER, headers);

                    curl_easy_setopt(mCurlHandle, CURLOPT_WRITEFUNCTION, WriteCallback);
                    curl_easy_setopt(mCurlHandle, CURLOPT_WRITEDATA, this);

                    curl_easy_setopt(mCurlHandle, CURLOPT_HEADERFUNCTION, HeaderCallback);
                    curl_easy_setopt(mCurlHandle, CURLOPT_HEADERDATA, this);

                    curl_easy_setopt(mCurlHandle, CURLOPT_FOLLOWLOCATION, 1L);
                    curl_easy_setopt(mCurlHandle, CURLOPT_NOPROGRESS, 0L);
                    curl_easy_setopt(mCurlHandle, CURLOPT_XFERINFOFUNCTION, ProgressCallback);
                    curl_easy_setopt(mCurlHandle, CURLOPT_XFERINFODATA, &mStopRequested);
                    curl_easy_setopt(mCurlHandle, CURLOPT_NOSIGNAL, 1L);
                    curl_easy_setopt(mCurlHandle, CURLOPT_CONNECTTIMEOUT, 10L);

                    // SSL-Verify
                    curl_easy_setopt(mCurlHandle, CURLOPT_SSL_VERIFYPEER, 1L);
                    curl_easy_setopt(mCurlHandle, CURLOPT_SSL_VERIFYHOST, 2L);

                    // disable buffering
                    curl_easy_setopt(mCurlHandle, CURLOPT_BUFFERSIZE, 1L);

                    res = curl_easy_perform(mCurlHandle);

                    if(res != CURLE_OK && res != CURLE_ABORTED_BY_CALLBACK) {
                        Log("[error] HttpStream error: %s", curl_easy_strerror(res));
                    }
                    curl_slist_free_all(headers);
                    curl_easy_cleanup(mCurlHandle);
                }
                stop();
                mRunning.store(false);
                if (onDisConnected) onDisConnected();
            });
        }
    }; //class


}
