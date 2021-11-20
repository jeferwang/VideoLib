#include <cstdint>
#include <memory>
#include <vector>
#include "d3d11.h"
#include "D3DUtils.h"
#include "D3DVideoProcessor.h"

namespace XGraphic {
    // void TextureUtil::NV12ToRGBA(void *InNV12, void **OutRGBA) {
    //     auto Processor = new VideoTextureProcessor();
    //
    //     HRESULT Hr;
    //
    //     auto NV12Tex = (ID3D11Texture2D *) InNV12;
    //     ID3D11Device *Device = nullptr;
    //     NV12Tex->GetDevice(&Device);
    //
    //     D3D11_TEXTURE2D_DESC NV12Desc;
    //     NV12Tex->GetDesc(&NV12Desc);
    //
    //
    //     // D3D11_TEXTURE2D_DESC RGBADesc;
    //     // memset(&RGBADesc, 0, sizeof(RGBADesc));
    //     // RGBADesc.Width = NV12Desc.Width;
    //     // RGBADesc.Height = NV12Desc.Height;
    //     // RGBADesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    //     // RGBADesc.SampleDesc.Count = 1;
    //     // RGBADesc.SampleDesc.Quality = 0;
    //     // RGBADesc.Usage = D3D11_USAGE_STAGING;//D3D11_USAGE_DEFAULT
    //     // RGBADesc.MipLevels = 1;
    //     // RGBADesc.ArraySize = 1;
    //     // RGBADesc.BindFlags = 0;// D3D11_BIND_RENDER_TARGET D3D11_BIND_SHADER_RESOURCE
    //     // RGBADesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    //     // RGBADesc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;// 支持共享给其他Device
    //     //
    //     // const auto RGBARowPitch = RGBADesc.Width * 4 * sizeof(uint8_t);
    //     // const auto RGBABufSize = RGBARowPitch * RGBADesc.Height;
    //     // auto RGBABuf = new uint8_t[RGBABufSize];
    //     // memset(RGBABuf, 0, RGBABufSize * sizeof(uint8_t));
    //     // D3D11_SUBRESOURCE_DATA RGBAInitData;
    //     // memset(&RGBAInitData, 0, sizeof(RGBAInitData));
    //     // RGBAInitData.pSysMem = RGBABuf;
    //     // RGBAInitData.SysMemPitch = RGBARowPitch;
    //     // RGBAInitData.SysMemSlicePitch = 0;
    //     //
    //     // ID3D11Texture2D *RGBATex = nullptr;
    //     // Hr = Device->CreateTexture2D(&RGBADesc, &RGBAInitData, &RGBATex);
    //     // delete[] RGBABuf;
    //     // if (FAILED(Hr)) return;
    //
    //     if (!Processor->Initialize(Device, NV12Desc.Width, NV12Desc.Height))return;
    //     // auto Ret = Processor->ProcessTexture(NV12Tex, RGBATex);
    //     // delete Processor;
    //     // if (!Ret) return;
    //     // *OutRGBA = RGBATex;
    // }

    std::shared_ptr<uint8_t> TextureUtil::CopyToMem(void *InTexPrt, uint32_t &OutLength, uint32_t &OutWidth, uint32_t &OutHeight) {
        OutLength = 0;
        OutWidth = 0;
        OutHeight = 0;
        HRESULT Hr = -1;

        std::shared_ptr<uint8_t> Data = nullptr;

        auto Tex = (ID3D11Texture2D *) InTexPrt;
        D3D11_TEXTURE2D_DESC TexDesc;
        Tex->GetDesc(&TexDesc);

        const auto Device = GetDevice(InTexPrt);
        const auto Context = GetDeviceContext(Device);

        D3D11_MAPPED_SUBRESOURCE MappedResource;
        Hr = Context->Map(Tex, 0, D3D11_MAP_READ, 0, &MappedResource);
        if (FAILED(Hr))
            return nullptr;
        OutLength = MappedResource.RowPitch * TexDesc.Height;
        OutWidth = TexDesc.Width;
        OutHeight = TexDesc.Height;
        Data = std::shared_ptr<uint8_t>(new uint8_t[OutLength], [](const uint8_t *p) { delete[] p; });
        memcpy(Data.get(), MappedResource.pData, OutLength);
        Context->Unmap(Tex, 0);

        return Data;
    }

    ComPtr<ID3D11Device> TextureUtil::GetDevice(const ComPtr<ID3D11Texture2D> &InTex) {
        if (InTex == nullptr) return nullptr;
        ComPtr<ID3D11Device> Device = nullptr;
        InTex->GetDevice(Device.GetAddressOf());
        return Device;
    }

    ComPtr<ID3D11Device> TextureUtil::GetDevice(void *InTex) {
        ComPtr<ID3D11Texture2D> Tex = (ID3D11Texture2D *) InTex;
        return GetDevice(Tex);
    }

    std::shared_ptr<VideoTextureProcessor> TextureUtil::CreateVideoTextureProcessor(const ComPtr<ID3D11Device> &InDevice, const uint32_t InWidth, const uint32_t InHeight) {
        auto Processor = std::make_shared<VideoTextureProcessor>();
        if (!Processor->Initialize(InDevice, InWidth, InHeight)) return nullptr;
        return Processor;
    }

    ComPtr<ID3D11Texture2D> TextureUtil::CreateVideoProcessOutputTexture_RGBA(const ComPtr<ID3D11Device> &InDevice, const uint32_t InWidth, const uint32_t InHeight) {
        ComPtr<ID3D11Texture2D> OutTex = nullptr;
        HRESULT Hr = -1;

        D3D11_TEXTURE2D_DESC Desc;
        memset(&Desc, 0, sizeof(Desc));
        Desc.Width = InWidth;
        Desc.Height = InHeight;
        Desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        Desc.SampleDesc.Count = 1;
        Desc.SampleDesc.Quality = 0;
        Desc.Usage = D3D11_USAGE_DEFAULT;
        Desc.MipLevels = 1;
        Desc.ArraySize = 1;
        Desc.BindFlags = D3D11_BIND_RENDER_TARGET;// VideoOutput必须使用RenderTarget
        Desc.CPUAccessFlags = 0;
        Desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;

        const auto RowPitch = Desc.Width * 4 * sizeof(uint8_t);
        const auto BufSize = RowPitch * Desc.Height;
        std::vector<uint8_t> Buf(BufSize);
        D3D11_SUBRESOURCE_DATA InitData;
        memset(&InitData, 0, sizeof(InitData));
        InitData.pSysMem = Buf.data();
        InitData.SysMemPitch = RowPitch;
        InitData.SysMemSlicePitch = 0;

        Hr = InDevice->CreateTexture2D(&Desc, &InitData, OutTex.GetAddressOf());
        if (FAILED(Hr))
            return nullptr;
        return OutTex;
    }

    ComPtr<ID3D11Texture2D> TextureUtil::ShareToDevice(void *InSrcTexture, void *InTargetDevice) {
        ComPtr<ID3D11Device> TargetDevice = (ID3D11Device *) InTargetDevice;
        ComPtr<ID3D11Texture2D> SrcTexture = (ID3D11Texture2D *) InSrcTexture;
        return ShareToDevice(SrcTexture, TargetDevice);
    }

    ComPtr<ID3D11Texture2D> TextureUtil::ShareToDevice(const ComPtr<ID3D11Texture2D> &InSrcTexture, const ComPtr<ID3D11Device> &InTargetDevice) {
        if (InSrcTexture == nullptr || InTargetDevice == nullptr) return nullptr;

        HRESULT Hr = -1;

        D3D11_TEXTURE2D_DESC Desc;
        InSrcTexture->GetDesc(&Desc);
        if (!(Desc.MiscFlags & D3D11_RESOURCE_MISC_SHARED)) return nullptr;

        // 先转换成DXGIResource
        ComPtr<IDXGIResource> DXGIRes;
        Hr = InSrcTexture->QueryInterface(__uuidof(IDXGIResource), (void **) DXGIRes.GetAddressOf());
        if (FAILED(Hr) || DXGIRes == nullptr) return nullptr;

        // 取得共享的Handle
        HANDLE SharedHandle;
        Hr = DXGIRes->GetSharedHandle(&SharedHandle);
        if (FAILED(Hr) || SharedHandle == nullptr) return nullptr;

        // 用另一个Device打开SharedHandle
        ComPtr<ID3D11Texture2D> TargetDeviceTexture;
        Hr = InTargetDevice->OpenSharedResource(SharedHandle, __uuidof(ID3D11Texture2D), (void **) TargetDeviceTexture.GetAddressOf());
        if (FAILED(Hr)) return nullptr;

        // SrcTexture对应的DeviceContext调用Flush方法之后，Share出去的Texture才会更新
        return TargetDeviceTexture;
    }

    bool TextureUtil::FlushSharedTexture(void *InSharedTexture) {
        ComPtr<ID3D11Texture2D> SharedTexture = (ID3D11Texture2D *) InSharedTexture;
        return FlushSharedTexture(SharedTexture);
    }

    bool TextureUtil::FlushSharedTexture(const ComPtr<ID3D11Texture2D> &InSharedTexture) {
        ComPtr<ID3D11DeviceContext> Context = GetDeviceContext(InSharedTexture);
        if (Context == nullptr) return false;

        Context->Flush();
        return true;
    }

    bool TextureUtil::CopyTexture(void *InDstTex, void *InSrcTex) {
        ComPtr<ID3D11Texture2D> SrcTex = (ID3D11Texture2D *) InSrcTex;
        ComPtr<ID3D11Texture2D> DstTex = (ID3D11Texture2D *) InDstTex;
        return CopyTexture(DstTex, SrcTex);
    }

    bool TextureUtil::CopyTexture(const ComPtr<ID3D11Texture2D> &InDstTex, const ComPtr<ID3D11Texture2D> &InSrcTex) {
        if (InSrcTex == nullptr || InDstTex == nullptr)
            return false;

        ComPtr<ID3D11DeviceContext> DstContext = GetDeviceContext(InDstTex);
        if (DstContext == nullptr)
            return false;

        DstContext->CopyResource(InDstTex.Get(), InSrcTex.Get());
        return true;
    }

    ComPtr<ID3D11DeviceContext> TextureUtil::GetDeviceContext(const ComPtr<ID3D11Device> &InDevice) {
        if (InDevice == nullptr) return nullptr;
        ComPtr<ID3D11DeviceContext> Context;
        InDevice->GetImmediateContext(Context.GetAddressOf());
        return Context;
    }

    ComPtr<ID3D11DeviceContext> TextureUtil::GetDeviceContext(const ComPtr<ID3D11Texture2D> &InTex) {
        auto Device = GetDevice(InTex);
        return GetDeviceContext(Device);
    }

    ComPtr<ID3D11Texture2D> TextureUtil::CreateStageTexture_RGBA(const ComPtr<ID3D11Device> &InDevice, const uint32_t InWidth, const uint32_t InHeight) {
        ComPtr<ID3D11Texture2D> OutTex = nullptr;
        HRESULT Hr = -1;

        D3D11_TEXTURE2D_DESC Desc;
        memset(&Desc, 0, sizeof(Desc));
        Desc.Width = InWidth;
        Desc.Height = InHeight;
        Desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        Desc.SampleDesc.Count = 1;
        Desc.SampleDesc.Quality = 0;
        Desc.Usage = D3D11_USAGE_STAGING;
        Desc.MipLevels = 1;
        Desc.ArraySize = 1;
        Desc.BindFlags = 0;
        Desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        Desc.MiscFlags = 0;

        const auto RowPitch = Desc.Width * 4 * sizeof(uint8_t);
        const auto BufSize = RowPitch * Desc.Height;
        std::vector<uint8_t> Buf(BufSize);
        D3D11_SUBRESOURCE_DATA InitData;
        memset(&InitData, 0, sizeof(InitData));
        InitData.pSysMem = Buf.data();
        InitData.SysMemPitch = RowPitch;
        InitData.SysMemSlicePitch = 0;

        Hr = InDevice->CreateTexture2D(&Desc, &InitData, OutTex.GetAddressOf());
        return FAILED(Hr) ? nullptr : OutTex;
    }


    // bool TextureUtil::CopyDifferentDeviceTexture(void *InSrcSharedTex, void *InDstTex) {
    //     ComPtr<ID3D11Texture2D> SrcSharedTex = (ID3D11Texture2D *) InSrcSharedTex;
    //     ComPtr<ID3D11Texture2D> DstTex = (ID3D11Texture2D *) InDstTex;
    //     return CopyDifferentDeviceTexture(SrcSharedTex, DstTex);
    // }

    // bool TextureUtil::CopyDifferentDeviceTexture(const ComPtr<ID3D11Texture2D> &InSrcSharedTex, const ComPtr<ID3D11Texture2D> &InDstTex) {
    //     auto DstDevice= GetDevice()
    // }
}
