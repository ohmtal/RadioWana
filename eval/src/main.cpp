//-----------------------------------------------------------------------------
// Copyright (c) 2012 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// RadioWanaEval
//         {"Icy-MetaData", "1"},
//         {"User-Agent", "RadioWana/2.0"}
//-----------------------------------------------------------------------------

#include <SDL3/SDL_main.h>
#include <imgui.h>

#include <BaseFlux.h>
#include <ImConsole.h>
#include <errorlog.h>

#include <imgui.h>
#include <miniaudio.h>

#include "HttpsStream.h"




// -----------------------------------------------------------------------------
// RadioWanaEval
// -----------------------------------------------------------------------------
void SDLCALL ConsoleLogFunction(void *userdata, int category, SDL_LogPriority priority, const char *message); //FWD

class RadioWanaEval
{


private:
     std::string mUrl = "https://streams.radiobob.de/powermetal/mp3-192/streams.radiobob.de/";
     // std::string mUrl = "https://stream.rockantenne.de/rockantenne/stream/mp3";
    // std::string mUrl = "http://yourip.chatwana.net/";


     void OnConsoleCommand(ImConsole* console, const char* cmdline) {}

     BaseFlux::Main mBaseFlux;
public:
    ImConsole mConsole;
    RadioWana::HttpStream mHttpsStream;
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
        mHttpsStream.shutdownCurl();
    }
    //--------------------------------------------------------------------------
    // ..... Stream .....
    //--------------------------------------------------------------------------
    void DrawMainWindow() {
        // ImGui::SetNextWindowSizeConstraints(ImVec2(800.0f, 600.f), ImVec2(FLT_MAX, FLT_MAX));
        if (ImGui::Begin("RadioWana")) {
            // ImGui::SetNextItemWidth(450.f);
            char urlBuff[256];
            strncpy(urlBuff, mUrl.c_str(), sizeof(urlBuff));
            if (ImGui::InputText("URL", urlBuff, sizeof(urlBuff))) {
                mUrl = urlBuff;
            }
            if ( mHttpsStream.isRunning() ) {
                if (ImGui::Button("close")) {
                    mHttpsStream.stop();
                }
            } else {
                if (ImGui::Button("open URL")) {
                    mHttpsStream.Execute(mUrl);
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

