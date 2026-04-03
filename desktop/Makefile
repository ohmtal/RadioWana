.PHONY: usage help build-info debug release windebug winrelease android webdebug webrelease webdist clean distclean

# Configuration
# --------------

BASE_BUILD_DIR := _build
WEBDIST_DIR := dist_web
EMSCRIPTEN_TOOLCHAIN := /usr/lib/emscripten/cmake/Modules/Platform/Emscripten.cmake

# Android config:
# Your assets must be in subdirectory assets!
#TODO: Change the path /opt/android to where you installed android studio !
NDK_DIRS := $(wildcard /opt/android/sdk/ndk/*/)
ANDROID_PROJ_DIR := android
ANDROID_NDK_HOME := $(lastword $(sort $(NDK_DIRS)))
# ANDROID_NDK_HOME := $(shell @ls -d /opt/android/sdk/ndk/*/ | sort -V | tail -n 1)
ANDROID_PLATFORM := android-24

# Parallel Build Detection
# Uses all available cores on Linux/FreeBSD, defaults to 4 if detection fails
# JOBS := -j $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
# Fix for BSD make:
JOBS != nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4
JOBS := -j $(JOBS)

# Tool Detection
BROTLI := $(shell command -v brotli 2> /dev/null)
GZIP   := $(shell command -v gzip 2> /dev/null)

# -----------------  H E L P  &  S E T U P  --------------------
help: usage

usage:
	@echo "-----------------------------------------------------------------"
	@echo -e "\e[34mOhmFlux Build System - Usage\e[0m"
	@echo "-----------------------------------------------------------------"
	@echo "make debug      : Build native Desktop (Debug)"
	@echo "make release    : Build native Desktop (Release)"
	@echo ""
	@echo "make windebug   : Cross-compile Windows (Debug via MinGW)"
	@echo "make winrelease : Cross-compile Windows (Release via MinGW)"
	@echo ""
	@echo "make android    : experimental Android (Release ARM64)"
	@echo ""
	@echo "make webdebug   : Build WebGL (Debug via Emscripten)"
	@echo "make webrelease : Build WebGL (Release via Emscripten)"
	@echo "make webdist    : Build WebGL Release and deploy to $(WEBDIST_DIR)"
	@echo ""
	@echo "make build-info : Show packages to install on Arch and FreeBSD "
	@echo ""
	@echo "make clean      : Remove $(BASE_BUILD_DIR)/ directory"
	@echo "make distclean  : Remove all build artifacts, binaries, and $(WEBDIST_DIR)"
	@echo "-----------------------------------------------------------------"
	@echo ""

build-info:
	@echo "--- [ Arch Linux Setup ] ---"
	@echo "Nativ:      sudo pacman -S sdl3 glew opengl-headers"
	@echo "Windows:    yay -S mingw-w64-sdl3 mingw-w64-glew"
	@echo "Android:    download and install Android Studio and set the pathes like:"
	@echo "            export ANDROID_HOME=/opt/android/sdk"
	@echo "            export ANDROID_SDK_ROOT=/opt/android/sdk"
	@echo "            export ANDROID_NDK_HOME=/opt/android/sdk/ndk/28.2.13676358  # Update version if different"
	@echo "            export PATH=$PATH:$ANDROID_HOME/cmdline-tools/latest/bin:$ANDROID_HOME/platform-tools"
	@echo "            sudo pacman -S gradle"
	@echo "Emscripten: sudo pacman -S emscripten"
	@echo "            testing @bash:"
	@echo "            source /etc/profile.d/emscripten.sh"
	@echo "            emrun index.html"
	@echo ""
	@echo "--- [ FreeBSD Setup ] ---"
	@echo "Nativ:   sudo pkg install cmake gcc sdl3 glew"
	@echo ""
	@echo ""

# -----------------  D E S K T O P  --------------------
debug:
	cmake -S . -B $(BASE_BUILD_DIR)/debug -DCMAKE_BUILD_TYPE=Debug
	cmake --build $(BASE_BUILD_DIR)/debug $(JOBS)

release:
	cmake -S . -B $(BASE_BUILD_DIR)/release -DCMAKE_BUILD_TYPE=Release
	cmake --build $(BASE_BUILD_DIR)/release $(JOBS)

# ---------- C R O S S C O M P I L E for W I N ----------------
windebug:
	x86_64-w64-mingw32-cmake -S . -B $(BASE_BUILD_DIR)/win_debug -DCMAKE_BUILD_TYPE=Debug
	cmake --build $(BASE_BUILD_DIR)/win_debug $(JOBS)

winrelease:
	x86_64-w64-mingw32-cmake -S . -B $(BASE_BUILD_DIR)/win_release -DCMAKE_BUILD_TYPE=Release
	cmake --build $(BASE_BUILD_DIR)/win_release $(JOBS)


# -----------------  W E B  --------------------
webdebug:
	cmake -S . -B $(BASE_BUILD_DIR)/web_debug \
		-DCMAKE_TOOLCHAIN_FILE=$(EMSCRIPTEN_TOOLCHAIN) \
		-DCMAKE_BUILD_TYPE=Debug
	cmake --build $(BASE_BUILD_DIR)/web_debug $(JOBS)

webrelease:
	cmake -S . -B $(BASE_BUILD_DIR)/web_release \
		-DCMAKE_TOOLCHAIN_FILE=$(EMSCRIPTEN_TOOLCHAIN) \
		-DCMAKE_BUILD_TYPE=Release
	cmake --build $(BASE_BUILD_DIR)/web_release $(JOBS)



# -----------------  C L E A N --------------------
clean:
	@echo "Removing build directory..."
	rm -rf build
	rm -rf $(BASE_BUILD_DIR)
	echo "done"

