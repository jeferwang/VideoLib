#pragma once

#include <memory>
#include <thread>
#include <functional>
#include <set>
#include <vector>
#include <mutex>

extern "C" {
#include "libavformat/avformat.h"
#include "libavutil/error.h"
#include "libavutil/pixdesc.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
}


// fix c++ av_err2str compile error
#undef  av_err2str
#define av_err2str(errnum) av_make_error_string((char*)_malloca(AV_ERROR_MAX_STRING_SIZE), AV_ERROR_MAX_STRING_SIZE, errnum)

#define AssertBreak(condition, ...) \
            if (!(condition)) { \
                Log::Write(__VA_ARGS__); \
                break;\
            }

#define AssertRet(condition, ...) \
            if (!(condition)) { \
                Log::Write(__VA_ARGS__); \
                return;\
            }

typedef std::function<void(void *ImageData, int32_t FrameIndex, int64_t FrameTime, int32_t Width, int32_t Height, int32_t LineSize)> VideoFrameUpdateCallback;
// typedef void(*VideoFrameUpdateCallback)(void *ImageData, int32_t FrameIndex, int64_t FrameTime);

namespace VP {

    using std::chrono::duration;
    using std::chrono::time_point;
    using std::chrono::steady_clock;

    class DllExport Player final {
    public:
        ~Player();

        // 初始化，打开指定视频文件输入，重复调用无效
        bool Init(const char *InUrl);

        // 执行清理，可重复调用
        void UnInit();

        bool Play();

        void Pause();

        void SetLoop(bool InLoop);

        void SetFrameUpdateCallback(const VideoFrameUpdateCallback &InCallback);

        int32_t GetWidth() const;

        int32_t GetHeight() const;

    private:

        void DoDecode();

        static AVCodec *DetectCodec(AVCodecID CodecID);

        static AVStream *FindVideoStream(AVFormatContext *InFormatContext);

    private:
        bool Init_ = false;
        std::shared_ptr<std::thread> TimerThread_ = nullptr;
        std::shared_ptr<std::thread> DecodeThread_ = nullptr;
        std::vector<VideoFrameUpdateCallback> FrameUpdateCallbacks_;

        // Playback Control
        bool Start_ = false;// 解码状态，启动和停止
        bool Playing_ = false;// 播放状态，播放和暂停
        bool Loop_ = false;
        duration<double> PlaybackTime_ = duration<double>(0);// 播放进度
        std::mutex PlaybackTimeMutex_;

        // Decode params
        AVFormatContext *FormatContext_ = nullptr;
        AVStream *Stream_ = nullptr;
        AVCodecContext *CodecContext_ = nullptr;
        AVPacket *Packet_ = nullptr;
        AVFrame *Frame_ = nullptr;
        SwsContext *SwsContext_ = nullptr;
        int FrameWidth_ = 0;
        int FrameHeight_ = 0;
        int64_t FramePTS_ = 0;
        uint8_t *ImageData_[4] = {nullptr, nullptr, nullptr, nullptr};
        int ImageLineSize[4] = {0, 0, 0, 0};

        void PlaybackTimer();
    };
}
