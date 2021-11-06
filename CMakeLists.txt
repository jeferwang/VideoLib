cmake_minimum_required(VERSION 3.0)
project(VIDEO_PLAYER)
set(CMAKE_CXX_STANDARD 11)

# 兼容包含中文字符的UTF8代码文件内容
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

# LodePNG
set(LODEPNG_PATH ${VIDEO_PLAYER_SOURCE_DIR}/thirdparty/lodepng)
set(LODEPNG_INC_PATH ${LODEPNG_PATH})
set(LODEPNG_SRC ${LODEPNG_PATH}/lodepng.cpp)
add_library(LODEPNG_LIB STATIC ${LODEPNG_SRC})

# FFMpeg
set(FFMPEG_PATH ${VIDEO_PLAYER_SOURCE_DIR}/thirdparty/ffmpeg)
set(FFMPEG_INC_PATH ${FFMPEG_PATH}/include)
set(FFMPEG_LIB_PATH ${FFMPEG_PATH}/lib)
set(FFMPEG_LIB avcodec.lib
        avdevice.lib
        avfilter.lib
        avformat.lib
        avutil.lib
        postproc.lib
        swresample.lib
        swscale.lib)

# VideoPlayerCore
add_library(VideoPlayerCore SHARED
        src/Player.cpp
        src/Logger.cpp
        src/ConsoleLogWriter.cpp
        src/Util.cpp)
target_include_directories(VideoPlayerCore PRIVATE ${FFMPEG_INC_PATH})
target_link_directories(VideoPlayerCore PRIVATE ${FFMPEG_LIB_PATH})
target_link_libraries(VideoPlayerCore ${FFMPEG_LIB} d3d9.lib)

# VPTest
add_executable(VideoPlayerTest src/Test.cpp)
target_include_directories(VideoPlayerTest PRIVATE ${FFMPEG_INC_PATH} ${LODEPNG_INC_PATH})
target_link_directories(VideoPlayerTest PRIVATE ${FFMPEG_LIB_PATH})
target_link_libraries(VideoPlayerTest
        VideoPlayerCore
        LODEPNG_LIB
        ${FFMPEG_LIB})
