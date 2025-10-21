//----------------------------------------------------------------------------------------
//
// Created by Q-engineering 2022/12/14
//
//----------------------------------------------------------------------------------------
#include "RTSPcam.h"
#include <chrono>
#include <sys/stat.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <dirent.h>
#include <algorithm>
#include <thread>
#include <opencv2/opencv.hpp>
#include <cmath>

#define DUMMY 35
#define COUNT 35
#define RECONNECT_DELAY_SEC 2   // delay between RTSP reconnect attempts

using namespace std;

RTSPcam::RTSPcam() : cap(new cv::VideoCapture), FirstPic(true), FrameCnt(0), dir(nullptr), Loop(false)
{
    UsePicture = false;
    UseFolder  = false;
    UseVideo   = false;
    UseRTSP    = false;
    CurrentFileName = "";
}

RTSPcam::~RTSPcam()
{
    if (cap->isOpened()) cap->release();
    delete cap;
    if (dir) closedir(dir);
}

void RTSPcam::Open(const int Value)
{
    cout << "Connecting to camera index : " << Value << endl;
    cap->open(Value);
    UseRTSP = true; 
    ProcessOpen();
}

void RTSPcam::Open(const string& MyString, int apiPreference)
{
    struct stat s;
    MyFile = MyString;
    string Ext;
    UsePicture = false;
    UseFolder  = false;
    UseVideo   = false;
    UseRTSP    = false;

    if (stat(MyString.c_str(), &s) == 0) {
        if (s.st_mode & S_IFREG) {
            Ext = MyString.substr(MyString.find_last_of(".") + 1);
            std::transform(Ext.begin(), Ext.end(), Ext.begin(), ::tolower);
            if (Ext == "bmp" || Ext == "jpg" || Ext == "png") {
                UsePicture = true;
                cout << "Open picture : " << MyFile << endl;
            } else {
                UseVideo = true;
                cout << "Open video file : " << MyFile << endl;
                cap->open(MyFile, apiPreference);
                ProcessOpen();
            }
            return;
        } else if (s.st_mode & S_IFDIR) {
            UseFolder = true;
            cout << "Open folder : " << MyFile << endl;
            return;
        }
    }

    // If not file/folder → assume RTSP/HTTP stream
    UseRTSP = true;
    cout << "Connecting to stream : " << MyFile << endl;
    cap->open(MyFile, apiPreference);
    ProcessOpen();
}

void RTSPcam::ProcessOpen()
{
    if (!cap->isOpened() && !UsePicture && !UseFolder)
        throw runtime_error("Failed to open camera or stream");

    if (UsePicture || UseFolder) return; // no FPS calc for static inputs

    FPS = cap->get(cv::CAP_PROP_FPS);
    if (FPS > 0.0 && FPS < 51.0) {
        FrameTime = 1000.0 / FPS;
    } else {
        cv::Mat frame; chrono::steady_clock::time_point t1, t2;
        for (int n = 0; n < DUMMY; n++) if (!cap->read(frame)) break;
        t1 = chrono::steady_clock::now();
        for (int n = 0; n < COUNT; n++) if (!cap->read(frame)) break;
        t2 = chrono::steady_clock::now();
        double Elapse = chrono::duration_cast<chrono::milliseconds>(t2 - t1).count();
        FrameTime = Elapse / COUNT;
    }
    fpsPrintTime = chrono::steady_clock::now();
   // cout << "FPS (reported/estimated): " << FPS << endl;
}

void RTSPcam::Rewind()
{
    if (UseVideo) {
        cout << "[RTSP] Fast rewind video file..." << endl;
        cap->set(cv::CAP_PROP_POS_FRAMES, 0);   // fast rewind
        FirstPic = true;
        FrameCnt = 0;
    }
    else if (UseFolder) {
        cout << "[RTSP] Rewinding folder..." << endl;
        if (dir) closedir(dir);
        FirstPic = true;
        dir = opendir(MyFile.c_str());
    }
}

bool RTSPcam::Reconnect(int apiPreference)
{
    if (!UseRTSP) return false; // only valid for RTSP streams

    while (true) {
        auto now = chrono::system_clock::to_time_t(chrono::system_clock::now());
        cout << "[RTSP] Reconnecting RTSP stream at "
             << put_time(localtime(&now), "%F %T") << " ..." << endl;

        if (cap->isOpened()) cap->release();

        cap->open(MyFile, apiPreference);

        if (cap->isOpened()) {
            cout << "[RTSPcam] Reconnected successfully." << endl;
            try {
                ProcessOpen();
            } catch (...) {
                cout << "[RTSP] Stream opened but not ready, retrying..." << endl;
                this_thread::sleep_for(chrono::seconds(RECONNECT_DELAY_SEC));
                continue;
            }
            return true;
        }

        cout << "[RTSP] Failed to reconnect. Retrying in "
             << RECONNECT_DELAY_SEC << " seconds..." << endl;
        this_thread::sleep_for(chrono::seconds(RECONNECT_DELAY_SEC));
    }
}

bool RTSPcam::GetLatestFrame(cv::Mat& frame)
{
    bool Success = false;

    // --- Single picture ---
    if (UsePicture) {
        frame = cv::imread(MyFile);
        if (!frame.empty()) {
            NiceString(MyFile);
            Success = true;
        }
        return Success;
    }

    // --- Folder looping ---
    if (UseFolder) {
        if (FirstPic) {
            FirstPic = false;
            if ((dir = opendir(MyFile.c_str())) == NULL) return false;
        }

        struct dirent* ent;
        while ((ent = readdir(dir)) != NULL) {
            string FullName = MyFile;
            if (FullName.back() != '/') FullName += "/";
            FullName += ent->d_name;

            frame = cv::imread(FullName);
            if (!frame.empty()) {
                NiceString(FullName);
                Success = true;
                break;
            }
        }

        if (!Success) { // loop folder
            rewinddir(dir);
            return GetLatestFrame(frame);
        }
        return Success;
    }

    // --- Video / RTSP ---
    if (!FirstPic) {
        auto now = chrono::steady_clock::now();
        double Elapse = chrono::duration_cast<chrono::milliseconds>(now - Tgrab).count();
        int LostFrames = static_cast<int>(ceil(1.15 * Elapse / FrameTime));
        LostFrames = min(LostFrames, 500);
        for (int n = 0; n < LostFrames; n++) cap->read(frame);
    }

    Success = cap->read(frame);
    if (!Success) {
        if (UseRTSP) {
            cout << "[RTSP] Frame read failed. Trying to reconnect..." << endl;
            if (Reconnect()) {
                return cap->read(frame); // try again after reconnect
            }
            return false;
        }
        else if (UseVideo && Loop) {
            cout << " End of video reached, Loop enabled → rewinding." << endl;
            Rewind();
            return GetLatestFrame(frame); // get next after rewind
        }
        else {
            return false; // no loop, just end
        }
    }

    Tgrab = chrono::steady_clock::now();
    FirstPic = false;
    FrameCnt++;
    NiceString(FrameCnt);

    // --- FPS overlay (real-time only) ---
    double fps_ms = max(1.0, static_cast<double>(chrono::duration_cast<chrono::milliseconds>(Tgrab - fpsPrintTime).count()));
    std::ostringstream fpsStream;
    fpsStream << "FPS: " << fixed << setprecision(2) << (1000.0 / fps_ms);
    string fpsText = fpsStream.str();
    cv::putText(frame, fpsText,
        cv::Point((frame.cols - cv::getTextSize(fpsText, cv::FONT_HERSHEY_SIMPLEX, 1.0, 2, nullptr).width) / 2, 30),
        cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);

    fpsPrintTime = Tgrab;
    return true;
}

void RTSPcam::NiceString(const std::string Str)
{
    size_t lastindex = Str.find_last_of("\\/");
    if (string::npos != lastindex) {
        CurrentFileName = Str.substr(lastindex + 1);
    }

    lastindex = CurrentFileName.find_last_of(".");
    if (string::npos != lastindex) {
        CurrentFileName = CurrentFileName.substr(0, lastindex);
    }
}

void RTSPcam::NiceString(const size_t Cnt)
{
    stringstream ss;
    ss << setfill('0') << setw(12) << Cnt;
    CurrentFileName = ss.str();
}
//----------------------------------------------------------------------------------------
