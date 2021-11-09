#pragma once

#include "Base.h"

namespace D3D {
    class D3D_UTILS_API VideoTextureFormat {
        void NV12ToRGBA(void *InNV12, void **OutRGBA);
    };
}


