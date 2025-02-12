# ELI-ALPR on Paperswithcode with SoA YOLO in Darknet</p>

------------

|**Document Control**|Srikant Jakilinki|
| :------------ | :------------ |
|**Contact Points**|srikant.jakilinki@xinthe.com|
|**Approved By**|Sridhar Panuganti (sridhar@xinthe.com)|
|**Co-Developed By**|Q-Engineering (info@qengineering.eu)|
|**Status**|External Release|
|**Distribution List**|Management, QEngineering|

|**DAJA**|Definitions, Acronyms, Jargon and Abbreviations|
| :------------ | :------------ |
|**ELI-ALPR**|Efficient Layout Independent Automatic License Plate Recognition|
|**SoA**|State of Art|
|**YOLO**|You Only Look Once|
|**OCR**|Optical Character Recognition|
|**BBOX**|Bounding Box|
|**GPU**|Graphics Processing Unit|
|**CUDA**|Compute Unified Device Architecture|

### Overview
This repository presents our implementation of Automatic License Plate Recognition (ALPR), built upon state-of-the-art YOLO models (from the research listed on Papers with Code) using the Darknet framework. Our system achieves an impressive accuracy of 96.9%, as reported in the original research.

### Features
The original research introduced three models:

Vehicle Detection: Identifies vehicles such as cars and bikes.
License Plate Detection: Detects license plates across various geographies.
License Plate Recognition: Recognizes alphanumeric characters without traditional OCR.
While the original approach involved manually cropping vehicles, license plates, and characters, our implementation automates the entire workflow. The system operates on a GPU/CUDA-enabled platform via Darknet, offering faster and more efficient performance.

### Capabilities
Processes images, videos, and RTSP streams (live video).
Provides real-time license plate recognition.
This solution enables quicker, fully automated license plate recognition, improving the overall accuracy and speed of real-world applications.

### References
For a deeper understanding of the YOLO-based approach and how it enhances ALPR performance, please refer to the original research papers

[1] R. Laroca, L. A. Zanlorensi, G. R. Gonçalves, E. Todt, W. R. Schwartz, D. Menotti, “An Efficient and Layout-Independent Automatic License Plate Recognition System Based on the YOLO Detector,” IET Intelligent Transport Systems, vol. 15, no. 4, pp. 483-503, 2021
https://web.inf.ufpr.br/vri/publications/layout-independent-alpr/

[2] R. Laroca, E. Severo, L. A. Zanlorensi, L. S. Oliveira, G. R. Gonçalves, W. R. Schwartz, D. Menotti, “A Robust Real-Time Automatic License Plate Recognition Based on the YOLO Detector,” in International Joint Conference on Neural Networks (IJCNN), July 2018, pp. 1–10.
https://web.inf.ufpr.br/vri/publications/laroca2018robust/

------------

|IMAGE|DETECTIONS|RESULTS|
| :------------ | :------------ | :------------ |
|[![](https://web.inf.ufpr.br/vri/wp-content/uploads/sites/7/2019/09/predictions-vd.jpg)](https://web.inf.ufpr.br/vri/wp-content/uploads/sites/7/2019/09/predictions-vd.jpg)   | [![](https://web.inf.ufpr.br/vri/wp-content/uploads/sites/7/2019/09/predictions-lpd.jpg)](https://web.inf.ufpr.br/vri/wp-content/uploads/sites/7/2019/09/predictions-lpd.jpg)  | [![](https://web.inf.ufpr.br/vri/wp-content/uploads/sites/7/2019/09/predictions-lpr.jpg)](https://web.inf.ufpr.br/vri/wp-content/uploads/sites/7/2019/09/predictions-lpr.jpg)  |

------------

## Dependencies.
To run the application, you need to have:
- OpenCV 64-bit installed.
- Darknet ([the Alexey version](https://github.com/AlexeyAB/darknet)) installed.
- MongoDB, Node.js and JSON for C++ installed.
- The darknet models downloaded from Gdrive.
- Code::Blocks installed, if you like to work with the C++ source. 

### Installing the dependencies.
Start with some evergreens
```
$ sudo apt-get update 
$ sudo apt-get upgrade
$ sudo apt-get install curl libcurl3
$ sudo apt-get install cmake wget
$ sudo apt-get install libcanberra-gtk*
```
#### Node.js
```
$ curl -sL https://deb.nodesource.com/setup_12.x | sudo -E bash -
$ sudo apt-get install -y nodejs
```
#### MongoDB
```
$ sudo apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv 9DA31620334BD75D9DCB49F368818C72E52529D4
$ echo "deb [ arch=amd64,arm64 ] https://repo.mongodb.org/apt/ubuntu xenial/mongodb-org/4.0 multiverse" | sudo tee /etc/apt/sources.list.d/mongodb-org-4.0.list
$ sudo apt-get update
$ sudo apt-get install -y openssl mongodb-org
# Start service
$ sudo systemctl start mongod
# Enable service on boot
$ sudo systemctl enable mongod
```
#### JSON for C++
written by [Niels Lohmann](https://github.com/nlohmann).
```
$ git clone https://github.com/nlohmann/json.git
$ cd json
$ mkdir build
$ cd build
$ cmake ..
$ make -j4
$ sudo make install
```
#### Code::Blocks
```
$ sudo apt-get install codeblocks
```
#### OpenCV
```
$ git clone --depth=1 https://github.com/opencv/opencv.git
$ git clone --depth=1 https://github.com/opencv/opencv_contrib.git 
$ cd ~/opencv
$ mkdir build
$ cd build
$ cmake -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr -D OPENCV_EXTRA_MODULES_PATH=~/opencv_contrib/modules -D EIGEN_INCLUDE_PATH=/usr/include/eigen3  -D WITH_OPENCL=OFF  -D WITH_CUDA=ON  -D CUDA_ARCH_BIN=7.5  -D CUDA_ARCH_PTX="sm_75"  -D WITH_CUDNN=ON  -D WITH_CUBLAS=ON  -D ENABLE_FAST_MATH=ON  -D CUDA_FAST_MATH=ON  -D OPENCV_DNN_CUDA=OFF   -D WITH_QT=OFF  -D WITH_OPENMP=ON  -D BUILD_TIFF=ON  -D WITH_FFMPEG=ON  -D WITH_GSTREAMER=ON  -D WITH_TBB=ON  -D BUILD_TBB=ON  -D BUILD_TESTS=OFF  -D WITH_EIGEN=ON  -D WITH_V4L=ON  -D WITH_LIBV4L=ON  -D WITH_PROTOBUF=ON  -D OPENCV_ENABLE_NONFREE=ON  -D INSTALL_C_EXAMPLES=OFF  -D INSTALL_PYTHON_EXAMPLES=OFF  -D PYTHON3_PACKAGES_PATH=/usr/lib/python3/dist-packages  -D OPENCV_GENERATE_PKGCONFIG=ON  -D BUILD_EXAMPLES=OFF .. 
-Note:
- Need to change the values of CUDA_ARCH_BIN & CUDA_ARCH_PTX. It depends on your GPU model.

$ make -j4
$ sudo rm -r /usr/include/opencv4/opencv2
$ sudo make install
- cleaning (frees 300 MB)
$ make clean
$ sudo apt-get update

- To check whether Opencv is installed or not
$ dpkg -l | grep libopencv
```
Follow this [guide](https://qengineering.eu/install-opencv-4.5-on-jetson-nano.html).
#### Darknet Installation

$ git clone https://github.com/AlexeyAB/darknet.git

```bash
- modify the Flags like OpenCV, CUDNN, GPU and change the compute arch value based on the device
$ cd darknet/
$ nano Makefile

- Edit and configure “Makefile” to enable all required libraries 
$ GPU=1 
$ OPENCV=1 
$ CUDNN=1 
$ OPENMP=1

- Execute the command “make” in the build_ directory 
$ make

- compile darknet with modifications
$ make clean
$ make -j8
```
Follow this [guide](https://qengineering.eu/install-darknet-on-jetson-nano.html).

#### Darknet models.
Due to their large size, all darknet models are stored at Gdrive [ALPR_models.zip](https://drive.google.com/file/d/1UCQi0BwtzOgcblaIPGi_V0Yim2yXBHKI/view?usp=share_link).<br>
After downloading you can unzip and save the models in the appropriate folder.

------------

## Installing the app.
To extract and run the network in Code::Blocks <br/>
$ mkdir *MyDir* <br/>
$ cd *MyDir* <br/>
$ git clone https://github.com/xactai/ELI-ALPR.git
Your *MyDir* folder must now look like this: <br/> 
```
.
├── models
│   ├── lp-detection-layout-classification.cfg
│   ├── lp-detection-layout-classification.data
│   ├── lp-detection-layout-classification.names
│   ├── lp-detection-layout-classification.weights
│   ├── lp-recognition.cfg
│   ├── lp-recognition.data
│   ├── lp-recognition.names
│   ├── lp-recognition.weights
│   ├── vehicle-detection.cfg
│   ├── vehicle-detection.data
│   ├── vehicle-detection.names
│   └── vehicle-detection.weights
├── include
│   ├── darkunistd.h
│   ├── http_stream.h
│   ├── MJPG_sender.h
│   ├── Numbers.h
│   ├── Regression.h
│   ├── RTSPcam.h
│   └── Tjson.h
├── src
│   ├── http_stream.cpp
│   ├── MJPG_sender.cpp
│   ├── main.cpp
│   ├── Regression.cpp
│   ├── RTSPcam.cpp
│   └── Tjson.cpp
├── CMakeLists.txt
├── config.json
└── YOLO_ALPR.cbp
```
------------

## Config.json.
All required settings are listed in the `config.json` file. Without this file, the app will not start.
```json
{
  "VERSION": "1.0.0",
  "VIDEO_INPUT": "video",
  "VIDEO_INPUTS_PARAMS": {
    "image": "/home/xactai/Pictures/academic-sample-image.jpg",
    "folder": "/home/xactai/Pictures/Academic+NovotelOUT-Images",
    "video": "/home/xactai/Videos/Novotel_Entry_22-mins_H264.mp4",
    "usbcam": "v4l2src device=/dev/video0 ! video/x-raw, framerate=30/1, width=640, height=360 ! videoconvert ! appsink",
    "CSI1": "nvarguscamerasrc sensor_id=0 ! video/x-raw(memory:NVMM),width=640, height=480, framerate=15/1, format=NV12 ! nvvidconv ! video/x-raw, format=BGRx, width=640, height=480 ! videoconvert ! video/x-raw, format=BGR ! appsink",
    "CSI2": "nvarguscamerasrc sensor_id=1 ! video/x-raw(memory:NVMM),width=640, height=480, framerate=15/1, format=NV12 ! nvvidconv ! video/x-raw, format=BGRx, width=640, height=480 ! videoconvert ! video/x-raw, format=BGR ! appsink",
    "CCTV": "rtsp://admin:Admin@12345@192.168.30.171/cam/realmonitor?channel=1&subtype=0",
    "remote_hls_gstreamer": "souphttpsrc location=http://YOUR_HLSSTREAM_URL_HERE.m3u8 ! hlsdemux ! decodebin ! videoconvert ! videoscale ! appsink"
  },

  "RoI": {
    "x_offset": 220,
    "y_offset": 500,
    "width": 640,
    "height": 480
  },

  "MJPEG_PORT": 8090,
  "MJPEG_WIDTH": 320,
  "MJPEG_HEIGHT": 240,

  "VEHICLE_MODEL": "./models/vehicle-detection",
  "LICENSE_MODEL": "./models/lp-detection-layout-classification",
  "OCR_MODEL": "./models/lp-recognition",

  "HEURISTIC_ON": true,

  "PRINT_ON_CLI": true,
  "PRINT_ON_RENDER": true,

  "FoI_FOLDER": "none",
  "VEHICLES_FOLDER": "none",
  "PLATES_FOLDER": "none",
  "JSONS_FOLDER": "./outputs/jsons",
  "RENDERS_FOLDER": "none",

  "THRESHOLD_VEHICLE": 0.01,
  "THRESHOLD_PLATE": 0.01,
  "THRESHOLD_OCR": 0.5
}
```
#### VIDEO_INPUT
Select your video input. It can be one of the sources listed under `VIDEO_INPUTS_PARAMS`:<br>
`file, movie, usbcam, raspberrycam, remote_cam or remote_hls_gstreamer`.<br>
Default choice is an RTSP video stream.
#### VIDEO_INPUTS_PARAMS
| Item      | Description |
| --------- | -----|
| image  | Name and location of the picture. It must be a jpg or png file. |
| folder  | Directory containing the pictures. They must be jpg or png. |
| video | Name and location of the video file. |
| usbcam  | The GStreamer pipeline connecting the ELI-ALPR to an USB camera. |
| CSI1 | The GStreamer pipeline connecting the ELI-ALPR to an MIPI camera (port 0). |
| CSI2 | The GStreamer pipeline connecting the ELI-ALPR to an MIPI camera (port 1). |
| CCTV | The GStreamer pipeline connecting the ELI-ALPR to an RTSP source. |
| remote_hls_gstreamer | The GStreamer pipeline connecting the ELI_ALPR to an HLS source. |
#### RoI
The coordinates of the cropped image that will be analyzed. All parameters are checked in advance.<br>
At run time, they can be modified if necessary to avoid crashes. The size and height take precedence over the x and y offset.
#### MJPEG_PORT
The port number of the local host to which the video is streamed.
#### MJPEG_WIDTH MJPEG_HEIGHT
The image size send to the local host.
#### _MODEL
The name and location where the darknet deep learning models can be found.<br>
You need three sets: one for detecting the vehicle, one for detecting a license plate and one for optical character recognition.<br>
Each set constist of three files. The topology (.cfg) file, the weights (.weights) file and the labels (.names).
#### PRINT_ON_CLI
When the boolean PRINT_ON_CLI is `true`, license plates are printed on the terminal.<br>
When `false`, the license plates are still detected and sent as JSON string to local port 8070, but are not shown on the terminal window.
#### PRINT_ON_RENDER
When the boolean PRINT_ON_RENDER is `true`, vehicles, license plates and OCR outcomes are drawn into the output window.
#### _FOLDER
Locations where debug images are stored. When the folder name is `none` the storage is inhibited.<br>
&nbsp;&nbsp;&nbsp;&nbsp;`FoI_FOLDER`: Every frame being analysed.<br>
&nbsp;&nbsp;&nbsp;&nbsp;`VEHICLES_FOLDER`: Every vehicle found.<br>
&nbsp;&nbsp;&nbsp;&nbsp;`PLATES_FOLDER`: Every lisence plate found.<br>
&nbsp;&nbsp;&nbsp;&nbsp;`JSONS_FOLDER`: OCR json files.<br>
&nbsp;&nbsp;&nbsp;&nbsp;`RENDERS_FOLDER`: Frames shown with boxes around found items.<br><br>
:point_right:&nbsp;&nbsp;Note that the amount of data stored can be _**huge**_! Within a few minutes, the SD card can be completely written.
#### HEURISTIC_ON
When the boolean HEURISTIC_ON is `true`, the characters found are sorted on position and doubles are removed.
#### THRESHOLD_
The threshold appied to the darknet model mentioned

------------

## Running the app.
Before compiling and running the application, modify the config.json to your situation.<br>
Once done, you have two options.<br>
#### CMake
The first option is using CMake. Please follow the next commands
```
$ cd *MyDir* <br/>
$ mkdir build
$ cd build
$ cmake ..
$ make
```
Find your ALPR app in *MyDir*<br> 
![Installation steps](outputs/Images/Installation_steps_ELI_ALPR.png)
![ELI-ALPR application](outputs/Images/ELI-ALPR_Succesfully_installed.png)
#### Code::Blocks
The second option is using Code::Blocks, an GNU C++ IDE.<br>
Load the project file YOLO_ALPR.cbp in Code::Blocks and run `<F9>`.<br>
More info follow the instructions at [Hands-On](https://qengineering.eu/deep-learning-examples-on-raspberry-32-64-os.html#HandsOn).<br/>


###  Acknowledgments

- OpenDataCam  https://github.com/opendatacam/opendatacam/blob/release-v3.0.2/README.md