#include <string>
#include <iostream>
#include "Player.h"
#include "ConsoleLogWriter.h"
#include "lodepng.h"

int main() {
    // add a LoggerWriter
    auto ConsoleWriter = new VP::Logger::ConsoleLogWriter();
    VP::Logger::Log::RegisterLogger(ConsoleWriter);
    // init VideoPlayer
    // std::string Url = "D:/Projects/XDance3.0/Game/Saved/StagedBuilds/WindowsNoEditor/XDance/Content/Coaches/chongqingdeweidaozuo1_2160p/Video.mp4";
    std::string Url = "D:/Projects/VPDemo/Content/Movies/lumi.mp4";
    VP::Player P;
    P.Init(Url.c_str(), 7);//DXVA2=4, D3D11VA=7
    P.SetFrameUpdateCallback([&P](void *ImageData, int32_t FrameIndex, int64_t FramePTS, int32_t Width, int32_t Height) {
        //Save frame rgba to png
        //char FileName[256] = {'\0'};
        //std::snprintf(FileName, 256, "D:/Projects/VPLib/Frame/%d.png", FrameIndex);
        //unsigned error = lodepng_encode32_file(FileName, (uint8_t *) ImageData, P.GetWidth(), P.GetHeight());
        //VP::Logger::Log::Write(VP::Logger::LogLevel::Info, "lodepng_encode32_file error=%d", error);
    });
    P.Play();
    std::this_thread::sleep_for(std::chrono::seconds(60));
    P.UnInit();
    // std::this_thread::sleep_for(std::chrono::seconds(3));
    return 0;
}