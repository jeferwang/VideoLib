#pragma once

#include "Base.h"

namespace D3D {
    class D3D_UTILS_API TextureUtil {
    public:
        static void NV12ToRGBA(void *InNV12, void **OutRGBA);

        static void CopyToMem(void *InTexPrt, uint8_t **OutPtr, uint32_t &OutLength,uint32_t& OutWidth,uint32_t& OutHeight);
    };
}


