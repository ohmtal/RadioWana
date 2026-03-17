# RadioWana II

## Status: Evaluatuion how to implement it ... 


I plane a remake of RadioWana a Internet Radio Recorder.

Goal is written in C++ Open Source und Cross Platform. 
Using: 

- Backend: SDL3
- Gui: ImGui 
- Https handling: httplib (https://github.com/yhirose/cpp-httplib ),  libCurl is good but to massive 
- MP3/Ogg converting: miniaudio (https://miniaud.io/) or dr_libs (https://github.com/mackron/dr_libs),  ffmpeg or ibmpg123/libvorbis


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

- [ ] Prototype enter URL (https://streams.radiobob.de/powermetal/mp3-192/streams.radiobob.de/) fetch stream data, decode, playback 
    - [X] Setup Project
        - CMakeLists.txt
        - src
        - libs 
            - OhmFlux ( for SDL3, ImGui and more )
            - httplib 
            - miniaudio
    - RadioWanaProto
        - [ ] fetch stream data
            - still fighting with httplib - not sure it's my friend ;) It works with simple urls but seams to have problems with redirects. 
            
        - [ ] decode 
        - [ ] playback
            
- [ ] Handle and Display MetaData ( HTTP Header: Icy-MetaData: 1 )
- [ ] Recoding 
- [ ] Gui enhancements and save playlists and more 
