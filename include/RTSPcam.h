//----------------------------------------------------------------------------------------
//
// Created by Q-engineering 2022/12/14
//
//----------------------------------------------------------------------------------------
#ifndef RTSPCAM_H
#define RTSPCAM_H

#include "dirent.h"
#include <opencv2/opencv.hpp>
#include <chrono>
#include <string>

class RTSPcam
{
public:
    RTSPcam(void);
    virtual ~RTSPcam();

    // Used with camera index (e.g., 0 for webcam)
    void Open(const int Value);
    // GStreamer, FFmpeg pipeline, RTSP/HTTP URLs, video files, folders, or pictures
    void Open(const std::string& MyString, const int apiPreference = cv::CAP_ANY);

    // Reconnect (only for RTSP streams)
bool Reconnect(int apiPreference = cv::CAP_ANY);

    // Rewind (only for video files and folders)
    void Rewind(void);

    // Get the latest frame with minimal latency
    bool GetLatestFrame(cv::Mat& frame);

    // Configure reconnect policy
    void SetReconnectPolicy(int maxWaitSeconds, int retryDelayMs = 1000);

    // Public info
    double FPS;                     // FPS
    bool UsePicture;                // true when a jpg, png or bmp picture is loaded
    bool UseFolder;                 // true when a folder of images is loaded
    bool UseVideo;                  // true when a video file is loaded
    bool UseRTSP;                   // true when RTSP/HTTP/stream input is used
    bool Loop;                      // true → loop file/folder when finished
    std::string CurrentFileName;    // name of picture or frame number

protected:
    cv::VideoCapture* cap;
    bool FirstPic;
    size_t FrameCnt;
    std::chrono::steady_clock::time_point fpsPrintTime;
    std::string MyFile;

private:
    DIR* dir;
    struct dirent* ent;
    double FrameTime;               // 1/FPS
    void ProcessOpen(void);
    void NiceString(const size_t Cnt);
    void NiceString(const std::string Str);
    std::chrono::steady_clock::time_point Tgrab;

    // reconnect settings
    int reconnectMaxWait;   // how long we wait before giving up (seconds)
    int reconnectDelay;     // delay between retries (ms)
};

#endif // RTSPCAM_H
