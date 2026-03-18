//-----------------------------------------------------------------------------
// Copyright (c) 2012 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once

#include <SDL3/SDL.h>
#include <iostream>
#include "StreamInfo.h"
#include <miniaudio.h>
#include <vector>
#include <mutex>

namespace RadioWana {

    class AudioHandler {
    protected:
        bool mDecoderInitialized = false;
        ma_decoder* mDecoder = nullptr;
        SDL_AudioStream* mStream = nullptr;

        std::vector<uint8_t> mRawBuffer; // ringbuffer
        std::recursive_mutex mBufferMutex;

        bool mInitialized = false;
        StreamInfo* mStreamInfo = nullptr;

    public:

        AudioHandler() {
            mDecoder = new ma_decoder();
            memset(mDecoder, 0, sizeof(ma_decoder));
            mStreamInfo = nullptr;
        }

        static void SDLCALL audio_callback(void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount);

        static ma_result OnReadFromRawBuffer(ma_decoder* pDecoder, void* pBufferOut, size_t bytesToRead, size_t* pBytesRead);
        static ma_result OnSeekDummy(ma_decoder* pDecoder, ma_int64 byteOffset, ma_seek_origin origin);


        bool init(StreamInfo* info);

        void OnAudioChunk(const void* buffer, size_t size);

        void onDisConnected();

    };
}
