//-----------------------------------------------------------------------------
// Copyright (c) 2012 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// RadioWanaEval
//-----------------------------------------------------------------------------

#include <fluxMain.h>
#include <SDL3/SDL_main.h>
#include <miniaudio.h>
#include <httplib.h>

class RadioWanaEval : public FluxMain
{
    typedef FluxMain Parent;
private:
    bool Initialize() override    { if (!Parent::Initialize()) return false;}
    void Deinitialize() override  { Parent::Deinitialize(); }
    void onMouseButtonEvent(SDL_MouseButtonEvent event) override  {}
    void onDraw() override {}
    void onKeyEvent(SDL_KeyboardEvent event) override  {
        if (event.type == SDL_EVENT_KEY_UP && event.key = SDLK_ESCAPE) {
            TerminateApplication();
        }
    }
    void onDrawTopMost() override
    {
        ImGui::SetNextWindowSizeConstraints(ImVec2(800.0f, 600.f), ImVec2(FLT_MAX, FLT_MAX));
        if (ImGui::Begin("RadioWana")) {

        }
        ImGui::End;

    }

};

int main(int argc, char* argv[])
{
    (void)argc; (void)argv;
    RadioWanaEval* app = new RadioWanaEval();
    app->mSettings.Caption = "RadioWanaEval";
    app->mSettings.Version = "0.260317";
    app->Execute();
    SAFE_DELETE(app);
    return 0;
}

