#include <SDL3/SDL.h>
#include "AudioHandler.h"
#include <errorlog.h>
#include <mutex>

#define MA_NO_DEVICE_IO
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>


namespace RadioWana {
    // -----------------------------------------------------------------------------

    void SDLCALL AudioHandler::audio_callback(void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount) {
        auto* self = static_cast<AudioHandler*>(userdata);

        int bytesPerFrame = sizeof(float) * self->mStreamInfo->channels;
        ma_uint32 framesToRead = additional_amount / bytesPerFrame;

        //  PCM-Data
        std::vector<float> pcmBuffer(framesToRead * self->mStreamInfo->channels);

        // miniaudio
        ma_uint64 framesRead;
        ma_result result = ma_decoder_read_pcm_frames(self->mDecoder, pcmBuffer.data(), framesToRead, &framesRead);

        // 4. Daten an SDL übergeben
        if (framesRead > 0) {
            SDL_PutAudioStreamData(stream, pcmBuffer.data(), (int)(framesRead * bytesPerFrame));
        }

        if (framesRead < framesToRead) {
            int silenceBytes = (framesToRead - (int)framesRead) * bytesPerFrame;
            std::vector<uint8_t> silence(silenceBytes, 0);
            SDL_PutAudioStreamData(stream, silence.data(), silenceBytes);
        }
    }

    // -----------------------------------------------------------------------------
    bool AudioHandler::init(StreamInfo* info) {
        if (!info) {
            Log("[error] init StreamInfo in NULL!!");
        }
        mStreamInfo = info;

        // FIXME reconnect [error] Unable to init decoder code:-203


        if (mInitialized ) {
            ma_decoder_uninit(mDecoder);
            mDecoderInitialized = false;
            mInitialized = false;
        }

        if ( !mDecoderInitialized )
            return false;


        if (mDecoder == nullptr) {
            dLog("mDecoder was nullptr?! ");
            mDecoder = new ma_decoder();
        }
        memset(mDecoder, 0, sizeof(ma_decoder));
        ma_decoder_config config = ma_decoder_config_init(ma_format_f32, info->channels, info->samplerate);
        config.encodingFormat = ma_encoding_format_mp3;
        dLog("this: %p, pDecoder: %p (Inhalt)", this, (void*)mDecoder);
        mDecoder->pUserData = this;
        ma_result result = ma_decoder_init(OnReadFromRawBuffer, OnSeekDummy, this, &config, mDecoder);

        if (result != MA_SUCCESS) {
            Log("[error] Unable to init decoder code:%d", result);
            return false;
        }

        bool needsNewSDLStream = true;
        if (result == MA_SUCCESS) {

            if (mStream) {
                SDL_AudioSpec currentSpec;
                if (SDL_GetAudioStreamFormat(mStream, &currentSpec, nullptr)) {
                    if (currentSpec.freq == info->samplerate && currentSpec.channels == info->channels) {
                        needsNewSDLStream = false;
                        SDL_ResumeAudioStreamDevice(mStream);
                    }
                }
            }
        }


        if (needsNewSDLStream && result == MA_SUCCESS) {

            if (mStream) {
                SDL_DestroyAudioStream(mStream);
                mStream = nullptr;
            }


            SDL_AudioSpec spec;
            spec.format = SDL_AUDIO_F32;
            spec.channels = 2;
            spec.freq =  44100 ;
            mStream = SDL_CreateAudioStream(&spec, &spec);


            if (!mStream)
            {
                Log("[error] SDL_OpenAudioDeviceStream failed: %s", SDL_GetError());
                return false;
            }

            SDL_SetAudioStreamGetCallback(mStream, AudioHandler::audio_callback, this);

            #ifdef FLUX_ENGINE
            AudioManager.bindStream(mStream);
            #else
            SDL_AudioDeviceID dev = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);

            if (dev == 0) {
                Log("[error] Failed to open audio device: %s", SDL_GetError());
                return false;
            }

            if (!SDL_BindAudioStream(dev, mStream)) {
                Log("[error] SDL_OpenAudioDeviceStream failed to bind stream: %s", SDL_GetError());
                return false;
            }
            #endif

            SDL_ResumeAudioStreamDevice(mStream);

        }
        mInitialized = true;
        return true;
    }
    // -----------------------------------------------------------------------------
    void AudioHandler::OnAudioChunk(const void* buffer, size_t size) {
          std::lock_guard<std::recursive_mutex> lock(mBufferMutex);
        mRawBuffer.insert(mRawBuffer.end(), (uint8_t*)buffer, (uint8_t*)buffer + size);

        if (mStreamInfo && !mDecoderInitialized && mRawBuffer.size() >= 16384) {
            mDecoderInitialized = true;
            this->init(mStreamInfo);
        }
    }

    // void AudioHandler::OnAudioChunk(const void* buffer, size_t size) {
    //     std::lock_guard<std::mutex> lock(mBufferMutex);
    //     const uint8_t* pData = static_cast<const uint8_t*>(buffer);
    //     mRawBuffer.insert(mRawBuffer.end(), pData, pData + size);
    //
    //     // if (!mDecoderInitialized && mRawBuffer.size() > 8192) {
    //     //     InitMiniaudio();
    //     // }
    // }
    // -----------------------------------------------------------------------------
    // Muss exakt 4 Argumente haben, wenn pUserData separat übergeben wird:

    ma_result AudioHandler::OnReadFromRawBuffer(ma_decoder* pDecoder, void* pBufferOut, size_t bytesToRead, size_t* pBytesRead)
    {
        auto* self = static_cast<AudioHandler*>(pDecoder->pUserData);
        if (!self || !pBufferOut) {
            return MA_ERROR;
        }

        std::lock_guard<std::recursive_mutex> lock(self->mBufferMutex);

        size_t available = self->mRawBuffer.size();
        size_t actualRead = std::min(bytesToRead, available);

        if (actualRead > 0) {
            memcpy(pBufferOut, self->mRawBuffer.data(), actualRead);
            self->mRawBuffer.erase(self->mRawBuffer.begin(), self->mRawBuffer.begin() + actualRead);
        }

        if (pBytesRead) {
            *pBytesRead = actualRead;
        }

        return MA_SUCCESS;
    }

    // -----------------------------------------------------------------------------
    ma_result AudioHandler::OnSeekDummy(ma_decoder* pDecoder, ma_int64 byteOffset, ma_seek_origin origin){
        return MA_SUCCESS;
    }
    // -----------------------------------------------------------------------------
    void AudioHandler::onDisConnected(){
        std::lock_guard<std::recursive_mutex> lock(mBufferMutex);
        mRawBuffer.clear();
        mDecoderInitialized = false;
        if (mStream) {
            SDL_PauseAudioStreamDevice(mStream);
        }
        if (mDecoder) {

            ma_decoder_uninit(mDecoder);
            memset(mDecoder, 0, sizeof(ma_decoder));
            mInitialized = false;
        }
    }

};

