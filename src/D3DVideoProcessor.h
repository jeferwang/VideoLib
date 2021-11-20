#pragma once

#include <map>
#include "wrl/client.h"
#include "Base.h"

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11VideoDevice;
struct ID3D11VideoContext;
struct ID3D11VideoProcessorEnumerator;
struct ID3D11VideoProcessor;
struct ID3D11VideoProcessorInputView;
struct ID3D11VideoProcessorOutputView;
struct ID3D11Texture2D;
namespace XGraphic {

    using Microsoft::WRL::ComPtr;

    class D3D_UTILS_API VideoTextureProcessor final {
    public:
        ~VideoTextureProcessor() = default;

        bool Initialize(const ComPtr<ID3D11Device> &InDevice, uint32_t InWidth, uint32_t InHeight);

        bool ProcessTexture(void *InTex, int64_t InTexArrayIdx);

        bool ProcessTexture(const ComPtr<ID3D11Texture2D> &InTex, int64_t InTexArrayIdx);

        void *GetOutputTexture();

    private:
        bool bInit = false;
        uint32_t Width = 0;
        uint32_t Height = 0;
        ComPtr<ID3D11Texture2D> OutputTexture;
        ComPtr<ID3D11Device> Device;
        ComPtr<ID3D11DeviceContext> DeviceContext;
        ComPtr<ID3D11VideoDevice> VideoDevice;
        ComPtr<ID3D11VideoContext> VideoContext;
        ComPtr<ID3D11VideoProcessorEnumerator> VideoProcessorEnumerator;
        ComPtr<ID3D11VideoProcessor> VideoProcessor;
        std::map<ComPtr<ID3D11Texture2D>, ComPtr<ID3D11VideoProcessorInputView>> InputViewMap;
        std::map<ComPtr<ID3D11Texture2D>, ComPtr<ID3D11VideoProcessorOutputView>> OutputViewMap;
    };
}

