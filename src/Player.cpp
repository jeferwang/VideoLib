#include "Player.h"
#include "Util.h"
#include "Logger.h"
#include <memory>
#include <thread>
#include <functional>
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
// #define AssertRet(condition, ...) \
//             if (!(condition)) { \
//                 Log::Write(__VA_ARGS__); \
//                 return;\
//             }

using VP::Logger::Log;
using VP::Logger::LogLevel;

namespace VP {
    static AVPixelFormat get_hw_format(AVCodecContext *ctx, const AVPixelFormat *pix_fmts) {
        const AVPixelFormat HwPixFmt = *((AVPixelFormat *) (ctx->opaque));
        const AVPixelFormat *p;
        for (p = pix_fmts; *p != -1; p++) {
            if (*p == HwPixFmt) return *p;
        }
        Log::Write(LogLevel::Error, "Failed to get HW surface format");
        return AV_PIX_FMT_NONE;
    }

    Player::Player() = default;

    Player::~Player() {
        UnInit();
    }

    bool Player::Init(const char *InUrl, const uint8_t InAVHWDeviceType) {
        if (Init_) return true;

        HWDeviceType = InAVHWDeviceType;

        do {
            if (FormatContext_ != nullptr) {
                avformat_close_input(&FormatContext_);
            }

            if (CodecContext_ != nullptr) {
                avcodec_free_context(&CodecContext_);
            }

            int ErrNum;

            // Open video file and read stream info
            ErrNum = avformat_open_input(&FormatContext_, InUrl, nullptr, nullptr);
            AssertBreak(ErrNum >= 0, LogLevel::Fatal, "avformat_open_input failed error=%d, msg=%s", ErrNum, av_err2str(ErrNum));
            ErrNum = avformat_find_stream_info(FormatContext_, nullptr);
            AssertBreak(ErrNum >= 0, LogLevel::Fatal, "avformat_find_stream_info failed error=%d, msg=%s", ErrNum, av_err2str(ErrNum));

            AVCodec *StreamCodec = nullptr;
            const int StreamIndex = av_find_best_stream(FormatContext_, AVMEDIA_TYPE_VIDEO, -1, -1, &StreamCodec, 0);
            AssertBreak(StreamIndex >= 0, LogLevel::Fatal, "av_find_best_stream failed error=%d, msg=%s", StreamIndex, av_err2str(StreamIndex));

            // Find Video Stream_
            Stream_ = FormatContext_->streams[StreamIndex];// 使用av_find_best_stream的结果来查找Stream
            // Stream_ = FindVideoStream(FormatContext_); // 查找第一个VideoStream
            AssertBreak(Stream_ != nullptr, LogLevel::Fatal, "Cannot Find VideoStream");

            // Detect Codec
            AVCodec *Codec = nullptr;
            UseHwDevice = InAVHWDeviceType != AVHWDeviceType::AV_HWDEVICE_TYPE_NONE;
            if (InAVHWDeviceType == AVHWDeviceType::AV_HWDEVICE_TYPE_NONE) {
                Codec = DetectCodec(Stream_->codecpar->codec_id);
                AssertBreak(Codec != nullptr, LogLevel::Fatal, "Cannot Find Stream_ Codec");
            } else {
                for (int32_t i = 0;; i++) {
                    Codec = StreamCodec;
                    const AVCodecHWConfig *Config = avcodec_get_hw_config(StreamCodec, i);
                    AssertBreak(Config, LogLevel::Fatal,
                                "Decoder %s does not support device type %s",
                                StreamCodec->name, av_hwdevice_get_type_name((AVHWDeviceType) InAVHWDeviceType));
                    if (Config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX
                        && Config->device_type == InAVHWDeviceType) {
                        HwPixFmt = Config->pix_fmt;
                        break;// Decoder Found.
                    }
                }
            }


            // Create Codec Context
            CodecContext_ = avcodec_alloc_context3(Codec);
            AssertBreak(CodecContext_ != nullptr, LogLevel::Fatal, "Cannot Find Stream_ Codec");

            // Init Codec Context From Stream_ Parameter
            ErrNum = avcodec_parameters_to_context(CodecContext_, Stream_->codecpar);
            AssertBreak(ErrNum >= 0, LogLevel::Fatal,
                        "avcodec_parameters_to_context init CodecContext_ failed error=%d, msg=%s",
                        ErrNum, av_err2str(ErrNum));

            if (UseHwDevice) {
                CodecContext_->opaque = &HwPixFmt;
                CodecContext_->get_format = get_hw_format;
                ErrNum = av_hwdevice_ctx_create(&HwDeviceCtx, (AVHWDeviceType) InAVHWDeviceType, nullptr, nullptr, 0);
                AssertBreak(ErrNum >= 0, LogLevel::Fatal,
                            "av_hwdevice_ctx_create Failed to create specified HW device error=%d, msg=%s",
                            ErrNum, av_err2str(ErrNum));
                CodecContext_->hw_device_ctx = HwDeviceCtx;
            }

            // Initialize AVCodecContext use given AVCodec
            ErrNum = avcodec_open2(CodecContext_, Codec, nullptr);
            AssertBreak(ErrNum >= 0, LogLevel::Fatal, "avcodec_open2 Initialize the AVCodecContext to use the given AVCodec failed error=%d, msg=%s",
                        ErrNum, av_err2str(ErrNum));

            // Print Video Info
            Log::Write(LogLevel::Info,
                       "VideoInfo: InFile=%s, Format=%s, CodecID=%s, Codec=%s, HwPixFmt=%s, Size=%dx%d, Length=%us, Fps=%f, FrameNum=%d",
                       InUrl, FormatContext_->iformat->name, avcodec_get_name(Stream_->codecpar->codec_id), Codec->name,
                       av_get_pix_fmt_name(CodecContext_->pix_fmt),
                       Stream_->codecpar->width, Stream_->codecpar->height,
                       av_rescale_q(Stream_->duration, Stream_->time_base, {1, 1000}) / 1000,
                       av_q2d(Stream_->avg_frame_rate),
                       Stream_->nb_frames);

            // Init Packet_
            if (Packet_ == nullptr) Packet_ = av_packet_alloc();
            Packet_->data = nullptr;
            Packet_->size = 0;

            // Init Frame_ and FrameSize
            if (Frame_ == nullptr) Frame_ = av_frame_alloc();
            // if (UseHwDevice && HwFrame_ == nullptr) HwFrame_ = av_frame_alloc();

            // Init Image Data
            FrameWidth_ = CodecContext_->width;
            FrameHeight_ = CodecContext_->height;
            if (ImageData_[0] != nullptr) av_freep(ImageData_);
            ErrNum = av_image_alloc(ImageData_, ImageLineSize, FrameWidth_, FrameHeight_, AV_PIX_FMT_RGBA, 1);
            AssertBreak(ErrNum >= 0, LogLevel::Fatal, "av_image_alloc failed error=%d, msg=%s", ErrNum, av_err2str(ErrNum));

            // Init SwsContext
            if (SwsContext_ != nullptr) sws_freeContext(SwsContext_);
            SwsContext_ = sws_getContext(
                    FrameWidth_, FrameHeight_, CodecContext_->pix_fmt,//src
                    FrameWidth_, FrameHeight_, AV_PIX_FMT_RGBA,//dst
                    SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
            AssertBreak(SwsContext_ != nullptr, LogLevel::Fatal, "sws_getContext failed");

            Init_ = true;

        } while (false);

        if (!Init_) {
            UnInit(); // clean
        }

        return Init_;
    }

    void Player::UnInit() {
        Start_ = false;
        Playing_ = false;

        if (DecodeThread_) {
            DecodeThread_->join();
            DecodeThread_ = nullptr;
        }
        if (TimerThread_) {
            TimerThread_->join();
            TimerThread_ = nullptr;
            {
                std::lock_guard<std::mutex> Lock(PlaybackTimeMutex_);
                PlaybackTime_ = duration<double>(0);
            }
        }

        if (Packet_ != nullptr) {
            av_packet_unref(Packet_);
            av_packet_free(&Packet_);
            Packet_ = nullptr;
        }
        if (Frame_ != nullptr) {
            av_frame_unref(Frame_);
            av_frame_free(&Frame_);
            Frame_ = nullptr;
        }
        // if (HwFrame_ != nullptr) {
        //     av_frame_unref(HwFrame_);
        //     av_frame_free(&HwFrame_);
        //     HwFrame_ = nullptr;
        // }

        Stream_ = nullptr;
        if (FormatContext_ != nullptr) {
            avformat_close_input(&FormatContext_);
            FormatContext_ = nullptr;
        }
        if (CodecContext_ != nullptr) {
            avcodec_flush_buffers(CodecContext_);
            avcodec_free_context(&CodecContext_);
            CodecContext_ = nullptr;
        }
        if (SwsContext_ != nullptr) {
            sws_freeContext(SwsContext_);
            SwsContext_ = nullptr;
        }
        if (ImageData_[0] != nullptr) {
            av_freep(ImageData_);
        }
        for (int i = 0; i < 4; i++) {
            ImageData_[i] = nullptr;
            ImageLineSize[i] = 0;
        }
        FrameWidth_ = 0;
        FrameHeight_ = 0;
        FramePTS_ = 0;
        Init_ = false;
    }

    bool Player::Play() {
        if (!Init_) return false;

        Start_ = true;
        Playing_ = true;
        if (DecodeThread_ == nullptr) {
            DecodeThread_ = std::make_shared<std::thread>([this]() {
                this->DoDecode();
            });
        }
        if (TimerThread_ == nullptr) {
            TimerThread_ = std::make_shared<std::thread>([this]() {
                this->PlaybackTimer();
            });
        }

        return true;
    }

    void Player::PlaybackTimer() {
        {
            std::lock_guard<std::mutex> Lock(PlaybackTimeMutex_);
            PlaybackTime_ = duration<double>(0);
        }

        auto PrevTime = steady_clock::now();
        while (Start_) {
            if (!Playing_) {
                PrevTime = steady_clock::now();
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
                continue;
            } else {
                auto CurrTime = steady_clock::now();
                duration<double> Delta = CurrTime - PrevTime;
                {
                    std::lock_guard<std::mutex> Lock(PlaybackTimeMutex_);
                    PlaybackTime_ += Delta;
                }
                PrevTime = CurrTime;
            }
        }
    }

    void Player::DoDecode() {
        int32_t FrameIndex = -1;
        bool HasPacket = false;
        bool HasFrame = false;
        int32_t ErrNum;

        while (Start_) {
            if (!Playing_) {
                std::this_thread::sleep_for(std::chrono::milliseconds(5));// 不处于播放状态
                continue;
            }

            // Read next frame packet data
            {
                if (HasPacket) {
                    HasPacket = false;
                    av_packet_unref(Packet_);
                }

                ErrNum = av_read_frame(FormatContext_, Packet_);
                if (ErrNum == AVERROR_EOF) {
                    if (!Loop_) break;
                    ErrNum = av_seek_frame(FormatContext_, Stream_->index, 0, 0);
                    AssertBreak(ErrNum >= 0, LogLevel::Error, "avformat_open_input failed error=%d, msg=%s", ErrNum, av_err2str(ErrNum));
                    avcodec_flush_buffers(CodecContext_);
                    {
                        std::lock_guard<std::mutex> Lock(PlaybackTimeMutex_);
                        PlaybackTime_ = duration<double>(0);
                    }
                    FrameIndex = -1;
                    continue;
                }
                AssertBreak(ErrNum >= 0, LogLevel::Error, "av_read_frame failed error=%d, msg=%s", ErrNum, av_err2str(ErrNum));

                if (Packet_->stream_index != Stream_->index) {
                    continue;
                }
                HasPacket = true;
            }

            // Decode and trans yuv to rgba
            {
                ErrNum = avcodec_send_packet(CodecContext_, Packet_);
                if (ErrNum == AVERROR(EAGAIN)) continue; // 当前状态不接受输入
                AssertBreak(ErrNum == 0, LogLevel::Error, "avcodec_send_packet failed error=%d, msg=%s", ErrNum, av_err2str(ErrNum));

                HasFrame = false;
                if (UseHwDevice) {
                    ErrNum = avcodec_receive_frame(CodecContext_, Frame_);
                    if (ErrNum == AVERROR(EAGAIN) || ErrNum == AVERROR_EOF) continue; // 当前状态输出不可用
                    // ErrNum = av_hwframe_transfer_data(Frame_, HwFrame_, 0);
                    // AssertBreak(ErrNum == 0, LogLevel::Error, "av_hwframe_transfer_data Error transferring the data to system memory error=%d, msg=%s", ErrNum, av_err2str(ErrNum));
                } else {
                    ErrNum = avcodec_receive_frame(CodecContext_, Frame_);
                    if (ErrNum == AVERROR(EAGAIN) || ErrNum == AVERROR_EOF) continue; // 当前状态输出不可用
                }
                AssertBreak(ErrNum == 0, LogLevel::Error, "avcodec_receive_frame failed error=%d, msg=%s", ErrNum, av_err2str(ErrNum));
                ++FrameIndex;
                HasFrame = true;
            }

            // Sleep until this frame show time
            {
                // 当前帧的预期播放时间
                FramePTS_ = (int64_t) FrameIndex * (int64_t) AV_TIME_BASE / int64_t(av_q2d(Stream_->avg_frame_rate));
                // FramePTS_ = av_rescale_q(Frame_->pts, Stream_->time_base, AVRational{1, AV_TIME_BASE});//硬件加速不支持计算帧时间
                while (FramePTS_ > (int64_t) (PlaybackTime_.count() * AV_TIME_BASE)) {
                    if (!Start_) break;
                    std::this_thread::sleep_for(std::chrono::milliseconds(5));// 还没到该帧的显示时间
                }
            }
            Log::Write(LogLevel::Info, "FrameIndex=%d, FrameTime=%d, PlaybackTime=%f", FrameIndex, FramePTS_, PlaybackTime_.count());
            for (auto &FrameUpdateCallback: FrameUpdateCallbacks_) {
                switch (Frame_->format) {
                    case AV_PIX_FMT_NV12:
                        sws_scale(SwsContext_, Frame_->data, Frame_->linesize, 0, Frame_->height, ImageData_, ImageLineSize);//NV12转换成RGBA
                        FrameUpdateCallback(ImageData_[0], FrameIndex, FramePTS_, Frame_->width, Frame_->height);// 返回RGBA
                        break;
                    case AV_PIX_FMT_D3D11:
                        FrameUpdateCallback(Frame_->data[0], FrameIndex, FramePTS_, Frame_->width, Frame_->height);// 直接返回ID3D11Texture2D pointer
                        break;
                    default:
                        Log::Write(LogLevel::Error, "UnSupported PixFmt %d", (int64_t) CodecContext_->pix_fmt);
                        break;
                }
            }
        }
        av_packet_unref(Packet_);
        av_frame_unref(Frame_);
    }

    AVCodec *Player::DetectCodec(int32_t CodecID) {
        AVCodec *Codec = nullptr;
        // Detect Hardware Acceleration
        GpuType Type = Util::DetectGPU();
        switch (Type) {
            case GpuType::NVIDIA:
                if (CodecID == AV_CODEC_ID_H264) {
                    Codec = avcodec_find_decoder_by_name("h264_cuvid");
                } else if (CodecID == AV_CODEC_ID_HEVC) {
                    Codec = avcodec_find_decoder_by_name("hevc_cuvid");
                }
                break;
                // amf do not support decode
                // case GpuType::AMD:
                //     if (CodecID == AV_CODEC_ID_H264) {
                //         Codec = avcodec_find_decoder_by_name("h264_amf");
                //     } else if (CodecID == AV_CODEC_ID_HEVC) {
                //         Codec = avcodec_find_decoder_by_name("hevc_amf");
                //     }
                //     break;
            case GpuType::UnKnown:
            default:
                break;
        }

        // Fallback to Soft Codec
        if (Codec == nullptr) {
            Codec = avcodec_find_decoder((AVCodecID) CodecID);
        }
        return Codec;
    }

    AVStream *Player::FindVideoStream(AVFormatContext *InFormatContext) {
        if (InFormatContext != nullptr) {
            for (unsigned int i = 0; i < InFormatContext->nb_streams; ++i) {
                auto StreamItem = InFormatContext->streams[i];
                if (StreamItem->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                    return StreamItem;
                }
            }
        }
        return nullptr;
    }

    int32_t Player::GetWidth() const {
        return FrameWidth_;
    }

    int32_t Player::GetHeight() const {
        return FrameHeight_;
    }

    void Player::SetLoop(bool InLoop) {
        Loop_ = InLoop;
    }

    void Player::SetFrameUpdateCallback(const VideoFrameUpdateCallback &InCallback) {
        // auto It = std::find(FrameUpdateCallbacks_.begin(), FrameUpdateCallbacks_.end(), InCallback);
        // if (It != FrameUpdateCallbacks_.end()) return;
        FrameUpdateCallbacks_.emplace_back(InCallback);
    }

    void Player::Pause() {
        Playing_ = false;
    }

    int32_t Player::GetHwFormat(AVCodecContext *InCodecContext, const int32_t *InPixFormats) {
        return 0;
    }

    bool Player::IsUseHwDevice() const {
        return UseHwDevice;
    }
}
