#pragma once

#include "Base.h"
#include <map>

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11VideoDevice;
struct ID3D11VideoContext;
struct ID3D11VideoProcessorEnumerator;
struct ID3D11VideoProcessor;
struct ID3D11VideoProcessorInputView;
struct ID3D11VideoProcessorOutputView;
namespace D3D {
    class D3D_UTILS_API VideoTextureProcessor final {
    public:
        ~VideoTextureProcessor();

        bool Initialize(void *InDevicePtr, uint32_t InWidth, uint32_t InHeight);

        bool ConvertTexture(void *InTex, void *OutTex);

    private:
        bool bInit = false;
        uint32_t Width = 0;
        uint32_t Height = 0;
        ID3D11Device *Device = nullptr;
        ID3D11DeviceContext *DeviceContext = nullptr;
        ID3D11VideoDevice *VideoDevice = nullptr;
        ID3D11VideoContext *VideoContext = nullptr;
        ID3D11VideoProcessorEnumerator *VideoProcessorEnumerator = nullptr;
        ID3D11VideoProcessor *VideoProcessor = nullptr;
        std::map<void *, ID3D11VideoProcessorInputView *> InputViewMap;
        std::map<void *, ID3D11VideoProcessorOutputView *> OutputViewMap;
    };
}

