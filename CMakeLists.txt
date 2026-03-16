cmake_minimum_required(VERSION 3.16)
project(RadioWana LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Disable the demos
set(DISABLE_DEMOS ON CACHE BOOL "Disable engine demos" FORCE)
set(WITH_BOX2D OFF CACHE BOOL "Enable Box 2D : off" FORCE)

# fetching OhmFlux
include(FetchContent)

FetchContent_Declare(
    ohmFlux
    GIT_REPOSITORY https://github.com/ohmtal/OhmFlux
    GIT_TAG        26-03-17
)

# This command downloads the code and automatically calls add_subdirectory
FetchContent_MakeAvailable(ohmFlux)

# ------------- Prototype

configure_project_target(RadioWanaProto  "${CMAKE_CURRENT_SOURCE_DIR}/" "${CMAKE_CURRENT_SOURCE_DIR}/assets@assets" "eval/main.cpp")

target_include_directories(RadioWanaProto PRIVATE
     "${CMAKE_CURRENT_LIST_DIR}/lib/miniaudio"
     "${CMAKE_CURRENT_LIST_DIR}/lib/httplib"
)


# ------------- RadioWana

# set(APP_SOURCES
#     src/main.cpp
#
# )

# example from TomsOldFM....
# set(OPL_DIR    "${CMAKE_CURRENT_LIST_DIR}/lib/opl")


# configure_project_target(RadioWana  "${CMAKE_CURRENT_SOURCE_DIR}/" "${CMAKE_CURRENT_SOURCE_DIR}/assets@assets" "${APP_SOURCES}")
#
# target_include_directories(RadioWana PRIVATE
#     # $<BUILD_INTERFACE:${OPL_DIR}>
# )
