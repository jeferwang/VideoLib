#include "D3DUtils.h"
#include "d3d11.h"

namespace D3D {
    void VideoTextureFormat::NV12ToRGBA(void *InNV12, void **OutRGBA) {
        auto Nv12Tex = (ID3D11Texture2D *) InNV12;
        D3D11_TEXTURE2D_DESC Desc;
        Nv12Tex->GetDesc(&Desc);
    }
}
