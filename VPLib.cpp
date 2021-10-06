#include "VPLib.h"
#include "Util.h"
#include "Logger/Writer.h"

using VP::Logger::Log;
using VP::Logger::LogLevel;

namespace VP {
    Player::~Player() {
        UnInit();
    }

    bool Player::Init(const char *InUrl) {
        if (Init_) return true;

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
            AssertBreak(ErrNum >= 0, LogLevel::Fatal, "avformat_open_input failed error=%d, msg=%s", ErrNum, av_err2str(ErrNum))
            ErrNum = avformat_find_stream_info(FormatContext_, nullptr);
            AssertBreak(ErrNum >= 0, LogLevel::Fatal, "avformat_find_stream_info failed error=%d, msg=%s", ErrNum, av_err2str(ErrNum))

            // Find Video Stream_
            Stream_ = FindVideoStream(FormatContext_);
            AssertBreak(Stream_ != nullptr, LogLevel::Fatal, "Cannot Find VideoStream")

            // Detect Codec
            AVCodec *Codec = DetectCodec(Stream_->codecpar->codec_id);
            AssertBreak(Codec != nullptr, LogLevel::Fatal, "Cannot Find Stream_ Codec")

            // Create Codec Context
            CodecContext_ = avcodec_alloc_context3(Codec);
            AssertBreak(CodecContext_ != nullptr, LogLevel::Fatal, "Cannot Find Stream_ Codec")

            // Init Codec Context From Stream_ Parameter
            ErrNum = avcodec_parameters_to_context(CodecContext_, Stream_->codecpar);
            AssertBreak(ErrNum >= 0, LogLevel::Fatal, "avcodec_parameters_to_context init CodecContext_ failed error=%d, msg=%s",
                        ErrNum, av_err2str(ErrNum))

            // Initialize AVCodecContext use given AVCodec
            ErrNum = avcodec_open2(CodecContext_, Codec, nullptr);
            AssertBreak(ErrNum >= 0, LogLevel::Fatal, "avcodec_open2 Initialize the AVCodecContext to use the given AVCodec failed error=%d, msg=%s",
                        ErrNum, av_err2str(ErrNum))

            // Print Video Info
            Log::Write(LogLevel::Info,
                       "VideoInfo: InFile=%s, Format=%s, CodecID=%s, Codec=%s, PixFmt=%s, Size=%dx%d, Length=%us, Fps=%f, FrameNum=%d",
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
            FrameWidth_ = CodecContext_->width;
            FrameHeight_ = CodecContext_->height;

            // Init Image Data
            if (ImageData_[0] != nullptr) av_freep(ImageData_);
            ErrNum = av_image_alloc(ImageData_, ImageLineSize, FrameWidth_, FrameHeight_, AV_PIX_FMT_RGBA, 1);
            AssertBreak(ErrNum >= 0, LogLevel::Fatal, "av_image_alloc failed error=%d, msg=%s", ErrNum, av_err2str(ErrNum))

            // Init SwsContext
            if (SwsContext_ != nullptr) sws_freeContext(SwsContext_);
            SwsContext_ = sws_getContext(
                    FrameWidth_, FrameHeight_, CodecContext_->pix_fmt,//src
                    FrameWidth_, FrameHeight_, AV_PIX_FMT_RGBA,//dst
                    SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
            AssertBreak(SwsContext_ != nullptr, LogLevel::Fatal, "sws_getContext failed")

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
        }
        if (Packet_ != nullptr) {
            av_packet_free(&Packet_);
            Packet_ = nullptr;
        }
        if (Frame_ != nullptr) {
            av_frame_free(&Frame_);
            Frame_ = nullptr;
        }

        Stream_ = nullptr;
        if (CodecContext_ != nullptr) {
            avcodec_free_context(&CodecContext_);
            CodecContext_ = nullptr;
        }
        if (FormatContext_ != nullptr) {
            avformat_close_input(&FormatContext_);
            FormatContext_ = nullptr;
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
            }
            auto CurrTime = steady_clock::now();
            duration<double> Delta = CurrTime - PrevTime;
            {
                std::lock_guard<std::mutex> Lock(PlaybackTimeMutex_);
                PlaybackTime_ += Delta;
            }
        }

        // clear
        {
            std::lock_guard<std::mutex> Lock(PlaybackTimeMutex_);
            PlaybackTime_ = duration<double>(0);
        }
    }

    void Player::DoDecode() {
        int FrameIndex = -1;
        bool HasPacket = false;
        bool HasFrame = false;
        int ErrNum;

        while (Start_) {
            if (!Playing_) {
                std::this_thread::sleep_for(std::chrono::milliseconds(5));// 不处于播放状态
                continue;
            }

            if (HasPacket) {
                HasPacket = false;
                av_packet_unref(Packet_);
            }

            // Read next frame packet data
            {
                ErrNum = av_read_frame(FormatContext_, Packet_);
                if (ErrNum == AVERROR_EOF) {
                    if (!Loop_) break;
                    ErrNum = av_seek_frame(FormatContext_, Stream_->index, 0, 0);
                    AssertBreak(ErrNum >= 0, LogLevel::Error, "avformat_open_input failed error=%d, msg=%s", ErrNum, av_err2str(ErrNum))
                    {
                        std::lock_guard<std::mutex> Lock(PlaybackTimeMutex_);
                        PlaybackTime_ = duration<double>(0);
                    }
                    FrameIndex = -1;
                    continue;
                }
                AssertBreak(ErrNum >= 0, LogLevel::Error, "av_read_frame failed error=%d, msg=%s", ErrNum, av_err2str(ErrNum))
                HasPacket = true;
            }

            // Decode and trans yuv to rgba
            {
                ErrNum = avcodec_send_packet(CodecContext_, Packet_);
                if (ErrNum == AVERROR(EAGAIN)) continue; // 当前状态不接受输入
                AssertBreak(ErrNum == 0, LogLevel::Error, "avcodec_send_packet failed error=%d, msg=%s", ErrNum, av_err2str(ErrNum))

                HasFrame = false;
                ErrNum = avcodec_receive_frame(CodecContext_, Frame_);
                if (ErrNum == AVERROR(EAGAIN)) continue; // 当前状态输出不可用
                AssertBreak(ErrNum == 0, LogLevel::Error, "avcodec_receive_frame failed error=%d, msg=%s", ErrNum, av_err2str(ErrNum))
                HasFrame = true;
            }

            // Sleep until this frame show time
            {
                FramePTS_ = av_rescale_q(Frame_->pts, Stream_->time_base, AVRational{1, AV_TIME_BASE});// 当前帧的预期播放时间
                while (FramePTS_ > (int64_t) (PlaybackTime_.count() * AV_TIME_BASE)) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(5));// 还没到该帧的显示时间
                }
            }
            sws_scale(SwsContext_, Frame_->data, Frame_->linesize, 0, Frame_->height, ImageData_, ImageLineSize);
            ++FrameIndex;
            Log::Write(LogLevel::Info, "FrameIndex=%d, FrameTime=%d", FrameIndex, FramePTS_);
            for (auto &FrameUpdateCallback: FrameUpdateCallbacks_) {
                FrameUpdateCallback(ImageData_[0], FrameIndex, FramePTS_, Frame_->width, Frame_->height, ImageLineSize[0]);
            }
        }
    }

    AVCodec *Player::DetectCodec(AVCodecID CodecID) {
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
            case GpuType::AMD:
                if (CodecID == AV_CODEC_ID_H264) {
                    Codec = avcodec_find_decoder_by_name("h264_amf");
                } else if (CodecID == AV_CODEC_ID_HEVC) {
                    Codec = avcodec_find_decoder_by_name("hevc_amf");
                }
                break;
            case GpuType::UnKnown:
                break;
        }

        // Fallback to Soft Codec
        if (Codec == nullptr) {
            Codec = avcodec_find_decoder(CodecID);
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

}
