#include <cstdint>
#include <memory>
#include "D3DUtils.h"
#include "d3d11.h"
#include "D3DVideoProcessor.h"

namespace D3D {
    void TextureUtil::NV12ToRGBA(void *InNV12, void **OutRGBA) {
        HRESULT Hr;

        auto NV12Tex = (ID3D11Texture2D *) InNV12;
        ID3D11Device *Device = nullptr;
        NV12Tex->GetDevice(&Device);

        D3D11_TEXTURE2D_DESC NV12Desc;
        NV12Tex->GetDesc(&NV12Desc);


        D3D11_TEXTURE2D_DESC RGBADesc;
        memset(&RGBADesc, 0, sizeof(RGBADesc));
        RGBADesc.Width = NV12Desc.Width;
        RGBADesc.Height = NV12Desc.Height;
        RGBADesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        RGBADesc.SampleDesc.Count = 1;
        RGBADesc.SampleDesc.Quality = 0;
        RGBADesc.Usage = D3D11_USAGE_DEFAULT;
        RGBADesc.MipLevels = 1;
        RGBADesc.ArraySize = 1;
        RGBADesc.BindFlags = D3D11_BIND_RENDER_TARGET;// D3D11_BIND_SHADER_RESOURCE
        RGBADesc.CPUAccessFlags = 0;
        RGBADesc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;// D3D11_RESOURCE_MISC_SHARED

        const auto RGBARowPitch = RGBADesc.Width * 4 * sizeof(uint8_t);
        const auto RGBABufSize = RGBARowPitch * RGBADesc.Height;
        auto RGBABuf = new uint8_t[RGBABufSize];
        memset(RGBABuf, 0, RGBABufSize * sizeof(uint8_t));
        D3D11_SUBRESOURCE_DATA RGBAInitData;
        memset(&RGBAInitData, 0, sizeof(RGBAInitData));
        RGBAInitData.pSysMem = RGBABuf;
        RGBAInitData.SysMemPitch = RGBARowPitch;
        RGBAInitData.SysMemSlicePitch = 0;

        ID3D11Texture2D *RGBATex = nullptr;
        Hr = Device->CreateTexture2D(&RGBADesc, &RGBAInitData, &RGBATex);
        delete[] RGBABuf;
        if (FAILED(Hr)) return;

        auto Processor = new VideoTextureProcessor();
        if (!Processor->Initialize(Device, NV12Desc.Width, NV12Desc.Height))return;
        auto Ret = Processor->ConvertTexture(NV12Tex, RGBATex);
        delete Processor;
    }

    std::shared_ptr<uint8_t> TextureUtil::CopyToMem(void *InTexPrt, uint32_t &OutLength, uint32_t &OutWidth, uint32_t &OutHeight) {
        OutLength = 0;
        OutWidth = 0;
        OutHeight = 0;
        HRESULT Hr = -1;

        auto Tex = (ID3D11Texture2D *) InTexPrt;
        D3D11_TEXTURE2D_DESC TexDesc;
        Tex->GetDesc(&TexDesc);

        {
            ID3D11Device *Device = nullptr;
            Tex->GetDevice(&Device);
            if (Device == nullptr) return nullptr;
            Device->AddRef();
            {
                ID3D11DeviceContext *Context = nullptr;
                Device->GetImmediateContext(&Context);
                if (Context == nullptr) return nullptr;
                Context->AddRef();
                {
                    D3D11_MAPPED_SUBRESOURCE MappedResource;
                    Hr = Context->Map(Tex, 0, D3D11_MAP_READ, 0, &MappedResource);
                    if (FAILED(Hr)) return nullptr;
                    {
                        OutLength = MappedResource.RowPitch * TexDesc.Height;
                        OutWidth = TexDesc.Width;
                        OutHeight = TexDesc.Height;
                        std::shared_ptr<uint8_t> Data(new uint8_t[OutLength],[](uint8_t *p){delete[] p;});
                        memcpy(Data.get(),MappedResource.pData,OutLength)
                        // todo 临时提交
                    }
                    Context->Unmap(Tex, 0);
                }
                Context->Release();
            }

            if (Device != nullptr) Device->Release();
        }
    }
}
