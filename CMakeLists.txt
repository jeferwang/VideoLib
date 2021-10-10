cmake_minimum_required(VERSION 3.0)
project(VPLib)
set(CMAKE_CXX_STANDARD 11)
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

set(LODEPNG_PATH ${VPLib_SOURCE_DIR}/ThirdParty/lodepng)
set(LODEPNG_INC_PATH ${LODEPNG_PATH})
set(LODEPNG_SRC ${LODEPNG_PATH}/lodepng.cpp)
add_library(LODEPNG_LIB STATIC ${LODEPNG_SRC})


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

add_library(VPLib SHARED
        VPLib.cpp
        Logger/Writer.cpp
        Logger/ConsoleLogWriter.cpp
        Util.cpp)
target_include_directories(VPLib PRIVATE ${FFMPEG_INC_PATH})
target_link_directories(VPLib PRIVATE ${FFMPEG_LIB_PATH})
target_link_libraries(VPLib ${FFMPEG_LIB} d3d9.lib)
target_precompile_headers(VPLib PRIVATE PCH.h)
set_target_properties(VPLib PROPERTIES PUBLIC_HEADER "VPLib.h;Util.h")
set(CMAKE_INSTALL_PREFIX "${VPLib_SOURCE_DIR}/out")

add_executable(VPTest Test.cpp)
target_include_directories(VPTest PRIVATE ${FFMPEG_INC_PATH} ${LODEPNG_INC_PATH})
target_link_directories(VPTest PRIVATE ${FFMPEG_LIB_PATH})
target_link_libraries(VPTest
        VPLib
        LODEPNG_LIB
        ${FFMPEG_LIB})
target_precompile_headers(VPTest PRIVATE PCH.h)

install(TARGETS VPTest)
file(REMOVE_RECURSE ${CMAKE_INSTALL_PREFIX})
install(TARGETS VPLib
        PUBLIC_HEADER DESTINATION include/
        )
install(FILES $<TARGET_PDB_FILE:VPTest>
        DESTINATION bin/
        OPTIONAL
        )
install(
        DIRECTORY ${FFMPEG_PATH}/bin
        DESTINATION .
        FILES_MATCHING PATTERN "*.dll"
)
