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
#include "gui/ImFlux.h"
#include "utils/errorlog.h"

#include <imgui.h>

#include "CurlGlue.h"
#include "StreamHandler.h"
#include "AudioHandler.h"
#include "StreamInfo.h"
#include "AudioRecorder.h"
#include "RadioBrowser.h"



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


    void OnConsoleCommand(ImConsole* console, const char* cmdline) {
         std::string cmd = fluxStr::getWord(cmdline,0);
         std::string params = fluxStr::removeWord(cmdline,0);
         if (cmd == "addslash") {
             Log("result: %s", fluxStr::addTrailingSlash(params).c_str());
         }

    }

     BaseFlux::Main mBaseFlux;


     std::unique_ptr<FluxRadio::StreamHandler> mStreamHandler;
     std::unique_ptr<FluxRadio::AudioHandler>  mAudioHandler;
     std::unique_ptr<FluxRadio::AudioRecorder> mAudioRecorder;
     std::unique_ptr<FluxRadio::RadioBrowser> mRadioBrowser;

     bool mRecording = false;
     bool mRecordingStartsOnNewTile = true;

     std::vector<FluxRadio::RadioStation> mStationData;
     std::string mSelectedStationUuid = "";
public:
    ImConsole mConsole;
    //--------------------------------------------------------------------------
    bool Initialize()   {

        mBaseFlux.mSettings = {
            .FpsLimit = 30, // lower cpu usage...
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

        mStreamHandler = std::make_unique<FluxRadio::StreamHandler>();
        mAudioHandler  = std::make_unique<FluxRadio::AudioHandler>();
        mAudioRecorder = std::make_unique<FluxRadio::AudioRecorder>();
        mRadioBrowser  = std::make_unique<FluxRadio::RadioBrowser>();



        mStreamHandler->OnConnected = [&]() {
            if (mAudioHandler.get())  mAudioHandler->init(mStreamHandler->getStreamInfo());
            mStreamHandler->dumpInfo();

        };
        mStreamHandler->OnStreamTitleUpdate = [&](std::string title, size_t streamPosition) {
            if (mAudioHandler.get()) mAudioHandler->OnStreamTitleUpdate(title, streamPosition);
        };
        mStreamHandler->OnAudioChunk = [&](const void* buffer , size_t size) {  mAudioHandler->OnAudioChunk(buffer, size); };
        mStreamHandler->onDisConnected = [&]() {
            if (mAudioHandler.get()) mAudioHandler->onDisConnected();
            if (mAudioRecorder.get()) mAudioRecorder->closeFile();
            mRecording = false;
        };

        mAudioHandler->OnTitleTrigger = [&]() {
            Log("Streamtitle %s", mAudioHandler->getCurrentTitle().c_str());
            //FIXME toggle delay ... needed for some stations
            if (mRecording && mAudioRecorder.get()) mAudioRecorder->openFile(mAudioHandler->getCurrentTitle());
        };

        mAudioHandler->OnAudioStreamData = [&](const uint8_t* buffer, size_t bufferSize)  {
            if (mAudioRecorder.get() && mAudioRecorder->isFileOpen()) {
                mAudioRecorder->OnStreamData(buffer,bufferSize);
            }
        };


        mRadioBrowser->OnStationResponse = [&](std::vector<FluxRadio::RadioStation> stations) {
            mStationData = stations;
        };

        mRadioBrowser->OnStationResponseError = [&]() {
                mStationData.clear();
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
        mAudioHandler->shutDown();
        mStreamHandler->stop();
        // SDL_Delay(1000);

        FluxNet::shutdownCurl();
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
            char strBuff[256];
            strncpy(strBuff, mUrl.c_str(), sizeof(strBuff));
            if (ImGui::InputText("URL", strBuff, sizeof(strBuff))) {
                mUrl = strBuff;
            }
            if ( mStreamHandler->isRunning() ) {
                if (ImFlux::ButtonFancy("close")) {
                    mStreamHandler->stop();
                }
                FluxRadio::StreamInfo* info = mStreamHandler->getStreamInfo();
                if (info)
                {
                    // inline void LCDText(std::string text, int display_chars, float height, ImU32 color_on, bool scroll = true, float scroll_speed = 2.0f) {

                    ImFlux::LCDText(mAudioHandler->getCurrentTitle(), 20, 36.f, ImFlux::COL32_NEON_ORANGE);
                    ImGui::SeparatorText(info->streamUrl.c_str());
                    ImGui::SeparatorText(info->name.c_str());
                    ImGui::TextColored(ImVec4(0.3f, 0.3f,0.7f,1.f), "%s", mAudioHandler->getCurrentTitle().c_str());
                    ImGui::TextDisabled("Next: %s", mAudioHandler->getNextTitle().c_str());
                    ImGui::Separator();
                    ImGui::Text("Description: %s", info->description.c_str());
                    ImGui::Text("Audio: %d Hz, %d kbps, %d Channels", info->samplerate, info->bitrate, info->channels);
                    ImGui::Text("Url: %s", info->url.c_str());
                }

                float vol = mAudioHandler->getVolume();
                if (ImFlux::LEDMiniKnob("Volume", &vol, 0.f, 1.f, vol_ks)) {
                    mAudioHandler->setVolume(vol);
                }
                ImFlux::SameLineBreak(200);

                if ( mAudioHandler->getManager() && mAudioHandler->getManager()->getVisualAnalyzer()) {
                        mAudioHandler->getManager()->getVisualAnalyzer()->renderVU(ImVec2(200,60), 70);

//FIXME separate VU's:
//                         float dbL, dbR;
//                         dbL = this->getDecible(0);
//                         dbR = this->getDecible(1);
//
//                         // this->getDecible(dbL, dbR);
//
//                         auto mapDB = [](float db) {
//                             float minDB = -20.0f;
//                             return (db < minDB) ? 0.0f : (db - minDB) / (0.0f - minDB);
//                         };
//
//                         ImFlux::VUMeter70th(halfSize, mapDB(dbL));
//                         ImGui::SameLine();
//                         ImFlux::VUMeter70th(halfSize, mapDB(dbR));


                }
                if ( mAudioHandler->getManager() && mAudioHandler->getManager()->getSpectrumAnalyzer()) {

                    mAudioHandler->getManager()->getSpectrumAnalyzer()->DrawSpectrumAnalyzer(ImVec2(fullWidth,60), true);
                }

                mAudioHandler->RenderRack(1);

                ImGui::SeparatorText("Recording");

                ImGui::Checkbox("Recording starts on when new stream title is triggered", &mRecordingStartsOnNewTile);
                if (ImFlux::LEDCheckBox("Enable Recording", &mRecording, ImVec4(0.8f,0.3f,0.3f,1.f))) {
                    if (mRecording && !mRecordingStartsOnNewTile && !mAudioHandler->getCurrentTitle().empty()) {
                        mAudioRecorder->openFile(mAudioHandler->getCurrentTitle());
                    }
                    if (!mRecording)
                        mAudioRecorder->closeFile();
                }
                if (mRecording) {
                    ImFlux::DrawLED("Recording", mAudioRecorder->isFileOpen(), ImFlux::LED_GREEN_ANIMATED_GLOW);
                    ImGui::SameLine();
                    ImGui::Text("File: %s", mAudioRecorder->getCurrentFilename().c_str());
                }

            } else {
                if (ImFlux::ButtonFancy("open URL")) {
                    mStreamHandler->Execute(mUrl);
                }
            }

            ImGui::SeparatorText("radio-browser.info");
            strncpy(strBuff, "", sizeof(strBuff));

            if (ImGui::InputText("Name:", strBuff, sizeof(strBuff), ImGuiInputTextFlags_EnterReturnsTrue)) {
                //FIXME hardcoded metal :P
                mRadioBrowser->searchStationsByNameAndTag(strBuff, "");
                dLog("query for %s", strBuff);
            }

            if (ImGui::BeginTable("RadioStations", 5, ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable)) {


                ImGui::TableSetupColumn("Station");
                ImGui::TableSetupColumn("Homepage");
                // std::string favicon; << TODO would be cool but
                // ImGui::TableSetupColumn("Codec"); //we can mp3 only at the moment
                ImGui::TableSetupColumn("Clicks");
                ImGui::TableSetupColumn("Country");
                ImGui::TableSetupColumn("Bitrate");
                ImGui::TableHeadersRow();

                for (const auto& station : mStationData) {
                    ImGui::PushID(station.stationuuid.c_str());
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();

                    bool isSelected = (mSelectedStationUuid == station.stationuuid);



                    if (ImGui::Selectable(station.name.c_str(), isSelected,
                        ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick)) {

                        //FIXME favo or save last
                        mSelectedStationUuid = station.stationuuid;

                        if (ImGui::IsMouseDoubleClicked(0)) {
                            mUrl = station.url;
                            mStreamHandler->Execute(mUrl);
                            mRadioBrowser->clickStation(station.stationuuid);
                        }
                    }

                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted(station.homepage.c_str());

                    // ImGui::TableNextColumn();
                    // ImGui::TextUnformatted(station.Codec.c_str());

                    ImGui::TableNextColumn();
                    ImGui::Text("%d  Trend:%d",station.clickcount, station.clicktrend);

                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted(station.countrycode.c_str());

                    ImGui::TableNextColumn();
                    if (station.bitrate >= 128) {
                        ImGui::TextColored(ImColor(0, 255, 0), "%d kbps", station.bitrate);
                    } else if (station.bitrate >= 64) {
                        ImGui::TextColored(ImColor(255, 255, 0), "%d kbps", station.bitrate);
                    } else if (station.bitrate == 0) {
                        ImGui::TextDisabled("---");
                    } else {
                        ImGui::TextColored(ImColor(255, 0, 0), "%d kbps", station.bitrate);
                    }

                    ImGui::PopID(/*station.stationuuid.c_str()*/);
                }
                ImGui::EndTable();
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
    gui->mConsole.AddLog("%s", fluxStr::removePart(message,"\r\n").c_str());
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

