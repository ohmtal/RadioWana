//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// BaseFlux Header
//-----------------------------------------------------------------------------
#pragma once
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"


#include <string>
#include <array>
#include <memory>
#include <functional>

namespace BaseFlux {

    //-----------------------------------------------------------------------------
    // Settings
    //-----------------------------------------------------------------------------
    inline std::string sanitizeFilenameWithUnderScores(std::string name)
    {
        std::string result;
        for (unsigned char c : name) {
            if (std::isalnum(c)) {
                result += c;
            } else if (std::isspace(c)) {
                result += '_';
            }
            // Special characters (like '.') are ignored/dropped here
        }
        return result;
    }
    //--------------------------------------------------------------------------
    struct Settings {
        std::array<uint16_t, 2> ScreenSize = { 1152, 648};
        // not the exact fps since i use integer and round it.
        uint16_t  FpsLimit = 0;
        bool WindowMaximized  = false;
        // you also can set FpsLimit
        bool EnableVSync      = true;
        std::string Company = "BaseFlux Company";
        std::string Caption = "BaseFlux Caption";
        std::string Version = "BaseFlux Version 1";

        // your window icon (have to be .bmp or .png)
        std::string IconFilename = "";

        //pre path for IconFilename and loadTexture
        // base:/ is replaced with your BasePath
        std::string AssetPath = "base:/assets/";

        //imgui
        bool EnableDockSpace = true;
        // pref:/ is replaced with your pref Path
        std::string IniFileName = "pref:/appgui.ini";

        std::string getPrefsPath() {
            static std::string cachedPath = "";
            if (!cachedPath.empty()) return cachedPath;
            const char* rawPath = SDL_GetPrefPath(getSafeCompany().c_str(), getSafeCaption().c_str());
            cachedPath = rawPath ? std::string(rawPath) : "";
            return cachedPath;
        }


        std::string getSafeCompany() {
            return sanitizeFilenameWithUnderScores(Company);
        }
        std::string getSafeCaption() {
            return sanitizeFilenameWithUnderScores(Caption);
        }

    };
    //--------------------------------------------------------------------------
    // String replace helper
    inline std::string string_replace_all(std::string str, const std::string& from, const std::string& to) {
        size_t start_pos = 0;
        while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length();
        }
        return str;
    }
    //--------------------------------------------------------------------------
    // getBasePath wrapper
    inline std::string getBasePath() {
        static std::string cachedPath = "";
        if (!cachedPath.empty()) return cachedPath;

        const char* rawPath = SDL_GetBasePath();
        if (rawPath) {
            cachedPath = rawPath;
            // NOT! SDL_free(const_cast<char*>(rawPath));
        }
        return cachedPath;
    }

    //-----------------------------------------------------------------------------
    // BaseFlux Main Class
    //-----------------------------------------------------------------------------
    class Main {
    protected:
        SDL_Window* mWindow = nullptr;
        SDL_Renderer* mRenderer = nullptr;

        ImGuiIO* mImGuiIO = nullptr;
        ImGuiID  mDockSpaceId = -1;

        bool mShutDownComplete = false;
        bool mRunning = false;

    public:
        Settings mSettings;

        Main() = default;
        ~Main()  = default;
        //----------------------------------------------------------------------
        SDL_Window*     getWindow()      const { return mWindow; };
        SDL_Renderer*   getRenderer()    const { return mRenderer; }

        ImGuiIO*        getImGuiIO()     const { return mImGuiIO; }
        ImGuiID         getDockSpaceId() const { return mDockSpaceId; }

        std::function<void(SDL_Renderer*)> OnRender = nullptr;
        std::function<void(const SDL_Event)> OnEvent = nullptr;
        std::function<void()> OnShutDown = nullptr;


        //--------------------------------------------------------------------------
        /**
         * replace a path with full path
         * %base => SDL_GetBasePath
         * %pref => Settings::getPrefsPath
         */
        void setFullPath(std::string  &path) {
            if (path.find("base:/", 0) != std::string::npos) {
                path = string_replace_all(path, "base:/", getBasePath());
            }
            if (path.find("pref:/", 0) != std::string::npos) {
                path = string_replace_all(path, "pref:/", mSettings.getPrefsPath());
            }
        }
        //--------------------------------------------------------------------------
        // Load a Texture (.bmp,.png) relative to AssetPath
        bool loadTexture(std::string fileName, SDL_Texture*& texture) {
            if (!mRenderer) return false;
            fileName = mSettings.AssetPath + "/" + fileName;
            setFullPath(fileName);
            // SDL_Log("[info] Loading image: %s", fileName.c_str());
            SDL_Surface* surface = SDL_LoadBMP(fileName.c_str());
            if (!surface) {
                SDL_Log("[error] Failed to load texture at %s: %s", fileName.c_str(), SDL_GetError());
                return false;
            }
            texture = SDL_CreateTextureFromSurface(mRenderer, surface);
            SDL_DestroySurface(surface);
            if (!texture) {
                SDL_Log("[error] Texture Create Error: %s", SDL_GetError());
                return false;
            }
            return true;
        }
        //--------------------------------------------------------------------------
        // Set the window icon relative to AssetPath
        bool setWindowIcon(SDL_Window* window, std::string fileName) {
            fileName = mSettings.AssetPath + "/" + fileName;
            setFullPath(fileName);
            // SDL_Log("[info] Loading icon: %s", fileName.c_str());

            SDL_Surface* iconSurface = SDL_LoadBMP(fileName.c_str());
            if (!iconSurface) {
                SDL_Log("[warn] Failed to load icon: %s Error:%s",fileName.c_str(), SDL_GetError());
                return false;
            }
            SDL_SetWindowIcon(window, iconSurface);
            SDL_DestroySurface(iconSurface);
            return true;
        }
        //----------------------------------------------------------------------
        bool InitSDL() {
            if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
                SDL_Log("[error] SDL_Init failed: %s", SDL_GetError());
                return false;
            }

            SDL_WindowFlags flags = SDL_WINDOW_RESIZABLE;
            if (mSettings.WindowMaximized)  flags |= SDL_WINDOW_MAXIMIZED;
            mWindow = SDL_CreateWindow(
                mSettings.Caption.c_str()
                , mSettings.ScreenSize[0]
                , mSettings.ScreenSize[1]
                , flags);

            if (!mWindow) {
                SDL_Log("[error] SDL_CreateWindow failed: %s", SDL_GetError());
                return false;
            }

            if (!mSettings.IconFilename.empty()) {
                setWindowIcon(mWindow, mSettings.IconFilename.c_str());
            }

            mRenderer = SDL_CreateRenderer(mWindow, NULL);
            if (!mRenderer) {
                SDL_Log("[error] SDL_CreateRenderer failed: %s", SDL_GetError());
                return false;
            }

            SDL_SetRenderVSync(mRenderer,mSettings.EnableVSync);

            return true;
        }
        //----------------------------------------------------------------------
        bool initImGui() {
            if (!mWindow || !mRenderer) {
                SDL_Log("[error] Init ImGui failed: InitSDL first!");
                return false;
            }

            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            mImGuiIO = &ImGui::GetIO();
            if (!mSettings.IniFileName.empty()) {
                setFullPath(mSettings.IniFileName);
                mImGuiIO->IniFilename = mSettings.IniFileName.c_str();
            } else {
                mImGuiIO->IniFilename = nullptr;
            }
            mImGuiIO->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
            mImGuiIO->ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
            ImGui_ImplSDL3_InitForSDLRenderer(mWindow, mRenderer);
            ImGui_ImplSDLRenderer3_Init(mRenderer);

            return true;
        }
        //----------------------------------------------------------------------
        void shutDown() {
            if (mShutDownComplete) return; //double call safety
            if (OnShutDown) OnShutDown();
            ImGui_ImplSDLRenderer3_Shutdown();
            ImGui_ImplSDL3_Shutdown();
            ImGui::DestroyContext();
            if (mRenderer) { SDL_DestroyRenderer(mRenderer); mRenderer = nullptr; }
            if (mWindow) {SDL_DestroyWindow(mWindow); mWindow = nullptr;}
            SDL_Quit();
            mShutDownComplete = true;
        }
        //----------------------------------------------------------------------
        bool Execute() {
            if (!mWindow || !mRenderer) {
                SDL_Log("[error] Init ImGui failed: InitSDL first!");
                return false;
            }

            bool usingImGui = mImGuiIO != nullptr;

            Uint32 frameStart, frameTime;
            uint16_t frameLimit = 0;

            if (mSettings.FpsLimit > 0) frameLimit = static_cast<uint16_t>(1000 / mSettings.FpsLimit);

            mRunning = true;

            while (mRunning) {
                frameStart = SDL_GetTicks();

                SDL_Event event;
                while (SDL_PollEvent(&event)) {
                    if (usingImGui)  ImGui_ImplSDL3_ProcessEvent(&event);
                    switch (event.type) {
                        case SDL_EVENT_QUIT: mRunning = false; break;
                        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                            if (event.window.windowID == SDL_GetWindowID(mWindow)) {
                                mRunning = false;
                            }
                            break;
                    }

                    if (OnEvent)
                        OnEvent(event);
                }

                // ~~~ ImGui Begin ~~~
                if (usingImGui) {

                    ImGui_ImplSDLRenderer3_NewFrame();
                    ImGui_ImplSDL3_NewFrame();
                    ImGui::NewFrame();

                    if (mSettings.EnableDockSpace)
                    {
                        ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode; //<< this makes it transparent
                        mDockSpaceId = ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), dockspace_flags);
                    }

                }

                SDL_SetRenderDrawColor(mRenderer, 45, 45, 45, 255);
                SDL_RenderClear(mRenderer);

                if (OnRender) OnRender(mRenderer);


                // ~~~ ImGui End ~~~
                if (usingImGui) {
                    ImGui::Render();
                    ImDrawData* drawData = ImGui::GetDrawData();
                    if (drawData)
                    {
                        ImGui_ImplSDLRenderer3_RenderDrawData(drawData, mRenderer);
                    }
                }

                SDL_RenderPresent(mRenderer);

                frameTime = SDL_GetTicks() - frameStart;
                // FPS Limiter
                if ( mSettings.FpsLimit > 0  && frameLimit > 0) {
                    if (frameTime < frameLimit) {
                        SDL_Delay(frameLimit - frameTime);
                    }
                }

            }
            shutDown();
            return true;
        }
        //----------------------------------------------------------------------
        void TerminateApplication(void)
        {
            static SDL_Event Q;
            Q.type = SDL_EVENT_QUIT;
            if(!SDL_PushEvent(&Q))
            {
                exit(1);
            }
            return;
        }
    }; //class BaseFlux
}; //namespace
