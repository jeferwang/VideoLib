#include <cstdint>
#include "D3DVideoProcessor.h"
#include "d3d11.h"

#define CheckRelease(RefPrt) \
    if (RefPrt != nullptr) { \
        RefPrt->Release(); \
        RefPrt = nullptr; \
    }

#define SetZero(Var) memset(&Var, 0, sizeof(Var));


namespace D3D {
    bool VideoTextureProcessor::Initialize(void *InDevicePtr, const uint32_t InWidth, const uint32_t InHeight) {
        if (bInit) return false;

        Width = InWidth;
        Height = InHeight;

        HRESULT Hr = -1;

        Device = (ID3D11Device *) InDevicePtr;
        if (Device == nullptr) return false;
        Device->AddRef();

        Device->GetImmediateContext(&DeviceContext);
        if (DeviceContext == nullptr) return false;
        DeviceContext->AddRef();

        Hr = Device->QueryInterface(_uuidof(ID3D11VideoDevice), (void **) &VideoDevice);
        if (VideoDevice == nullptr || FAILED(Hr)) return false;
        VideoDevice->AddRef();

        Hr = DeviceContext->QueryInterface(__uuidof(ID3D11VideoContext), (void **) &VideoContext);
        if (VideoContext == nullptr || FAILED(Hr)) return false;
        VideoContext->AddRef();

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

        Hr = VideoDevice->CreateVideoProcessorEnumerator(&VideoContentDesc, &VideoProcessorEnumerator);
        if (VideoProcessorEnumerator == nullptr || FAILED(Hr)) return false;
        VideoProcessorEnumerator->AddRef();

        Hr = VideoDevice->CreateVideoProcessor(VideoProcessorEnumerator, 0, &VideoProcessor);
        if (VideoProcessor == nullptr || FAILED(Hr)) return false;
        VideoProcessor->AddRef();

        bInit = true;
        return true;
    }

    VideoTextureProcessor::~VideoTextureProcessor() {
        for (auto &It: InputViewMap) {
            CheckRelease(It.second);
        }
        for (auto &It: OutputViewMap) {
            CheckRelease(It.second);
        }
        CheckRelease(VideoProcessor);
        CheckRelease(VideoProcessorEnumerator);
        CheckRelease(VideoContext);
        CheckRelease(VideoDevice);
        CheckRelease(DeviceContext);
        CheckRelease(Device);
    }

    bool VideoTextureProcessor::ConvertTexture(void *InTex, void *OutTex) {
        HRESULT Hr = -1;

        auto InTexDX11 = (ID3D11Texture2D *) InTex;
        auto OutTexDX11 = (ID3D11Texture2D *) OutTex;

        if (InputViewMap.find(InTex) == InputViewMap.end()) {
            D3D11_VIDEO_PROCESSOR_INPUT_VIEW_DESC InputViewDesc;
            SetZero(InputViewDesc);
            InputViewDesc.FourCC = 0;
            InputViewDesc.ViewDimension = D3D11_VPIV_DIMENSION_TEXTURE2D;
            InputViewDesc.Texture2D = {0, 0};// MipSlice & ArraySlice
            ID3D11VideoProcessorInputView *InputView;
            Hr = VideoDevice->CreateVideoProcessorInputView(InTexDX11, VideoProcessorEnumerator, &InputViewDesc, &InputView);
            if (InputView == nullptr || FAILED(Hr)) return false;
            InputView->AddRef();
            InputViewMap.insert(std::pair<void *, ID3D11VideoProcessorInputView *>(InTex, InputView));
        }

        if (OutputViewMap.find(InTex) == OutputViewMap.end()) {
            D3D11_VIDEO_PROCESSOR_OUTPUT_VIEW_DESC OutputViewDesc = { D3D11_VPOV_DIMENSION_TEXTURE2D };
            // D3D11_VIDEO_PROCESSOR_OUTPUT_VIEW_DESC OutputViewDesc;
            // SetZero(OutputViewDesc);
            // OutputViewDesc.ViewDimension = D3D11_VPOV_DIMENSION_TEXTURE2D;
            ID3D11VideoProcessorOutputView *OutputView;
            Hr = VideoDevice->CreateVideoProcessorOutputView(OutTexDX11, VideoProcessorEnumerator, &OutputViewDesc, &OutputView);
            if (OutputView == nullptr || FAILED(Hr)) return false;
            OutputView->AddRef();
            OutputViewMap.insert(std::pair<void *, ID3D11VideoProcessorOutputView *>(OutTex, OutputView));
        }

        D3D11_VIDEO_PROCESSOR_STREAM Stream;
        SetZero(Stream);
        Stream.Enable = true;
        Stream.OutputIndex = 0;
        Stream.InputFrameOrField = 0;
        Stream.PastFrames = 0;
        Stream.FutureFrames = 0;
        Stream.ppPastSurfaces = nullptr;
        Stream.pInputSurface = InputViewMap[InTex];
        Stream.ppFutureSurfaces = nullptr;
        Stream.ppPastSurfacesRight = nullptr;
        Stream.pInputSurfaceRight = nullptr;
        Stream.ppFutureSurfacesRight = nullptr;

        Hr = VideoContext->VideoProcessorBlt(VideoProcessor, OutputViewMap[OutTex], 0, 1, &Stream);
        return FAILED(Hr) ? false : true;
    }
}
