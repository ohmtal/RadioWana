//-----------------------------------------------------------------------------
// Copyright (c) 2012 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// RadioWanaEval
//-----------------------------------------------------------------------------

#include <SDL3/SDL_main.h>
#include <imgui.h>

#include <BaseFlux.h>
#include "gui/ImConsole.h"
#include "utils/errorlog.h"

#include <imgui.h>

#include "CurlGlue.h"
#include "StreamHandler.h"
#include "AudioHandler.h"
#include "StreamInfo.h"
#include "gui/ImFlux.h"

// -----------------------------------------------------------------------------
// RadioWanaEval
// -----------------------------------------------------------------------------
void SDLCALL ConsoleLogFunction(void *userdata, int category, SDL_LogPriority priority, const char *message); //FWD

class RadioWanaEval
{


private:
    // std::string mUrl = "https://streams.radiobob.de/powermetal/mp3-192/streams.radiobob.de/";
    std::string mUrl = "https://stream.rockantenne.de/rockantenne/stream/mp3";
    // std::string mUrl = "http://yourip.chatwana.net/";


     void OnConsoleCommand(ImConsole* console, const char* cmdline) {}
     BaseFlux::Main mBaseFlux;

public:
    ImConsole mConsole;
    RadioWana::StreamHandler mStreamHandler;
    RadioWana::AudioHandler mAudioHandler;
    //--------------------------------------------------------------------------
    bool Initialize()   {

        mBaseFlux.mSettings = {
            .WindowMaximized = true,
            .Company = "Ohmtal",
            .Caption = "RadioWana Prototype",
            // IconFilename is append to .AssetsPath
            // .IconFilename = "icon.bmp",
            // IniFileName : you may also use something like: "pref:/appgui.ini"
            //               where pref:/ is replaced with your prefs path build be
            //               Company and Caption
            //               If empty no ini file is written
            .IniFileName = "RadioWanaEval.ini",
        };

        if ( !mBaseFlux.InitSDL() ) return false;
        mBaseFlux.initImGui();

        mConsole.OnCommand =  [&](ImConsole* console, const char* cmd) { OnConsoleCommand(console, cmd); };
        SDL_SetLogOutputFunction(ConsoleLogFunction, this);
        mBaseFlux.OnRender = [&](SDL_Renderer* renderer) { OnRender(renderer);};
        mBaseFlux.OnEvent  = [&](const SDL_Event event) { OnEvent(event);};
        mBaseFlux.OnShutDown = [&]() { OnShutDown();};

        InitErrorLog("RadioWanaEval.log", "RadioWanaEval", "1");


        mStreamHandler.OnConnected = [&]() {
            mAudioHandler.init(mStreamHandler.getStreamInfo());
            mStreamHandler.dumpInfo();

        };
        mStreamHandler.OnStreamTitleUpdate = [&](std::string title, size_t streamPosition)
            {  mAudioHandler.OnStreamTitleUpdate(title, streamPosition); };
        mStreamHandler.OnAudioChunk = [&](const void* buffer , size_t size) {  mAudioHandler.OnAudioChunk(buffer, size); };
        mStreamHandler.onDisConnected = [&]() {  mAudioHandler.onDisConnected(); };

        mAudioHandler.OnTitleTrigger = [&]() {
            Log("[warn]FIXME Streamtitle TRIGGER!!! %s", mAudioHandler.getCurrentTitle().c_str());
        };

        return true;

    }
    //--------------------------------------------------------------------------
    void OnRender(SDL_Renderer* renderer) {
        DrawMainWindow();
        bool foo;
        mConsole.Draw("Console", &foo);

    }
    //--------------------------------------------------------------------------
    void OnEvent(const SDL_Event event) {}
    void OnShutDown() {
        SDL_SetLogOutputFunction(nullptr, nullptr); // log must be unlinked first!!
        mAudioHandler.shutDown();
        mStreamHandler.stop();
        // SDL_Delay(1000);

        RadioWana::shutdownCurl();
    }
    //--------------------------------------------------------------------------
    // ..... Stream .....
    //--------------------------------------------------------------------------
    const ImFlux::KnobSettings vol_ks =  ImFlux::ksBlack; // ImFlux::DARK_KNOB.WithRadius(24.f);

    void DrawMainWindow() {
        // ImGui::SetNextWindowSizeConstraints(ImVec2(800.0f, 600.f), ImVec2(FLT_MAX, FLT_MAX));
        if (ImGui::Begin("RadioWana")) {
            float fullWidth = ImGui::GetContentRegionAvail().x;

            // ImGui::SetNextItemWidth(450.f);
            char urlBuff[256];
            strncpy(urlBuff, mUrl.c_str(), sizeof(urlBuff));
            if (ImGui::InputText("URL", urlBuff, sizeof(urlBuff))) {
                mUrl = urlBuff;
            }
            if ( mStreamHandler.isRunning() ) {
                if (ImFlux::ButtonFancy("close")) {
                    mStreamHandler.stop();
                }
                RadioWana::StreamInfo* info = mStreamHandler.getStreamInfo();
                if (info)
                {
                    // inline void LCDText(std::string text, int display_chars, float height, ImU32 color_on, bool scroll = true, float scroll_speed = 2.0f) {

                    ImFlux::LCDText(mAudioHandler.getCurrentTitle(), 30, 36.f, ImFlux::COL32_NEON_ORANGE);
                    ImGui::SeparatorText(info->streamUrl.c_str());
                    ImGui::SeparatorText(info->name.c_str());
                    ImGui::TextColored(ImVec4(0.3f, 0.3f,0.7f,1.f), "%s", mAudioHandler.getCurrentTitle().c_str());
                    ImGui::TextDisabled("Next: %s", mAudioHandler.getNextTitle().c_str());
                    ImGui::Separator();
                    ImGui::Text("Description: %s", info->description.c_str());
                    ImGui::Text("Audio: %d Hz, %d kbps, %d Channels", info->samplerate, info->bitrate, info->channels);
                    ImGui::Text("Url: %s", info->url.c_str());
                }

                float vol = mAudioHandler.getVolume();
                if (ImFlux::LEDMiniKnob("Volume", &vol, 0.f, 1.f, vol_ks)) {
                    mAudioHandler.setVolume(vol);
                }
                ImFlux::SameLineBreak(200);

                if ( mAudioHandler.getManager() && mAudioHandler.getManager()->getVisualAnalyzer()) {
                        mAudioHandler.getManager()->getVisualAnalyzer()->renderVU(ImVec2(200,60), 70);
                        // mAudioHandler.getManager()->getVisualAnalyzer()->DrawVisualAnalyzerOszi(ImVec2(200,80), info->channels);
                }
                if ( mAudioHandler.getManager() && mAudioHandler.getManager()->getSpectrumAnalyzer()) {

                    mAudioHandler.getManager()->getSpectrumAnalyzer()->DrawSpectrumAnalyzer(ImVec2(fullWidth,60), true);
                }

                mAudioHandler.RenderRack(1);



            } else {
                if (ImFlux::ButtonFancy("open URL")) {
                    mStreamHandler.Execute(mUrl);
                }
            }


        }
        ImGui::End();
    }

    void Execute() {
        mBaseFlux.Execute();
    }

};

// -----------------------------------------------------------------------------
// Console handling
// -----------------------------------------------------------------------------
void SDLCALL ConsoleLogFunction(void *userdata, int category, SDL_LogPriority priority, const char *message)
{
    if (!userdata) return;
    auto* gui = static_cast<RadioWanaEval*>(userdata);

    if (!gui)
        return;

    char lBuffer[1024];
    if (priority == SDL_LOG_PRIORITY_ERROR)
    {
        snprintf(lBuffer, sizeof(lBuffer), "[ERROR] %s", message);
    }
    else if (priority == SDL_LOG_PRIORITY_WARN)
    {
        snprintf(lBuffer, sizeof(lBuffer), "[WARN] %s", message);
    }
    else
    {
        snprintf(lBuffer, sizeof(lBuffer), "%s", message);
    }

    // bad if we are gone !!
    gui->mConsole.AddLog("%s", message);
}
// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    (void)argc; (void)argv;
    RadioWanaEval app;
    if (!app.Initialize()) return 1;
    app.Execute();

    return 0;
}

