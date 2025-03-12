#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <fstream>
#include <thread>
#include <opencv2/opencv.hpp>
#include "opencv2/core/version.hpp"
#include "opencv2/videoio/videoio.hpp"
#include "yolo_v2_class.hpp"	        // imported functions from .so
#include "RTSPcam.h"
#include "Regression.h"
#include "Tjson.h"
#include "MJPG_sender.h"

using namespace std;

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
//note, SortSingleLine can erase elements from cur_bbox_vec
//that way shorten the length of the vector. Hence size_t& bnd
void SortSingleLine(vector<bbox_t>& cur_bbox_vec, float ch_wd, float ch_ht, size_t StartPos, size_t& StopPos)
{
    // Ensure StartPos and StopPos are within bounds
    if (StartPos >= cur_bbox_vec.size() || StopPos > cur_bbox_vec.size() || StopPos <= StartPos) return;
    size_t i, j;
    bbox_t tmp_box;
    int d, i1, i2;

    if((StopPos-StartPos)<=1) return;

    //sort by x position
    for(i=StartPos; i<StopPos; i++){
        for(j=i+1; j<StopPos; j++){
            if(cur_bbox_vec[j].x<cur_bbox_vec[i].x){
                //swap
                tmp_box=cur_bbox_vec[j];
                cur_bbox_vec[j]=cur_bbox_vec[i];
                cur_bbox_vec[i]=tmp_box;
            }
        }
    }

    //get the distance between each char, too close? select the highest prob.
    for(i=StartPos; i<StopPos-1; i++){
        i1=cur_bbox_vec[i].x; i2=cur_bbox_vec[i+1].x;
        d=(i2-i1)*2;            //d<0? two lines and jumping from the top to the bottom line.
        if(d>=0 && d<ch_wd){
            if(cur_bbox_vec[i+1].prob < cur_bbox_vec[i].prob) cur_bbox_vec.erase(cur_bbox_vec.begin()+i+1);
            else                                              cur_bbox_vec.erase(cur_bbox_vec.begin()+i);
            StopPos--;  i--;    //one element less in the array, due to the erase
        }
    }
}
//----------------------------------------------------------------------------------------
void SortPlate(vector<bbox_t>& cur_bbox_vec)
{
    size_t i, j, n, bnd;
    size_t len=cur_bbox_vec.size();
    bbox_t tmp_box;
    float prb,ch_wd, ch_ht;
    double A, B, R;
    TLinRegression LinReg;

    if(len < 2) return;         //no need to investigate 1 character

    //check nr of chars
    while(len > 10){
        //get the lowest probability
        for(prb=1000.0, i=0;i<len;i++){
            if(cur_bbox_vec[i].prob < prb){ prb=cur_bbox_vec[i].prob; n=i;}
        }
        //delete the lowest
        cur_bbox_vec.erase(cur_bbox_vec.begin()+n);
        len=cur_bbox_vec.size();
    }

    //get average width and height of the characters
    for(ch_ht=ch_wd=0.0, i=0; i<len; i++){
        ch_wd+=cur_bbox_vec[i].w;
        ch_ht+=cur_bbox_vec[i].h;
    }
    ch_wd/=len; ch_ht/=len;

    if(len > 4){
        //get linear regression through all (x,y)
        for(i=0; i<len; i++){
            LinReg.Add(cur_bbox_vec[i].x, cur_bbox_vec[i].y);
        }
        LinReg.Execute(A,B,R);
        //now you can do a warp perspective if the skew is too large.
        //in that case, you have to run the ocr detection again.
        //here we see how well a single line fits all the characters.
        //if the standard deviation is high, you have one line of text
        //if the R is low, you have a two-line number plate.

//        cout << "A = " << A << "  B = " << B << "  R = " << R << endl;
    }
    else{
        R=1.0;  // with 4 or fewer characters, assume we got always one line.
    }

    if(R<0.08 && A>-1.0 && A<1.0){
        //two lines -> sort on y first
        for(i=0; i<len; i++){
            for(j=i+1; j<len; j++){
                if(cur_bbox_vec[j].y<cur_bbox_vec[i].y){
                    //swap
                    tmp_box=cur_bbox_vec[j];
                    cur_bbox_vec[j]=cur_bbox_vec[i];
                    cur_bbox_vec[i]=tmp_box;
                }
            }
        }

        //get the boundary between the first and second line.
        for(n=0, i=0; i<len-1; i++){
            j=cur_bbox_vec[i+1].y-cur_bbox_vec[i].y;
            if(j>n){ n=j; bnd=i+1; }
        }
        //sort the first line 0 < bnd
        SortSingleLine(cur_bbox_vec, ch_wd, ch_ht, 0, bnd);

        len=cur_bbox_vec.size();        //SortSingleLine can shorten the length of the vector
        //sort the second line bnd < len
        SortSingleLine(cur_bbox_vec, ch_wd, ch_ht, bnd, len);
    }
    else{
        //one line -> sort by x position
        SortSingleLine(cur_bbox_vec, ch_wd, ch_ht, 0, len);
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
                    string filename = string(), int timeout = 400000, int port = 8070){
    string send_str;
    if (cur_bbox_vec.empty()) return false;  // Early return if no bounding boxes are detected
    char *tmp_buf = (char *)calloc(1024, sizeof(char));
    if (!filename.empty()) {
        sprintf(tmp_buf, "{\n \"frame_id\":%s, \n \"filename\":\"%s\", \n \"objects\": [ \n", frame_id.c_str(), filename.c_str());
    }
    else {
        sprintf(tmp_buf, "{\n \"frame_id\":%s, \n \"objects\": [ \n", frame_id.c_str());
    }
    send_str = tmp_buf;
    free(tmp_buf);

    for (auto & i : cur_bbox_vec) {
        char *buf = (char *)calloc(2048, sizeof(char));

        sprintf(buf, "  {\"class_id\":%d, \"name\":\"%s\", \"absolute_coordinates\":{\"center_x\":%d, \"center_y\":%d, \"width\":%d, \"height\":%d}, \"confidence\":%f",
            i.obj_id, obj_names[i.obj_id].c_str(), i.x, i.y, i.w, i.h, i.prob);

        send_str += buf;

        if (!isnan(i.z_3d)) {
            sprintf(buf, "\n    , \"coordinates_in_meters\":{\"x_3d\":%.2f, \"y_3d\":%.2f, \"z_3d\":%.2f}",
                i.x_3d, i.y_3d, i.z_3d);
            send_str += buf;
        }

        send_str += "}\n";

        free(buf);
    }

    send_str += "\n ] \n}";

    if(Js.Json_Folder!="none"){
        ofstream Jfile(Js.Json_Folder+"/"+frame_id);
        Jfile << send_str;
        Jfile.close();
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
int main()
{
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

    Success = Js.GetSettings();
    if(!Success){
        return -1;
    }

    cout << "ALPR Version : " << Js.Version << endl;

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
                cout<<"Input stream is closed"<<endl;
                break;
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

                    //send json into the world (port 8070)
                    send_json_http(result_ocr, OcrNames, cam.CurrentFileName+"_"+ChrCar+"_"+ChrPlate+"_utc.json");

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

