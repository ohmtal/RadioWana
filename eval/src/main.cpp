//-----------------------------------------------------------------------------
// Copyright (c) 2012 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// RadioWanaEval
//-----------------------------------------------------------------------------

#include <SDL3/SDL_main.h>
#include "imgui.h"

#include <BaseFlux.h>
#include <ImConsole.h>

#include <imgui.h>
#include <miniaudio.h>
#include <httplib.h>

#include <iostream>
#include <thread>

// -----------------------------------------------------------------------------
// Url parser ... still can't believe httplib have no url parser ...
// -----------------------------------------------------------------------------

struct URLParts {
    std::string protocol;
    std::string hostname;
    std::string path;
};

URLParts parseURL(const std::string& url) {
    URLParts parts;

    size_t pos = url.find("://");
    if (pos != std::string::npos) {
        parts.protocol = url.substr(0, pos);
        pos += 3;
    } else {
        pos = 0;
    }

    size_t pathStart = url.find('/', pos);
    if (pathStart != std::string::npos) {
        parts.hostname = url.substr(pos, pathStart - pos);
        parts.path = url.substr(pathStart);
    } else {
        parts.hostname = url.substr(pos);
        parts.path = "/";
    }

    return parts;
}

// -----------------------------------------------------------------------------
// RadioWanaEval
// -----------------------------------------------------------------------------
void SDLCALL ConsoleLogFunction(void *userdata, int category, SDL_LogPriority priority, const char *message); //FWD

class RadioWanaEval
{


private:

     // std::string mUrl = "https://streams.radiobob.de/powermetal/mp3-192/streams.radiobob.de/";
     std::string mUrl = "https://stream.rockantenne.de/rockantenne/stream/mp3";
     bool mStreamOpen = false;

     void OnConsoleCommand(ImConsole* console, const char* cmdline) {}

     BaseFlux::Main mBaseFlux;
public:
    ImConsole mConsole;
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
    }
    //--------------------------------------------------------------------------
    // ..... Stream .....
    //--------------------------------------------------------------------------

    bool closeStream() { return true; }

    bool openStream() {
        mStreamOpen = false;

         URLParts parts = parseURL(mUrl);

         dLog("proto =>  %s  host => %s ::: path => %s", parts.protocol.c_str(), parts.hostname.c_str(), parts.path.c_str());


        // httplib::SSLClient cli(parts.protocol +  parts.hostname); // => [error] Could not establish connection
        httplib::SSLClient cli(parts.hostname); // => [error] Failed to read connection
        cli.enable_server_certificate_verification(true);

        // httplib::Client cli(parts.protocol + parts.hostname); //=> [error] Could not establish connection

        cli.set_connection_timeout(60);
        cli.set_follow_location(true);



        httplib::Headers headers = {
            {"Icy-MetaData", "1"},
            {"User-Agent", "RadioWana/2.0"}
            // {"User-Agent", "Mozilla/5.0"}
        };

        // Stream asynchron
        auto res = cli.Get(parts.path, headers,
                           [&](const httplib::Response &response) {
                               if (response.status == 200) {
                                   mStreamOpen = true;
                                   // FIXME Content-Type (audio/mpeg )

                                   return true;
                               }
                               dLog("HTTP Response: %d", response.status);
                               return false;
                           },
                           [&](const char *data, size_t data_length) {
                               dLog("got data %d", (int)data_length);

                               //FIXME handle data chunks
                               // audio data
                               // ICY - Meta Data
                               return true;
                           }
        );

        if (!res) {
            auto err = res.error();
            dLog("[error] %s", httplib::to_string(err).c_str());
            return false;
        }

        return mStreamOpen;
    }
    //--------------------------------------------------------------------------
    void DrawMainWindow() {
        ImGui::SetNextWindowSizeConstraints(ImVec2(800.0f, 600.f), ImVec2(FLT_MAX, FLT_MAX));
        if (ImGui::Begin("RadioWana")) {
            // ImGui::SetNextItemWidth(450.f);
            char urlBuff[256];
            strncpy(urlBuff, mUrl.c_str(), sizeof(urlBuff));
            if (ImGui::InputText("URL", urlBuff, sizeof(urlBuff))) {
                mUrl = urlBuff;
            }
            if ( mStreamOpen ) {
                if (ImGui::Button("close")) {
                    closeStream();
                }
            } else {
                if (ImGui::Button("open")) {
                    openStream();
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

