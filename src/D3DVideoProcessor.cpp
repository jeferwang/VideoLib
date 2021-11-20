#include <cstdint>
#include "d3d11.h"
#include "wrl/client.h"
#include "D3DVideoProcessor.h"
#include "D3DUtils.h"

#define SetZero(Var) memset(&Var, 0, sizeof(Var));

namespace XGraphic {
    using Microsoft::WRL::ComPtr;

    bool VideoTextureProcessor::Initialize(const ComPtr<ID3D11Device> &InDevice, const uint32_t InWidth, const uint32_t InHeight) {
        if (bInit) return false;

        Width = InWidth;
        Height = InHeight;

        HRESULT Hr = -1;

        Device = InDevice;
        if (Device == nullptr) return false;

        Device->GetImmediateContext(DeviceContext.GetAddressOf());
        if (DeviceContext == nullptr) return false;

        Hr = Device->QueryInterface(_uuidof(ID3D11VideoDevice), (void **) VideoDevice.GetAddressOf());
        if (VideoDevice == nullptr || FAILED(Hr)) return false;

        Hr = DeviceContext->QueryInterface(__uuidof(ID3D11VideoContext), (void **) VideoContext.GetAddressOf());
        if (VideoContext == nullptr || FAILED(Hr)) return false;

        D3D11_VIDEO_PROCESSOR_CONTENT_DESC VideoContentDesc;
        SetZero(VideoContentDesc);
        VideoContentDesc.InputFrameFormat = D3D11_VIDEO_FRAME_FORMAT_PROGRESSIVE;
        VideoContentDesc.InputFrameRate = {1, 1};
        VideoContentDesc.InputWidth = Width;
        VideoContentDesc.InputHeight = Height;
        VideoContentDesc.OutputFrameRate = {1, 1};
        VideoContentDesc.OutputWidth = Width;
        VideoContentDesc.OutputHeight = Height;
        VideoContentDesc.Usage = D3D11_VIDEO_USAGE_PLAYBACK_NORMAL;

        Hr = VideoDevice->CreateVideoProcessorEnumerator(&VideoContentDesc, VideoProcessorEnumerator.GetAddressOf());
        if (VideoProcessorEnumerator == nullptr || FAILED(Hr)) return false;

        Hr = VideoDevice->CreateVideoProcessor(VideoProcessorEnumerator.Get(), 0, VideoProcessor.GetAddressOf());
        if (VideoProcessor == nullptr || FAILED(Hr)) return false;

        OutputTexture = TextureUtil::CreateVideoProcessOutputTexture_RGBA(Device, Width, Height);

        bInit = true;
        return true;
    }

    bool VideoTextureProcessor::ProcessTexture(const ComPtr<ID3D11Texture2D> &InTex, const int64_t InTexArrayIdx) {
        HRESULT Hr = -1;

        if (InputViewMap.find(InTex) == InputViewMap.end()) {
            D3D11_VIDEO_PROCESSOR_INPUT_VIEW_DESC InputViewDesc;
            SetZero(InputViewDesc);
            InputViewDesc.FourCC = 0;
            InputViewDesc.ViewDimension = D3D11_VPIV_DIMENSION_TEXTURE2D;
            InputViewDesc.Texture2D = {0, static_cast<uint32_t>(InTexArrayIdx)};// MipSlice & ArraySlice
            ComPtr<ID3D11VideoProcessorInputView> InputView;
            Hr = VideoDevice->CreateVideoProcessorInputView(InTex.Get(), VideoProcessorEnumerator.Get(), &InputViewDesc, InputView.GetAddressOf());
            if (InputView == nullptr || FAILED(Hr))
                return false;
            InputViewMap.insert(std::pair<ComPtr<ID3D11Texture2D>, ComPtr<ID3D11VideoProcessorInputView>>(InTex, InputView));
        }

        if (OutputViewMap.find(InTex) == OutputViewMap.end()) {
            D3D11_VIDEO_PROCESSOR_OUTPUT_VIEW_DESC OutputViewDesc;
            SetZero(OutputViewDesc);
            OutputViewDesc.ViewDimension = D3D11_VPOV_DIMENSION_TEXTURE2D;
            ComPtr<ID3D11VideoProcessorOutputView> OutputView;
            Hr = VideoDevice->CreateVideoProcessorOutputView(OutputTexture.Get(), VideoProcessorEnumerator.Get(), &OutputViewDesc, &OutputView);
            if (OutputView == nullptr || FAILED(Hr))
                return false;
            OutputViewMap.insert(std::pair<ComPtr<ID3D11Texture2D>, ComPtr<ID3D11VideoProcessorOutputView>>(OutputTexture.Get(), OutputView));
        }

        D3D11_VIDEO_PROCESSOR_STREAM Stream;
        SetZero(Stream);
        Stream.Enable = true;
        Stream.OutputIndex = 0;
        Stream.InputFrameOrField = 0;
        Stream.PastFrames = 0;
        Stream.FutureFrames = 0;
        Stream.ppPastSurfaces = nullptr;
        Stream.pInputSurface = InputViewMap[InTex].Get();
        Stream.ppFutureSurfaces = nullptr;
        Stream.ppPastSurfacesRight = nullptr;
        Stream.pInputSurfaceRight = nullptr;
        Stream.ppFutureSurfacesRight = nullptr;

        Hr = VideoContext->VideoProcessorBlt(VideoProcessor.Get(), OutputViewMap[OutputTexture].Get(), 0, 1, &Stream);
        if (FAILED(Hr))
            return false;

        auto Context = TextureUtil::GetDeviceContext(Device);
        Context->Flush();
        return true;
    }

    void *VideoTextureProcessor::GetOutputTexture() {
        return OutputTexture.Get();
    }

    bool VideoTextureProcessor::ProcessTexture(void *InTex, const int64_t InTexArrayIdx) {
        ComPtr<ID3D11Texture2D> Tex = (ID3D11Texture2D *) InTex;
        return ProcessTexture(Tex, InTexArrayIdx);
    }
}
