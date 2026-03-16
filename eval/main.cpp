//-----------------------------------------------------------------------------
// Copyright (c) 2012 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// RadioWanaEval
//-----------------------------------------------------------------------------
#include <SDL3/SDL_main.h>

#include <fluxMain.h>
#include <gui/fluxGuiGlue.h>
#include <gui/ImFlux.h>
#include <imgui.h>
#include <miniaudio.h>
#include <httplib.h>

class RadioWanaEval : public FluxMain
{
    typedef FluxMain Parent;

private:
     FluxGuiGlue* mGuiGlue;
public:

    bool Initialize() override    {
        if (!Parent::Initialize()) return false;
        mGuiGlue = new FluxGuiGlue(true);
        if (!mGuiGlue->Initialize())
            return false;
        return true;
    }
    void Deinitialize() override  {
        SAFE_DELETE(mGuiGlue);
        Parent::Deinitialize();
    }
    void onEvent(SDL_Event event) override{ mGuiGlue->onEvent(event);}
    void onKeyEvent(SDL_KeyboardEvent event) override  {
        if ((event.type == SDL_EVENT_KEY_UP) && (event.key = SDLK_ESCAPE)) {
            TerminateApplication();
        }
    }
    void onDrawTopMost() override
    {
        mGuiGlue->DrawBegin();
        ImGui::SetNextWindowSizeConstraints(ImVec2(800.0f, 600.f), ImVec2(FLT_MAX, FLT_MAX));
        if (ImGui::Begin("RadioWana")) {


        }
        ImGui::End();
        mGuiGlue->DrawEnd();

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

