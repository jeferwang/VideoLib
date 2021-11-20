#pragma once

#include <memory>
#include "Base.h"
#include "d3d11.h"
#include "wrl/client.h"

namespace XGraphic {
    class VideoTextureProcessor;

    using Microsoft::WRL::ComPtr;

    class D3D_UTILS_API TextureUtil {
    public:
        static ComPtr<ID3D11Device> GetDevice(void *InTex);

        static ComPtr<ID3D11Device> GetDevice(const ComPtr<ID3D11Texture2D> &InTex);

        static ComPtr<ID3D11DeviceContext> GetDeviceContext(const ComPtr<ID3D11Device> &InDevice);

        static ComPtr<ID3D11DeviceContext> GetDeviceContext(const ComPtr<ID3D11Texture2D> &InTex);

        static bool CopyTexture(void *InDstTex, void *InSrcTex);

        static bool CopyTexture(const ComPtr<ID3D11Texture2D> &InDstTex, const ComPtr<ID3D11Texture2D> &InSrcTex);

        static bool FlushSharedTexture(void *InSharedTexture);

        static bool FlushSharedTexture(const ComPtr<ID3D11Texture2D> &InSharedTexture);

        static ComPtr<ID3D11Texture2D> ShareToDevice(void *InSrcTexture, void *InTargetDevice);

        static ComPtr<ID3D11Texture2D> ShareToDevice(const ComPtr<ID3D11Texture2D> &InSrcTexture, const ComPtr<ID3D11Device> &InTargetDevice);

        static std::shared_ptr<VideoTextureProcessor> CreateVideoTextureProcessor(const ComPtr<ID3D11Device> &InDevice, uint32_t InWidth, uint32_t InHeight);

        static ComPtr<ID3D11Texture2D> CreateVideoProcessOutputTexture_RGBA(const ComPtr<ID3D11Device> &InDevice, uint32_t InWidth, uint32_t InHeight);

        static ComPtr<ID3D11Texture2D> CreateStageTexture_RGBA(const ComPtr<ID3D11Device> &InDevice,uint32_t InWidth, uint32_t InHeight);

        static std::shared_ptr<uint8_t> CopyToMem(void *InTexPrt, uint32_t &OutLength, uint32_t &OutWidth, uint32_t &OutHeight);
    };
}


