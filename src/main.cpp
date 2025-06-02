#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <fstream>
#include <thread>
#include <regex>
#include <opencv2/opencv.hpp>
#include "opencv2/core/version.hpp"
#include "opencv2/videoio/videoio.hpp"
#include "yolo_v2_class.hpp"	        // imported functions from .so
#include "RTSPcam.h"
#include "Regression.h"
#include "Tjson.h"
#include "MJPG_sender.h"
#include <algorithm>

using namespace std;
bool messagePrinted = false;

//----------------------------------------------------------------------------------------
// set the config.json with its settings global

Tjson Js;

//----------------------------------------------------------------------------------------
void draw_vehicle(cv::Mat& bgr, bbox_t& v)
{
    //Create the rectangle
    cv::Rect roi(v.x+Js.RoiCrop.x, v.y+Js.RoiCrop.y, v.w, v.h);
    if(v.obj_id == 0) cv::rectangle(bgr, roi, cv::Scalar(255, 255,   0),2); //cyan - car
    else              cv::rectangle(bgr, roi, cv::Scalar(255,   0, 255),2); //magenta - motorcycle
}
//----------------------------------------------------------------------------------------
void draw_plate(cv::Mat& bgr, bbox_t& v, bbox_t& p)
{
    //Create the rectangle
    cv::Rect roi(p.x+v.x+Js.RoiCrop.x, p.y+v.y+Js.RoiCrop.y, p.w, p.h);
    cv::rectangle(bgr, roi, cv::Scalar(0, 255, 0),2); //green - plate
}
//----------------------------------------------------------------------------------------
void draw_ocr(cv::Mat& bgr, bbox_t& v, bbox_t& p,
    const std::vector<bbox_t>& result_vec,
    const std::vector<std::string>& obj_names)
{
    if (result_vec.empty()) return;
    std::string text;
    for (size_t i = 0; i < result_vec.size() && i < 32; ++i) {
        text += obj_names[result_vec[i].obj_id][0];
    }
    
    double fontScale = 1.0;
    int thickness = 3, baseLine = 0, padding = 7;

    cv::Size textSize = cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, fontScale, thickness, &baseLine);
    int rectWidth = textSize.width + 2 * padding;
    int rectHeight = textSize.height + 2 * padding;
    int plate_center_x = p.x + v.x + Js.RoiCrop.x + (p.w / 2);
    int rectX = std::clamp(static_cast<int>(plate_center_x - (rectWidth / 2)), 0, bgr.cols - rectWidth);
    int rectY = std::clamp(static_cast<int>(p.y + v.y + Js.RoiCrop.y + p.h), 0, bgr.rows - rectHeight);

    cv::rectangle(bgr, cv::Rect(rectX, rectY, rectWidth, rectHeight), cv::Scalar(0, 0, 0), cv::FILLED);
    int text_x = rectX + (rectWidth - textSize.width) / 2;
    int text_y = rectY + (rectHeight + textSize.height - baseLine) / 2 + 4;
    cv::putText(bgr, text, cv::Point(text_x, text_y), cv::FONT_HERSHEY_SIMPLEX, fontScale, cv::Scalar(255, 255, 255), thickness);
}
//----------------------------------------------------------------------------------------
void print_result(vector<bbox_t> const result_vec, vector<string> const obj_names)
{
	for (auto &i : result_vec) {
		if (obj_names.size() > i.obj_id) cout << obj_names[i.obj_id] << " - ";
		cout << setprecision(3) << "prob = " << i.prob << ",  x = " << i.x << ", y = " << i.y
			<< ", w = " << i.w << ", h = " << i.h << endl;
	}
    cout << " " << endl;
}
//----------------------------------------------------------------------------------------
vector<string> objects_names_from_file(string const filename)
{
	ifstream file(filename);
	vector<string> file_lines;

	if (!file.is_open()) return file_lines;

	for(string line; file >> line;) file_lines.push_back(line);

	cout << "object names loaded \n";

	return file_lines;
}
//----------------------------------------------------------------------------------------
void SortSingleLine(std::vector<bbox_t>& vec, float ch_wd, float ch_ht, size_t start, size_t& stop) {
    if (start >= vec.size() || stop > vec.size() || stop <= start) return;

    // Sort by x-position
    std::sort(vec.begin() + start, vec.begin() + stop, [](const bbox_t& a, const bbox_t& b) {
        return a.x < b.x;
    });

    // Remove overlapping/duplicate characters (too close)
    for (size_t i = start; i < stop - 1; ++i) {
        int d = (vec[i + 1].x - vec[i].x) * 2;
        if (d >= 0 && d < ch_wd) {
            if (vec[i + 1].prob < vec[i].prob)
                vec.erase(vec.begin() + i + 1);
            else
                vec.erase(vec.begin() + i);
            --stop;
            --i;
        }
    }
}

void SortPlate(std::vector<bbox_t>& vec) {
    size_t len = vec.size();
    if (len < 2) return;

    // Remove excess characters (keep top 10 by probability)
    while (vec.size() > 10) {
        auto min_it = std::min_element(vec.begin(), vec.end(),
            [](const bbox_t& a, const bbox_t& b) { return a.prob < b.prob; });
        vec.erase(min_it);
    }

    len = vec.size();
    if (len < 2) return;

    // Compute average width and height
    float avg_w = 0, avg_h = 0;
    for (const auto& b : vec) {
        avg_w += b.w;
        avg_h += b.h;
    }
    avg_w /= len;
    avg_h /= len;

    // Compute average Y to do clustering into top and bottom lines
    std::vector<float> ys(len);
    for (size_t i = 0; i < len; ++i) ys[i] = vec[i].y;

    float mean_y = std::accumulate(ys.begin(), ys.end(), 0.0f) / len;
    float variance = 0;
    for (float y : ys) variance += (y - mean_y) * (y - mean_y);
    variance /= len;
    float stddev_y = std::sqrt(variance);

    // Heuristic: if Y standard deviation is large → assume 2 lines
    bool is_two_lines = (stddev_y > avg_h * 0.5);

    if (is_two_lines) {
        // Split into two lines based on average Y
        std::vector<bbox_t> top_line, bottom_line;
        for (const auto& b : vec) {
            if (b.y < mean_y)
                top_line.push_back(b);
            else
                bottom_line.push_back(b);
        }

        // Sort each line
        size_t bnd1 = top_line.size();
        SortSingleLine(top_line, avg_w, avg_h, 0, bnd1);

        size_t bnd2 = bottom_line.size();
        SortSingleLine(bottom_line, avg_w, avg_h, 0, bnd2);

        // Merge back
        vec.clear();
        vec.insert(vec.end(), top_line.begin(), top_line.end());
        vec.insert(vec.end(), bottom_line.begin(), bottom_line.end());
    } else {
        // One line — sort all by x
        size_t end = len;
        SortSingleLine(vec, avg_w, avg_h, 0, end);
    }
}
//----------------------------------------------------------------------------------------
static std::mutex mtx_mjpeg;

void send_mjpeg(cv::Mat& mat, int port, int timeout, int quality)
{
    try {
        std::lock_guard<std::mutex> lock(mtx_mjpeg);
        static MJPG_sender wri(port, timeout, quality);

        wri.write(mat);
    }
    catch (...) {
        cerr << " Error in send_mjpeg() function \n";
    }
}
//----------------------------------------------------------------------------------------
bool send_json_http(vector<bbox_t> cur_bbox_vec, vector<string> obj_names, string frame_id,
                    string filename = string(), int timeout = 400000, int port = -1){
    string send_str;

    char *tmp_buf = (char *)calloc(1024, sizeof(char));
    sprintf(tmp_buf, "{\n \"frame_id\":%s, \n \"objects\": [\n", frame_id.c_str());
    send_str = tmp_buf;
    free(tmp_buf);

    // Iterate over each bounding box to form JSON objects
    for (size_t idx = 0; idx < cur_bbox_vec.size(); idx++) {
        auto &i = cur_bbox_vec[idx];
        char *buf = (char *)calloc(2048, sizeof(char));

        // Calculate relative coordinates
        float rel_center_x = (i.x + (i.w / 2.0)) / (float)Js.RoiCrop.width;
        float rel_center_y = (i.y + (i.h / 2.0)) / (float)Js.RoiCrop.height;
        float rel_width = i.w / (float)Js.RoiCrop.width;
        float rel_height = i.h / (float)Js.RoiCrop.height;


        sprintf(buf, 
            "  {\"class_id\":%d, \"name\":\"%s\", "
            "\"relative_coordinates\":{\"center_x\":%.6f, \"center_y\":%.6f, \"width\":%.6f, \"height\":%.6f}, \"confidence\":%.6f}",
            i.obj_id, obj_names[i.obj_id].c_str(),
            rel_center_x, rel_center_y, rel_width, rel_height, i.prob);

        send_str += buf;

        if (idx != cur_bbox_vec.size() - 1) {
            send_str += ",\n";
        } else {
        send_str += "\n";
    }
        free(buf);
    }

    send_str += " ]\n}";

    if(Js.Json_Folder!="none"){
        ofstream Jfile(Js.Json_Folder + "/" + frame_id + ".json");
        Jfile << send_str;
        Jfile.close();
    }
     if (port == -1) {
        port = Js.JSON_Port;
    }

    send_json_custom(send_str.c_str(), port, timeout);
    return true;
}
//----------------------------------------------------------------------------------------
void CropMat(cv::Mat& In, cv::Mat& Out) //checks the RoI parameters on forehand
{
    cv::Rect R;

    if(Js.RoiCrop.width  <= In.cols) R.width  = Js.RoiCrop.width;
    else                             R.width  = In.cols;

    if(Js.RoiCrop.height <= In.rows) R.height = Js.RoiCrop.height;
    else                             R.height = In.rows;

    if(Js.RoiCrop.x < 0 ) R.x=0;
    else{
        if((Js.RoiCrop.x+R.width) <= In.cols) R.x=Js.RoiCrop.x;
        else                                  R.x=In.cols-R.width;
    }

    if(Js.RoiCrop.y < 0 ) R.y=0;
    else{
        if((Js.RoiCrop.y+R.height) <= In.rows) R.y=Js.RoiCrop.y;
        else                                   R.y=In.rows-R.height;
    }

    Out = In(R);
    //important update the Js.roiCrop as it is used as offset in the remaining code.
    //in fact you may overrule the config.json here.
    Js.RoiCrop = R;
}
//----------------------------------------------------------------------------------------
int main(int argc, char** argv) {

    bool Success;
    char ChrCar='a';
    char ChrPlate='1';
    unsigned int Wd, Ht;
    unsigned int WdC, HtC;
    cv::Mat frame;
    cv::Mat frame_full;
    cv::Mat frame_full_render;
    RTSPcam cam;
    vector<bbox_t> result_ocr;

    //Js takes care of printing errors.
    Js.LoadFromFile("./config.json");

   //Js takes care of printing errors.
   std::string config_file = "./config.json";

   if (argc > 1) {
       config_file = argv[1];
       std::cout << "Using config file: " << config_file << std::endl;
   } else {
       std::cout << "No config file provided. Using default: " << config_file << std::endl;
   }

   Js.LoadFromFile(config_file);

   Success = Js.GetSettings();
   if (!Success) {
       std::cerr << "Failed to load config: " << config_file << std::endl;
       return -1;
   }

    cout << "ALPR Version : " << Js.Version << endl;
     // Print current mode based on config

    //see if we must make some output directories.
    Js.MakeFolders();

	Detector CarNet(Js.Cstr+".cfg", Js.Cstr+".weights");
	auto CarNames = objects_names_from_file(Js.Cstr+".names");

	Detector PlateNet(Js.Lstr+".cfg", Js.Lstr+".weights");
    auto PlateNames = objects_names_from_file(Js.Lstr+".names");

	Detector OcrNet(Js.Ostr+".cfg", Js.Ostr+".weights");
	auto OcrNames = objects_names_from_file(Js.Ostr+".names");

    cam.Open(Js.Gstr);   //you can dump anything OpenCV eats. (cv::CAP_ANY)

    while (true) {
        try {
            if(!cam.GetLatestFrame(frame_full)){
                if (!messagePrinted) {
                    std::cout << "Input stream is closed" << std::endl;
                    messagePrinted = true;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }
            else{
                if(!frame_full.empty()){
                    //store the frame_full only if the directory name is valid
                    //note it stores a MASSIVE bulk of pictures on your disk!
                    if(Js.FoI_Folder!="none"){
                        cv::imwrite( Js.FoI_Folder+"/"+cam.CurrentFileName+"_utc.png", frame_full);
                    }
                    //crop and copy the frame
                    frame_full_render = frame_full.clone();
                    CropMat(frame_full,frame);
                    //draw crop borders
                    cv::rectangle(frame_full_render, Js.RoiCrop, cv::Scalar(128, 128, 128),2);

                    //detect the cars
                    vector<bbox_t> result_car = CarNet.detect(frame,Js.ThresCar);

                    //loop through the found cars/motorbikes
                    Wd = frame.cols;  Ht = frame.rows; ChrCar='a';
                    for (auto &i : result_car) {
                        //a known issue; the whole image is selected as an object -> skip this result
                        if((100*i.w>(95*Wd)) || (100*i.h>(95*Ht))) continue;    //stay in the integer domain
                        //Create the rectangle
                        if((i.w > 40) && (i.h > 40) &&    //get some width and height (40x40)
                           ((i.x + i.w) < Wd) && ((i.y + i.h) < Ht)){
                                cv::Rect roi(i.x, i.y, i.w, i.h);
                                //Create the ROI
                                cv::Mat frame_car = frame(roi);

                                //draw borders around cars/motorbikes
                                draw_vehicle(frame_full_render, i);

                                //store the car only if the directory name is valid
                                if(Js.Car_Folder!="none"){
                                    cv::imwrite( Js.Car_Folder+"/"+cam.CurrentFileName+"_"+ChrCar+"_utc.png", frame_car);
                                    ChrCar++;
                                }

                                //detect plates
                                vector<bbox_t> result_plate = PlateNet.detect(frame_car,Js.ThresPlate);

                                //loop through the found license plates
                                WdC = frame_car.cols;  HtC = frame_car.rows; ChrPlate='1';
                                for (auto &j : result_plate) {
                                    WdC = frame_car.cols;  HtC = frame_car.rows;
                                    if((j.w > 20) && (j.h > 10) &&    //get some width and height (20x10)
                                       ((j.x + 2 + j.w) < WdC) && ((j.y + 2 + j.h) < HtC)){
                                        cv::Rect roi(j.x, j.y, j.w+2, j.h+2);
                                        //Create the ROI
                                        cv::Mat frame_plate = frame_car(roi);

                                        //draw borders around plates
                                        draw_plate(frame_full_render, i, j);
                                        //store the car only if the directory name is valid
                                        if(Js.Plate_Folder!="none"){
                                            cv::imwrite( Js.Plate_Folder+"/"+cam.CurrentFileName+"_"+ChrCar+"_"+ChrPlate+"_utc.png", frame_plate);
                                            ChrPlate++;
                                        }

                                        //detect plates
                                        result_ocr = OcrNet.detect(frame_plate,Js.ThresOCR);

                                        //heuristics
                                        if(Js.HeuristicsOn){
                                            SortPlate(result_ocr);
                                        }
                                        else {
                                            SortPlate(result_ocr);  // Even when heuristics are off, still call SortPlate
                                        }    

                                        //show
                                        if(Js.PrintOnCli){
                                            print_result(result_ocr, OcrNames);
                                        }
                                        //draw borders around plates
                                        draw_ocr(frame_full_render, i, j, result_ocr, OcrNames);
                                }
                            }
                        }
                        //store the frame_full only if the directory name is valid
                        //note it stores a MASSIVE bulk of pictures on your disk!
                        if(Js.Render_Folder!="none"){
                            cv::imwrite( Js.Render_Folder+"/"+cam.CurrentFileName+"_utc.png", frame_full_render);
                        }
                    }
                                        
                    int frame_id = -1;
                    std::smatch match;
                    std::regex re("(\\d+)");
                    
                    if (std::regex_search(cam.CurrentFileName, match, re)) {
                        try {
                            frame_id = std::stoi(match.str(1));
                        } catch (...) {
                            frame_id = 0;
                        }
                    } else {
                        static int static_frame_counter = 0;
                        frame_id = static_frame_counter++;
                    }
                    //send json into the world (port 8070)
                    send_json_http(result_car, CarNames, std::to_string(frame_id), cam.CurrentFileName + "_" + ChrCar + "_" + ChrPlate + "_utc.json");

                    //send the frame to port 8090
                    if(Js.MJPEG_Port > 0){
                        cv::Mat frame_resize(Js.MJPEG_Height, Js.MJPEG_Width, CV_8UC3);
                        cv::resize(frame_full_render,frame_resize,frame_resize.size(),0,0);
                        send_mjpeg(frame_resize, Js.MJPEG_Port, 4000000, 90);
                    }

                    //print frame
                    cout << "CurrentFileName : "<< cam.CurrentFileName << endl;

                    //show frame
                    if(Js.PrintOnRender){
                        cv::imshow("RTSP stream",frame_full_render);
                        if(cam.UsePicture){
                            char esc = cv::waitKey();       //in case of a static picture wait infinitive
                            if(esc == 27) break;
                        }
                        else{
                            char esc = cv::waitKey(5);
                            if(esc == 27) break;
                        }
                    }
                }
            }
        }
        catch (exception &e) { cerr << "exception: " << e.what() << "\n"; getchar(); }
        catch (...) { cerr << "unknown exception \n"; getchar(); }
	}
	return 0;
}
//----------------------------------------------------------------------------------------

