# RadioWana II

## Status: prototyping


I plan a remake of RadioWana a Internet Radio Recorder.

Goal is written in C++ Open Source und Cross Platform. 
Using: 

- Backend: SDL3
- Gui: ImGui 
- Https handling: libCurl 
- Decoder: miniaudio 


## RadioWana made in 200? 

Technologie: 

- written in Object Pascal using Delphi 5
- bass.dll for stream and meta data handling 
- some compontents to make it a nicer look

Features: 

- Playing a Internet Radio Stream
- Displaying the Meta-Data
- A fancy Stream visualizer 
- Playlists: Import of pls and m3u but using it's own format to save the stations 
- Recording 
    - the stream and saving into different MP3 files
    - Filename is defined using the Meta-Data
    - A Delay in Miliseconds, to match the name is earlier as the song begins - if there is a constant delay
    - saved in a folder where the files is played like [Artist]/[Songname]
    - 🐞 Unhandled: today some stations do update the meta long after the song is started 

    
It still runs fine but: 

- Windows only and porting to freePascal would be the same work as writting it new. And I barely use Windows anymore. 
- Need some enhancements but bass.dll is closed source 
- ImGui give me much more power for visual styling. 
    

# Milestones

- [ ] Prototype enter URL, fetch stream data, decode, playback, and recording 
    - [X] Setup Project
        - CMakeLists.txt
        - src
        - libs 
            - OhmFlux ( for SDL3, ImGui and more )
            - httplib 
            - miniaudio
    - [X] Setup Project again for faster prototyping 
        - CMakeLists using SDL3 from system
        - libs
            - BaseFlux
            - ImGui 
            - Ohmtal: some parts of OhmFlux
            - httplib
            - miniaudio
    - [ ] RadioWanaProto
        - [X] fetch stream data and meta data :D 
        - [ ] decode 
            - [X] mp3 
            - [ ] add ffmpeg backend #include "extras/miniaudio_libav.h"
        - [X] playback
        - [X] sync title trigger with audio stream 
        - [X] fix memory raise over time =>  rawbuffer, decoder, curl ... ?  => sanitize=address 

    - [ ] Recoding
        - [X] add callback to AudioHandler
        - [X] add class AudioRecorder
        - [X] add checks for directory and file system full
        - [X] open/close/write stream 
        - [X] add source to main for recording
        - [ ] add format mp3 only at the moment
        
    - [ ] radio-browser.info using Curl 
        - [~] SRV lookup _api._tcp.radio-browser.info << cross platform problem ?
            - for testing i use de1.api.radio-browser.info
        - [X] add a search
        - [ ] results of queries should be cached. 
        - [X] XML or JSON parsing the request results << nlohman
    
- [ ] Final Version 
    - [ ] switch to OhmFlux agian with it's build system 
    - [ ] add delay to switch file name, this should be saved for each station 
        - rock antenne sends meta data too early for example 3 sec or so 
        - i can use OhmFlux Scheduler :) 
    - [ ] Gui enhancements 
        - [ ] design like an old 70th/80th radio recorder 
        - [ ] keep android GUI in mind (no context menus)
        - [ ] define and save settings 
        - [ ] ...fixme write todos ;) ...
        
    - [ ] Test on windows 
        - write docu like https://github.com/ohmtal/OhmFlux/blob/main/README_BUILD_WINDOWS.md but add curl 
        - i may add curl to OhmFlux so simple add this there 
    
