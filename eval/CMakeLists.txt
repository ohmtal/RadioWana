cmake_minimum_required(VERSION 3.16)
project(RadioWanaProto LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL3 REQUIRED)

# HTTPS
find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
    set(HTTPLIB_IS_USING_OPENSSL TRUE)
endif()

set(IMGGUI_DIR "${CMAKE_CURRENT_LIST_DIR}/lib/imgui")

# ------------- Prototype
set( APP_SOURCES

    eval/main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/lib/ohmtal/errorlog.cpp

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
)

target_include_directories(${PROJECT_NAME} PRIVATE
     "${IMGGUI_DIR}"
     "${IMGGUI_DIR}/backends"
     "${CMAKE_CURRENT_LIST_DIR}/lib/ohmtal"
     "${CMAKE_CURRENT_LIST_DIR}/lib/miniaudio"
     "${CMAKE_CURRENT_LIST_DIR}/lib/httplib"
)

target_link_libraries(${PROJECT_NAME} PUBLIC
        $<$<BOOL:${HTTPLIB_IS_USING_OPENSSL}>:OpenSSL::SSL>
        $<$<BOOL:${HTTPLIB_IS_USING_OPENSSL}>:OpenSSL::Crypto>)

target_compile_definitions(${PROJECT_NAME} PUBLIC
        $<$<BOOL:${HTTPLIB_IS_USING_OPENSSL}>:CPPHTTPLIB_OPENSSL_SUPPORT>
        )

target_compile_definitions(${PROJECT_NAME} PUBLIC IMGUI_DEFINE_MATH_OPERATORS)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

