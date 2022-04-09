
cmake_minimum_required(VERSION 3.10.2)

project("helloffmpeg")

include_directories(
        include
        ${CMAKE_SOURCE_DIR}/util)

add_library( # Sets the name of the library.
        hello-ffmpeg

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        hello-ffmpeg.cpp)

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
        ffmpeg)