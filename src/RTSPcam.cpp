//----------------------------------------------------------------------------------------
//
// Created by Q-engineering 2022/12/14
//
//----------------------------------------------------------------------------------------
#include "RTSPcam.h"
#include <chrono>
#include <sys/stat.h>
#include <iostream>
#include <string>

#define DUMMY 35
#define COUNT 35

using namespace std;
//----------------------------------------------------------------------------------------
RTSPcam::RTSPcam(void): cap(NULL), FirstPic(true), FrameCnt(0)
{
    cap = new cv::VideoCapture;
    UsePicture = false;
    UseFolder  = false;                 // true when a only folder name is loaded.
    CurrentFileName = "";
}
//----------------------------------------------------------------------------------------
RTSPcam::~RTSPcam()
{
    cap->release();
    delete cap;
}
//----------------------------------------------------------------------------------------
void RTSPcam::Open(const int Value)
{
    cout << "Connecting to : "<< Value << endl;

    cap->open(Value);

    ProcessOpen();
}
//----------------------------------------------------------------------------------------
void RTSPcam::Open(const string& MyString, int apiPreference)
{
    struct stat s;
    MyFile = MyString;
    string Ext;

    UsePicture = false;
    UseFolder  = false;                 // true when a only folder name is loaded.

    if(stat(MyString.c_str(),&s)==0){
        if(s.st_mode & S_IFREG){
            Ext=MyString.substr(MyString.find_last_of(".") + 1);
            std::transform(Ext.begin(), Ext.end(), Ext.begin(),[](unsigned char c){ return std::tolower(c); });
            if(Ext=="bmp" || Ext=="jpg" || Ext=="png" ){
                UsePicture = true;
                cout << "Open picture : " << MyFile << endl;
            }
            else{
                //video file
                cout << "Open movie : " << MyFile << endl;
                cap->open(MyString, apiPreference);
                ProcessOpen();
            }
            return;
        }
        else{
            if(s.st_mode & S_IFDIR){
                UseFolder=true;
                cout << "Open folder : " << MyFile << endl;
                return;
            }
        }
    }

    cout << "Connecting to : "<< MyString << endl;

    cap->open(MyString, apiPreference);

    ProcessOpen();
}
//----------------------------------------------------------------------------------------
void RTSPcam::ProcessOpen(void)
{
    int n;
    double Elapse;
    cv::Mat frame;
    chrono::steady_clock::time_point Tyet;

    if(!cap->isOpened()){
        throw runtime_error("Failed to open camera");
    }

    FPS=cap->get(cv::CAP_PROP_FPS);

    if(FPS > 0.0 && FPS < 51.0){
        FrameTime=1000.0/FPS;
    }
    else{
        //measure fps cap.get(cv::CAP_PROP_FPS) doesn't work always
        //start with some dummy reads
        for(n=0;n<DUMMY;n++) if(!cap->read(frame)) break;
        if(n!=DUMMY) throw runtime_error("Can't read some frames");

        Tyet  = chrono::steady_clock::now();
        for(n=0;n<COUNT;n++) if(!cap->read(frame)) break;
        Tgrab = chrono::steady_clock::now();

        if(n!=COUNT) throw runtime_error("Can't read some frames");

        Elapse   = chrono::duration_cast<chrono::milliseconds> (Tgrab - Tyet).count();
        FrameTime=Elapse/COUNT;
        if (FPS > 0.0) {
            FrameTime = 1000.0 / FPS;
        }
    }
    fpsPrintTime = std::chrono::steady_clock::now();
    cout << "FPS : " << FPS << endl;
}
//----------------------------------------------------------------------------------------
bool RTSPcam::GetLatestFrame(cv::Mat& frame)
{
    bool Success=false;
    double Elapse;
    int LostFrames;
    string FullName;

    chrono::steady_clock::time_point Tyet;

    if(UsePicture){
        frame = cv::imread(MyFile);
        if(!frame.empty()){
            NiceString(MyFile);
            Success=true;
        }
        return Success;
    }
    if(UseFolder){
        //open folder
        if(FirstPic){
            FirstPic=false;
            if((dir = opendir (MyFile.c_str())) == NULL){
                Success=false;
                return Success;
            }
        }
        //read folder
        while((ent = readdir(dir)) != NULL) {
            FullName = MyFile;
            if(FullName.length()>1){
                if( FullName[ FullName.length()-1] != '/') FullName += "/";
                FullName += ent->d_name;

                frame = cv::imread(FullName);
                if(!frame.empty()){
                    NiceString(FullName);
                    Success=true;
                    break;
                }
            }
        }
        return Success;
    }

    //no static picture but a video
    if(!FirstPic){
        Tyet   = chrono::steady_clock::now();
        Elapse = chrono::duration_cast<chrono::milliseconds> (Tyet - Tgrab).count();
        if(Elapse > FrameTime){
            LostFrames=ceil(1.15*Elapse/FrameTime);     //1.15 to be sure you have always the latest frame, in case FrameTime isn't accurate
            if(LostFrames>500) LostFrames=500;          //don't wait forever.
            //flush the lost frames
            FrameCnt+=LostFrames;
            for(int n=0;n<LostFrames;n++){
                if(!cap->read(frame)) break;
            }
        }
    }
    Success = cap->read(frame);
    Tgrab   = chrono::steady_clock::now();

    FirstPic= false;
    FrameCnt++;
    NiceString(FrameCnt);

    auto now = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::seconds>(now - fpsPrintTime).count() >= 5) {
    double actualFPS = 1000.0 / Elapse;
    //std::cout << "\r Actual FPS: " << std::fixed << std::setprecision(2) << actualFPS << std::flush;
    fpsPrintTime = now;
    }

    // Display actual FPS on top of the video frame
    std::ostringstream fpsStream;
    fpsStream << "FPS: " << std::fixed << std::setprecision(2) << (1000.0 / Elapse);
    std::string actual_fps_text = fpsStream.str();

    // Overlay the actual FPS on the frame
    cv::putText(frame, actual_fps_text, 
    cv::Point((frame.cols - cv::getTextSize(actual_fps_text, cv::FONT_HERSHEY_SIMPLEX, 1.0, 2, nullptr).width) / 2, 30), 
    cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);

    return Success;
}
//----------------------------------------------------------------------------------------
void RTSPcam::NiceString(const std::string Str)
{
    size_t lastindex;

    lastindex = Str.find_last_of("\\/");
    if (std::string::npos != lastindex){
        CurrentFileName = Str.substr(lastindex+1, Str.length());
    }

    lastindex = CurrentFileName.find_last_of(".");
    if (std::string::npos != lastindex){
        CurrentFileName = CurrentFileName.substr(0, lastindex);
    }
}
//----------------------------------------------------------------------------------------
void RTSPcam::NiceString(const size_t Cnt)
{
    std::stringstream ss;

    ss << std::setfill('0') << std::setw(12) << Cnt ;

    CurrentFileName = ss.str();
}
//----------------------------------------------------------------------------------------
