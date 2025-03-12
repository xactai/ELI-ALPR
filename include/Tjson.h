#ifndef TJSON_H
#define TJSON_H

#include <opencv2/opencv.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <fstream>

using json = nlohmann::json;

//----------------------------------------------------------------------------------------
inline bool FileExists(const std::string& name)
{
    if(FILE *file = fopen(name.c_str(), "r")){
        fclose(file);
        return true;
    }
    else return false;
}
//----------------------------------------------------------------------------------------
inline void MakeDir(const std::string& folder)
{
    if(folder!="none"){
        std::string Str = "mkdir -p "+folder;
        system(Str.c_str());
    }
}
//----------------------------------------------------------------------------------------
class Tjson
{
public:
    Tjson();
    ~Tjson();
    bool LoadFromFile(const std::string FileName);
    bool GetSettings(void);
    void MakeFolders(void);
    std::string  Version;     //Version string
    std::string  Gstr;        //Gstreamer string
    std::string  Cstr;        //Car darknet model file name
    std::string  Lstr;        //License darknet model file name
    std::string  Ostr;        //OCR darknet model file name
    bool   PrintOnCli;        //Show license plate on teminal
    bool PrintOnRender;       //show license plate in window
    bool HeuristicsOn;        //Sort character position and remove doubles
    double   ThresCar;        //threshold detection of car model
    double ThresPlate;        //threshold detection of license plate model
    double   ThresOCR;        //threshold detection of ocr model
    std::string FoI_Folder;   //directory with analysed frames
    std::string Car_Folder;   //directory with analysed cars
    std::string Plate_Folder; //directory with analysed plates
    std::string Json_Folder;  //directory with json outputs
    std::string Render_Folder; //directory with ocr results
    int MJPEG_Port;           //output stream (0 = no stream, usually 8090)
    int MJPEG_Width;          //output image width 8090 stream
    int MJPEG_Height;         //output image height 8090 stream
    cv::Rect RoiCrop;         //cropped roi (width==0 and/or height==0 no cropping)

private:
    json j;
    bool Jvalid;
    bool GetSetting(const json& s, const std::string Key, int& Value);
    bool GetSetting(const json& s, const std::string Key, bool& Value);
    bool GetSetting(const json& s, const std::string Key, double& Value);
    bool GetSetting(const json& s, const std::string Key, std::string& Value);
};
//----------------------------------------------------------------------------------------

#endif // TJSON_H
