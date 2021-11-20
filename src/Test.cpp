#include <string>
#include <iostream>
#include "d3d11.h"

#include "Player.h"
#include "ConsoleLogWriter.h"
#include "D3DUtils.h"
#include "D3DVideoProcessor.h"
#include "lodepng.h"
#include "LogWriter.h"

int main() {
    // add a LoggerWriter
    auto ConsoleWriter = new XLog::ConsoleLogWriter();
    XLog::Logger::RegisterLogger(ConsoleWriter);
    // init VideoPlayer
    // std::string Url = "D:/Projects/XDance3.0/Game/Saved/StagedBuilds/WindowsNoEditor/XDance/Content/Coaches/chongqingdeweidaozuo1_2160p/Video.mp4";

    std::shared_ptr<XGraphic::VideoTextureProcessor> Processor;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> StageTex;

    XVideo::Player P;
    std::string Url = "D:/Projects/VPDemo/Content/Movies/lumi.mp4";
    P.Init(Url.c_str(), 7);//DXVA2=4, D3D11VA=7
    P.SetFrameUpdateCallback([&](void *ImageData, int64_t TexIndex, int32_t FrameIndex, int64_t FramePTS, int32_t Width, int32_t Height) {
        auto Device = XGraphic::TextureUtil::GetDevice(ImageData);

        if (Processor == nullptr) {
            Processor = XGraphic::TextureUtil::CreateVideoTextureProcessor(Device, Width, Height);
            StageTex = XGraphic::TextureUtil::CreateStageTexture_RGBA(Device, Width, Height);
        }
        if (Processor != nullptr) {
            Processor->ProcessTexture(ImageData, TexIndex);
            const auto OutputTex = Processor->GetOutputTexture();
            XGraphic::TextureUtil::CopyTexture(StageTex, Microsoft::WRL::ComPtr<ID3D11Texture2D>((ID3D11Texture2D *) OutputTex));
            uint32_t OutLen = 0;
            uint32_t OutWidth = 0;
            uint32_t OutHeight = 0;
            std::shared_ptr<uint8_t> Data = XGraphic::TextureUtil::CopyToMem(StageTex.Get(), OutLen, OutWidth, OutHeight);
            if (OutLen != 0) {
                char FileName[256] = {'\0'};
                std::snprintf(FileName, 256, "D:/Projects/XVideoLib/Frame/%d.png", FrameIndex);
                // unsigned error = lodepng_encode32_file(FileName, (uint8_t *) Data.get(), OutWidth, OutHeight);
                // XLog::Logger::Write(XLog::LogLevel::Info, "lodepng_encode32_file error=%d", error);
            }
        }
    });
    P.Play();
    std::this_thread::sleep_for(std::chrono::seconds(999));
    P.UnInit();
    // std::this_thread::sleep_for(std::chrono::seconds(3));
    return 0;
}