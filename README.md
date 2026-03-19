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
    - RadioWanaProto
        - [X] fetch stream data and meta data :D 
        - [X] decode 
        - [X] playback
        - [X] sync title trigger with audio stream 
        

    - [ ] Recoding
        - [ ] test: simply write stream to file on title trigger 
    
- [ ] Final Version 
    - [ ] switch to OhmFlux agian with it's build system 
    - [ ] Gui enhancements 
        - [ ] ...fixme write todos ;) ...
        - [ ] ...
    - [ ] Test on windows 
    
- [ ] Maybe:
    - [ ] radio-browser.info. 
        - [ ] SRV lookup _api._tcp.radio-browser.info << cross platform problem ?
        - [ ] results of queries should be cached. 
        - [ ] XML or JSON parsing the request results << nlohman
    
