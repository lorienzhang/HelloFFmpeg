
cmake_minimum_required(VERSION 3.10.2)

project("helloffmpeg")

include_directories(
        include
        ${CMAKE_SOURCE_DIR}/util
        ${CMAKE_SOURCE_DIR}/render
        ${CMAKE_SOURCE_DIR}/decoder)

file(GLOB src-files
        ${CMAKE_SOURCE_DIR}/*.cpp
        ${CMAKE_SOURCE_DIR}/util/*.cpp
        ${CMAKE_SOURCE_DIR}/player/*.cpp
        ${CMAKE_SOURCE_DIR}/player/decoder/*.cpp
        ${CMAKE_SOURCE_DIR}/player/render/video/*.cpp
        ${CMAKE_SOURCE_DIR}/player/render/audio/*.cpp
        )

add_library( # Sets the name of the library.
        hello-ffmpeg

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        ${src-files})

add_library(ffmpeg SHARED IMPORTED)
set_target_properties(ffmpeg PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libffmpeg.so)

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)


target_link_libraries( # Specifies the target library.
        hello-ffmpeg

        # Links the target library to the log library
        # included in the NDK.
        ${log-lib}
        android
        OpenSLES
        ffmpeg)