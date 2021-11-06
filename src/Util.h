#pragma once

namespace VP {
    enum class GpuType {
        UnKnown = 0,
        NVIDIA = 1,
        AMD = 2,
    };

    class Util {
    public:
        static GpuType DetectGPU();
    };
}