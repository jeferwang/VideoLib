cmake_minimum_required(VERSION 3.0)
project(VPLib)
set(CMAKE_CXX_STANDARD 11)


set(LODEPNG_PATH ${VPLib_SOURCE_DIR}/ThirdParty/lodepng)
set(LODEPNG_INC_PATH ${LODEPNG_PATH})
set(LODEPNG_SRC ${LODEPNG_PATH}/lodepng.cpp)
add_library(LODEPNG STATIC ${LODEPNG_SRC})


set(FFMPEG_PATH ${VPLib_SOURCE_DIR}/ThirdParty/ffmpeg)
set(FFMPEG_INC_PATH ${FFMPEG_PATH}/include)
set(FFMPEG_LIB_PATH ${FFMPEG_PATH}/lib)
set(FFMPEG_LIB avcodec.lib
        avdevice.lib
        avfilter.lib
        avformat.lib
        avutil.lib
        postproc.lib
        swresample.lib
        swscale.lib
        #        libavcodec.dll.a
        #        libavdevice.dll.a
        #        libavfilter.dll.a
        #        libavformat.dll.a
        #        libavutil.dll.a
        #        libpostproc.dll.a
        #        libswresample.dll.a
        #        libswscale.dll.a
        )

add_executable(VPLib
        Test.cpp
        VPLib.cpp
        Logger/Writer.cpp
        Logger/ConsoleLogWriter.cpp
        Util.cpp Util.h)

target_include_directories(VPLib PRIVATE
        ${FFMPEG_INC_PATH}
        ${LODEPNG_INC_PATH})
target_link_directories(VPLib PRIVATE ${FFMPEG_LIB_PATH})
target_link_libraries(VPLib
        ${FFMPEG_LIB}
        LODEPNG
        d3d9.lib)

#add_executable(VPTest Test/Test.cpp)
#target_include_directories(VPTest PUBLIC ${VPLib_SOURCE_DIR})
#target_link_libraries(VPTest VP)

#file(COPY ${FFMPEG_PATH}/bin/avcodec-58.dll DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

