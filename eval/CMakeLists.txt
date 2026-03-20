cmake_minimum_required(VERSION 3.16)
project(RadioWanaProto LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL3 REQUIRED)
find_package(CURL REQUIRED)


set(IMGGUI_DIR "${CMAKE_CURRENT_LIST_DIR}/../lib/imgui")

# ------------- Prototype
set( APP_SOURCES

    src/main.cpp
    src/AudioHandler.cpp
    src/StreamHandler.cpp

    ${CMAKE_CURRENT_LIST_DIR}/../lib/ohmtal/utils/errorlog.cpp

    #  --- imggui ---
    ${IMGGUI_DIR}/imgui.cpp
    ${IMGGUI_DIR}/imgui_draw.cpp
    ${IMGGUI_DIR}/imgui_widgets.cpp
    ${IMGGUI_DIR}/imgui_tables.cpp
    ${IMGGUI_DIR}/backends/imgui_impl_sdl3.cpp
    ${IMGGUI_DIR}/backends/imgui_impl_sdlrenderer3.cpp

)

add_executable(${PROJECT_NAME}
    ${APP_SOURCES}
)
target_link_libraries(RadioWanaProto PRIVATE
    SDL3::SDL3
    pthread
    CURL::libcurl
)

# target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=address -g)
# target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=address)

target_include_directories(${PROJECT_NAME} PRIVATE
     "${IMGGUI_DIR}"
     "${IMGGUI_DIR}/backends"
     "${CMAKE_CURRENT_LIST_DIR}/../lib/ohmtal"
     "${CMAKE_CURRENT_LIST_DIR}/../lib/ohmtal/dsp"
     "${CMAKE_CURRENT_LIST_DIR}/../lib/miniaudio"
     "${CMAKE_CURRENT_LIST_DIR}/../lib/json/single_include"
)


target_compile_definitions(${PROJECT_NAME} PUBLIC IMGUI_DEFINE_MATH_OPERATORS)
target_compile_definitions(${PROJECT_NAME} PUBLIC FLUX_ENGINE) # fake entry for DSP
target_compile_definitions(${PROJECT_NAME} PUBLIC FLUX_ENGINE_FAKE)


if (UNIX)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_NAME "${PROJECT_NAME}.x86_64"

    )
endif()


