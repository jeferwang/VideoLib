#pragma once

#include "Base.h"
#include <memory>
#include <thread>
#include <functional>
#include <vector>
#include <mutex>

// typedef void(*VideoFrameUpdateCallback)(void *ImageData, int32_t FrameIndex, int64_t FrameTime);
typedef std::function<void(void *ImageData, int64_t FrameIndex, int64_t FrameTime, int32_t Width, int32_t Height)> VideoFrameUpdateCallback;

struct AVFormatContext;
struct AVStream;
struct AVCodec;
struct AVCodecContext;
struct AVCodecParserContext;
struct AVPacket;
struct AVFrame;
struct SwsContext;
struct AVBufferRef;

namespace VP {

    using std::chrono::duration;
    using std::chrono::time_point;
    using std::chrono::steady_clock;

    class VP_DLL_EXPORT Player final {
    public:
        Player();

        ~Player();

        // 初始化，打开指定视频文件输入，重复调用无效
        bool Init(const char *InUrl, uint8_t InAVHWDeviceType);

        // 执行清理，可重复调用
        void UnInit();

        bool Play();

        void Pause();

        void SetLoop(bool InLoop);

        void SetFrameUpdateCallback(const VideoFrameUpdateCallback &InCallback);

        int32_t GetWidth() const;

        int32_t GetHeight() const;

        bool IsUseHwDevice() const;

    private:
        void DoDecode();

        void PlaybackTimer();

        static AVCodec *DetectCodec(int32_t CodecID);

        static AVStream *FindVideoStream(AVFormatContext *InFormatContext);

        int32_t GetHwFormat(AVCodecContext *InCodecContext, const int32_t *InPixFormats);

    private:
        bool Init_ = false;
        std::vector<VideoFrameUpdateCallback> FrameUpdateCallbacks_;

        // 硬解
        bool UseHwDevice = false;
        uint8_t HWDeviceType = 0;//AVHWDeviceType
        int32_t HwPixFmt = -1; // AVPixelFormat
        AVBufferRef *HwDeviceCtx = nullptr;
        // AVFrame *HwFrame_ = nullptr;

        // 解码线程
        std::shared_ptr<std::thread> DecodeThread_ = nullptr;
        std::mutex PlaybackTimeMutex_; // 修改PlaybackTime的时候需要加锁
        std::shared_ptr<std::thread> TimerThread_ = nullptr;

        // Playback Control
        bool Start_ = false;// 解码状态，启动和停止
        bool Playing_ = false;// 播放状态，播放和暂停
        bool Loop_ = false;
        duration<double> PlaybackTime_ = duration<double>(0);// 播放进度

        // Decode params
        AVFormatContext *FormatContext_ = nullptr;
        AVStream *Stream_ = nullptr;
        AVCodecContext *CodecContext_ = nullptr;
        AVPacket *Packet_ = nullptr;
        AVFrame *Frame_ = nullptr;
        SwsContext *SwsContext_ = nullptr;
        int FrameWidth_ = 0;
        int FrameHeight_ = 0;
        int64_t FramePTS_ = 0;// 单位：微秒
        uint8_t *ImageData_[4] = {nullptr, nullptr, nullptr, nullptr};
        int ImageLineSize[4] = {0, 0, 0, 0};
    };
}
