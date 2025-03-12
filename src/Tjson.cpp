//----------------------------------------------------------------------------------------
//
// Created by Q-engineering 2023/1/2
//
//----------------------------------------------------------------------------------------
#include "Tjson.h"
#include <iostream>
#include <fstream>
//----------------------------------------------------------------------------------------
Tjson::Tjson(): MJPEG_Port(0)
{
    Jvalid=false;
}
//----------------------------------------------------------------------------------------
Tjson::~Tjson()
{
    //dtor
}
//----------------------------------------------------------------------------------------
bool Tjson::LoadFromFile(const std::string FileName)
{
    if(FileExists(FileName)){
        try{
            std::ifstream f(FileName);
            j = json::parse(f);
            Jvalid=true;
        }
        catch ( ... ){
            std::cout << "parse error in file " << FileName << std::endl;
        }
    }
    else{
        Jvalid=false;
        std::cout << FileName << " file not found!" << std::endl;
    }

    return Jvalid;
}
//----------------------------------------------------------------------------------------
bool Tjson::GetSettings(void)
{
    std::string Jstr;
    bool Success=false;

    if(Jvalid){
        //video source
        Jstr = j.at("VIDEO_INPUT");
        if(!Jstr.empty()){
            if(!GetSetting(j["VIDEO_INPUTS_PARAMS"],Jstr,Gstr)) return Success;
        }
        else{
            std::cout << "Error reading VIDEO_INPUT value!" << std::endl;
            return Success;
        }
        if(!GetSetting(j,"VERSION",Version))               return Success;
        if(!GetSetting(j,"VEHICLE_MODEL",Cstr))            return Success;
        if(!GetSetting(j,"LICENSE_MODEL",Lstr))            return Success;
        if(!GetSetting(j,"OCR_MODEL",Ostr))                return Success;
        if(!GetSetting(j,"PRINT_ON_CLI",PrintOnCli))       return Success;
        if(!GetSetting(j,"PRINT_ON_RENDER",PrintOnRender)) return Success;
        if(!GetSetting(j,"HEURISTIC_ON",HeuristicsOn))     return Success;
        if(!GetSetting(j,"THRESHOLD_VEHICLE",ThresCar))   return Success;
        if(!GetSetting(j,"THRESHOLD_PLATE",ThresPlate))    return Success;
        if(!GetSetting(j,"THRESHOLD_OCR",ThresOCR))        return Success;
        if(!GetSetting(j,"FoI_FOLDER",FoI_Folder))         return Success;
        if(!GetSetting(j,"VEHICLES_FOLDER",Car_Folder))    return Success;
        if(!GetSetting(j,"PLATES_FOLDER",Plate_Folder))    return Success;
        if(!GetSetting(j,"JSONS_FOLDER",Json_Folder))      return Success;
        if(!GetSetting(j,"RENDERS_FOLDER",Render_Folder))  return Success;
        if(!GetSetting(j,"MJPEG_PORT",MJPEG_Port))         return Success;
        if(!GetSetting(j,"MJPEG_WIDTH",MJPEG_Width))       return Success;
        if(!GetSetting(j,"MJPEG_HEIGHT",MJPEG_Height))     return Success;

        //crop sizes
        if(!GetSetting(j["RoI"],"x_offset",RoiCrop.x))     return Success;
        if(!GetSetting(j["RoI"],"y_offset",RoiCrop.y))     return Success;
        if(!GetSetting(j["RoI"],"width",RoiCrop.width))    return Success;
        if(!GetSetting(j["RoI"],"height",RoiCrop.height))  return Success;

        //so far, so good
        Success=true;
    }

    return Success;
}
//----------------------------------------------------------------------------------------
void Tjson::MakeFolders(void)
{
    MakeDir(FoI_Folder);
    MakeDir(Car_Folder);
    MakeDir(Plate_Folder);
    MakeDir(Json_Folder);
    MakeDir(Render_Folder);
}
//----------------------------------------------------------------------------------------
bool Tjson::GetSetting(const json& s,const std::string Key,std::string& Value)
{
    bool Success=false;

    if(Jvalid){
        //read the key
        Value = s.at(Key);
        if(Value.empty()){
            std::cout << "Error reading value of "<< Key <<" in json file!" << std::endl;
        }
        else Success=true;
    }

    return Success;
}
//----------------------------------------------------------------------------------------
bool Tjson::GetSetting(const json& s, const std::string Key,bool& Value)
{
    bool Success=false;

    if(Jvalid){
        //read the key
        try{
            Value = s.at(Key);
            Success=true;
        }
        catch( ... ){
            std::cout << "Error reading value of "<< Key <<" in json file!" << std::endl;
        }
    }

    return Success;
}
//----------------------------------------------------------------------------------------
bool Tjson::GetSetting(const json& s, const std::string Key,int& Value)
{
    bool Success=false;

    if(Jvalid){
        try{
            Value = s.at(Key);
            Success=true;
        }
        catch( ... ){
            std::cout << "Error reading value of "<< Key <<" in json file!" << std::endl;
        }
    }

    return Success;
}
//----------------------------------------------------------------------------------------
bool Tjson::GetSetting(const json& s, const std::string Key,double& Value)
{
    bool Success=false;

    if(Jvalid){
        try{
            Value = s.at(Key);
            Success=true;
        }
        catch( ... ){
            std::cout << "Error reading value of "<< Key <<" in json file!" << std::endl;
        }
    }

    return Success;
}
//----------------------------------------------------------------------------------------

