cmake_minimum_required(VERSION 3.0)
project(XVideoLib)
set(CMAKE_CXX_STANDARD 11)

#兼容包含中文字符的UTF8代码文件内容
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

#LodePNG
set(LODEPNG_PATH ${XVideoLib_SOURCE_DIR}/thirdparty/lodepng)
set(LODEPNG_INC_PATH ${LODEPNG_PATH})
set(LODEPNG_SRC ${LODEPNG_PATH}/lodepng.cpp)
add_library(LODEPNG_LIB STATIC ${LODEPNG_SRC})

#FFMpeg
set(FFMPEG_PATH ${XVideoLib_SOURCE_DIR}/thirdparty/ffmpeg)
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

#XLog
add_library(XLog SHARED src/LogWriter.cpp src/ConsoleLogWriter.cpp)
target_compile_definitions(XLog PRIVATE "BUILD_XLOG=1")

#XGraphic
add_library(XGraphic SHARED src/D3DUtils.cpp src/D3DVideoProcessor.cpp)
target_link_libraries(XGraphic XLog d3d11.lib)
target_compile_definitions(XGraphic PRIVATE "BUILD_XGRAPHIC=1")

#XVideo
add_library(XVideo SHARED src/Player.cpp src/Util.cpp)
target_include_directories(XVideo PRIVATE ${FFMPEG_INC_PATH})
target_link_directories(XVideo PRIVATE ${FFMPEG_LIB_PATH})
target_link_libraries(XVideo ${FFMPEG_LIB} XLog XGraphic d3d9.lib)
target_compile_definitions(XVideo PRIVATE "BUILD_XVIDEO=1")

#Test
add_executable(VideoPlayerTest src/Test.cpp)
target_include_directories(VideoPlayerTest PRIVATE ${FFMPEG_INC_PATH} ${LODEPNG_INC_PATH})
target_link_directories(VideoPlayerTest PRIVATE ${FFMPEG_LIB_PATH})
target_link_libraries(VideoPlayerTest
        XVideo
        LODEPNG_LIB
        ${FFMPEG_LIB})
