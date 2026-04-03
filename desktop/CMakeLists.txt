cmake_minimum_required(VERSION 3.16)
project(IncludeTestProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Disable the demos
set(DISABLE_DEMOS ON CACHE BOOL "Disable engine demos" FORCE)
set(WITH_BOX2D OFF CACHE BOOL "with Box2D" FORCE)


set(OPL_DIR    "${CMAKE_CURRENT_LIST_DIR}/lib/opl")

# fetching OhmFlux
include(FetchContent)

FetchContent_Declare(
    ohmFlux
    GIT_REPOSITORY https://github.com/ohmtal/OhmFlux
    GIT_TAG        26-04-02.2
)

# This command downloads the code and automatically calls add_subdirectory
FetchContent_MakeAvailable(ohmFlux)

# ------------------ Radio Wana ---------------------
set(RADIOWANA_DIR "${CMAKE_CURRENT_LIST_DIR}/")

if(MSVC)
    find_package(CURL REQUIRED)
    list(APPEND PROJECT_LIBS CURL::libcurl)
    message(STATUS "Added Curl .....")

    set(RES_FILES "${RADIOWANA_DIR}/res/resources.rc")

else ()



    if (ANDROID)
        # set(CMAKE_POSITION_INDEPENDENT_CODE ON)
        # FetchContent_Declare(
        #     boringssl
        #     URL "${CMAKE_CURRENT_LIST_DIR}/lib/boringssl-0.20260327.0.tar.gz"
        #     DOWNLOAD_EXTRACT_TIMESTAMP FALSE
        # )
        #
        # FetchContent_MakeAvailable(boringssl)
        #
        # set(FAKE_LIB_PATH "${boringssl_SOURCE_DIR}/CMakeLists.txt")
        #
        # set(OPENSSL_FOUND TRUE CACHE BOOL "" FORCE)
        # set(OpenSSL_FOUND TRUE CACHE BOOL "" FORCE)
        #
        # set(OPENSSL_CRYPTO_LIBRARY "${FAKE_LIB_PATH}" CACHE FILEPATH "" FORCE)
        # set(OPENSSL_SSL_LIBRARY "${FAKE_LIB_PATH}" CACHE FILEPATH "" FORCE)
        #
        # set(OPENSSL_INCLUDE_DIR "${boringssl_SOURCE_DIR}/include" CACHE PATH "" FORCE)
        # set(OPENSSL_VERSION "1.1.1" CACHE STRING "" FORCE)
        # set(OPENSSL_IS_BORINGSSL ON CACHE BOOL "" FORCE)
        # set(CURL_USE_OPENSSL ON CACHE BOOL "" FORCE)

        set(CURL_USE_OPENSSL OFF CACHE INTERNAL "")

    else()
        set(CURL_USE_OPENSSL ON CACHE INTERNAL "")
    endif()


    set(BUILD_CURL_EXE OFF CACHE INTERNAL "")
    set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
    set(CURL_USE_LIBPSL OFF CACHE INTERNAL "")
    set(CURL_DISABLE_LDAP ON CACHE INTERNAL "")
    set(CURL_DISABLE_LDAPS ON CACHE INTERNAL "")
    set(HTTP_ONLY ON CACHE INTERNAL "") # i only need https at the moment !


    FetchContent_Declare(
        curl
        URL "${ohmflux_SOURCE_DIR}/lib/curl-curl-8_19_0.tar.gz"
        DOWNLOAD_EXTRACT_TIMESTAMP FALSE
    )
    FetchContent_MakeAvailable(curl)

if (ANDROID)
    # if(TARGET libcurl_static)
    #     target_link_libraries(libcurl_static PRIVATE ssl crypto)
    #     set_property(TARGET ssl PROPERTY EXPORT_NAME "ssl")
    #     set_property(TARGET crypto PROPERTY EXPORT_NAME "crypto")
    # endif()
    #
    # set(CURL_SKIP_INSTALL ON CACHE BOOL "" FORCE)
    # if(TARGET crypto)
    #     target_link_libraries(crypto INTERFACE log)
    # endif()

    # list(APPEND PROJECT_LIBS openssl z log)
endif()


    list(APPEND PROJECT_LIBS libcurl)
endif()



set(RADIOWANA_SOURCES
    "${RADIOWANA_DIR}/src/main.cpp"
    "${RADIOWANA_DIR}/src/appMain.h"
    "${RADIOWANA_DIR}/src/RadioWana.h"
    "${RADIOWANA_DIR}/src/RadioWana.cpp"
    "${RADIOWANA_DIR}/src/AudioHandler.h"
    "${RADIOWANA_DIR}/src/AudioHandler.cpp"
    "${RADIOWANA_DIR}/src/AudioRecorder.h"
    "${RADIOWANA_DIR}/src/BackGroundEffects.h"
    "${RADIOWANA_DIR}/src/StreamHandler.h"
    "${RADIOWANA_DIR}/src/StreamHandler.cpp"
    "${RADIOWANA_DIR}/src/RadioBrowser.h"
    "${RADIOWANA_DIR}/src/RadioBrowser.cpp"
    "${RADIOWANA_DIR}/src/StreamInfo.h"

)
configure_project_target(RadioWana ${RADIOWANA_DIR} "${RADIOWANA_DIR}/assets@assets" "${RADIOWANA_SOURCES}" ${RES_FILES})


