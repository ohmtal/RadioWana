//-----------------------------------------------------------------------------
// Copyright (c) 2012 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once
#include <curl/curl.h>
#include <iostream>
#include <string>
#include "errorlog.h"

#include <thread>
#include <atomic>
#include <mutex>


namespace RadioWana { //FIXME better name ?
    //         {"Icy-MetaData", "1"},
    //         {"User-Agent", "RadioWana/2.0"}

    static bool gCurlInitialized = false;

    class HttpStream {
    private:
        std::string mUrl;
        std::atomic<bool> mStopRequested{false}; // remember : .store .load  :P
        std::thread mThread;
        std::atomic<bool> mRunning = false;

    public:
        // HttpStream() { initCurl(); }
        ~HttpStream() {
            stop();
            if (mThread.joinable()) mThread.join();
        }

        bool isRunning() {return mRunning.load();}

        void initCurl() {
            if ( gCurlInitialized ) return;
            curl_global_init(CURL_GLOBAL_DEFAULT);
            gCurlInitialized = true;
        }

        void shutdownCurl( ) {
            if (!gCurlInitialized) return;
            curl_global_cleanup();
            gCurlInitialized = true;
        }



        void stop() {
            mStopRequested.store( true ) ;
        };

        static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
            size_t realsize = size * nmemb;
            dLog("[info] https got chunk: %d", (uint32_t)realsize);
            //FIXME DATA std::cout.write(static_cast<const char*>(contents), realsize);
            dLog("[info] %s", static_cast<const char*>(contents));
            return realsize;
        }

        static int ProgressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
            std::atomic<bool>* stopRequested = static_cast<std::atomic<bool>*>(clientp);
            if (stopRequested && stopRequested->load()) {
                return 1;
            }
            return 0; // 0 ==> continue
        }

        void Execute(std::string url = "") {
            if (mRunning.load()) {
                Log("Request is running - stop first ! ");
            }

            if (mThread.joinable()) {
                mStopRequested.store(true);
                mThread.join();
            }
            if (!url.empty()) mUrl = url; //overwrite by parameter
            mStopRequested.store(false);
            initCurl();

            mThread = std::thread([this]() {
                mRunning.store(true);
                CURL* curl;
                CURLcode res;
                curl = curl_easy_init();
                if(curl) {
                    curl_easy_setopt(curl, CURLOPT_URL, mUrl.c_str());
                    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
                    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
                    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
                    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, ProgressCallback);
                    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &mStopRequested);
                    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
                    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);

                    // SSL-Verify
                    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
                    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

                    // disable buffering
                    curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, 1L);

                    res = curl_easy_perform(curl);

                    if(res != CURLE_OK) {
                        Log("[error] HttpStream error: %d", curl_easy_strerror(res));
                    }
                    curl_easy_cleanup(curl);
                }
                stop();
                mRunning.store(false);
            });
        }
    }; //class


}
