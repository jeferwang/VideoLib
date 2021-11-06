#include "Util.h"
#include "dxgi.h"
#include "d3d9.h"

namespace VP {
    GpuType Util::DetectGPU() {
        GpuType Type = GpuType::UnKnown;

        LPDIRECT3D9 g_pD3D = nullptr;
        if (nullptr != (g_pD3D = Direct3DCreate9(D3D_SDK_VERSION))) {
            D3DADAPTER_IDENTIFIER9 D3DAdapterIdentifier;
            g_pD3D->GetAdapterIdentifier(0, 0, &D3DAdapterIdentifier);
            // Detect GPU type, Data Source is https://pci-ids.ucw.cz/read/PC?restrict=1
            switch (D3DAdapterIdentifier.VendorId) {
                case 0x10de:
                case 0x12d2:
                    Type = GpuType::NVIDIA;
                    break;
                case 0x1002:
                case 0x1022:
                case 0x1206:
                    Type = GpuType::AMD;
                    break;
                default:
                    Type = GpuType::UnKnown;
                    break;
            }
            g_pD3D->Release();
            g_pD3D = nullptr;
        }
        return Type;
    }
}
