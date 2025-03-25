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

---

### **Overview**
This repository presents an implementation of Automatic License Plate Recognition (ALPR), built upon state-of-the-art YOLO models (from the research listed on Papers with Code) using the Darknet framework. Our system achieves an impressive accuracy of 96.9%, as reported in the original research.

---
### **Features**
The original research introduced three models:

- **Vehicle Detection**: Identifies vehicles such as cars and bikes.
- **License Plate Detection**: Detects license plates across various geographies.
- **License Plate Recognition**: Recognizes alphanumeric characters without traditional OCR.

While the original approach involved manually cropping vehicles, license plates, and characters, our implementation automates the entire workflow. The system operates on a GPU/CUDA-enabled platform via Darknet, offering faster and more efficient performance.

---
### **Capabilities**

- Processes images, videos, and RTSP streams (live video).
- Provides real-time license plate recognition.
This solution enables quicker, fully automated license plate recognition, improving the overall accuracy and speed of real-world applications.

---
### **Configuration**

The `config.json` file contains all required settings. Without this file, the app will not start.

```json
{
  "VERSION": "1.0.0",
  "VIDEO_INPUT": "video",
  "VIDEO_INPUTS_PARAMS": {
    "image": "./images/4.jpg",
    "folder": "./inputs/images",
    "video": "./images/demo.mp4",
    "usbcam": "v4l2src device=/dev/video0 ! video/x-raw, framerate=30/1, width=640, height=360 ! videoconvert ! appsink",
    "CCTV": "rtsp://admin:Admin@12345@192.168.30.171/cam/realmonitor?channel=1&subtype=0",
    "remote_hls_gstreamer": "souphttpsrc location=http://YOUR_HLSSTREAM_URL_HERE.m3u8 ! hlsdemux ! decodebin ! videoconvert ! videoscale ! appsink"
  },

  "RoI": {
    "x_offset": 220,
    "y_offset": 500,
    "width": 1920,
    "height": 1080
  },

  "MJPEG_PORT": 8090,
  "MJPEG_WIDTH": 1920,
  "MJPEG_HEIGHT": 1080,

  "VEHICLE_MODEL": "./models/vehicle-detection",
  "LICENSE_MODEL": "./models/lp-detection-layout-classification",
  "OCR_MODEL": "./models/lp-recognition",

  "HEURISTIC_ON": false,

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
---
### **Input Options**

You can choose your video input source in the `config.json` file under `VIDEO_INPUTS_PARAMS`. Available options include:
Default choice is an RTSP video stream.

### VIDEO_INPUTS_PARAMS

| **Parameter**      | **Description**                                                     |
| ---------          | -----                                                               |
|  **image**         | Name and location of the picture. It must be a jpg or png file.     |
| **folder**         | Directory containing the pictures. They must be jpg or png.         |
|**video**           | Name and location of the video file. |
| **usbcam**         | The GStreamer pipeline connecting the ELI-ALPR to an USB camera.    |
| **remote_hls_gstreamer** | The GStreamer pipeline connecting the ELI_ALPR to an HLS source. |

---

### **RoI**

The coordinates of the cropped image that will be analyzed. All parameters are checked in advance.<br>
At run time, they can be modified if necessary to avoid crashes. The size and height take precedence over the x and y offset.

### **MJPEG_PORT**

The port number of the local host to which the video is streamed.

### **MJPEG_WIDTH MJPEG_HEIGHT**

The image size send to the local host.

### **_MODEL**

The name and location where the darknet deep learning models can be found.<br>
You need three sets: one for detecting the vehicle, one for detecting a license plate and one for optical character recognition.<br>
Each set constist of three files. The topology (.cfg) file, the weights (.weights) file and the labels (.names)

### **PRINT_ON_CLI**

When the boolean PRINT_ON_CLI is `true`, license plates are printed on the terminal.<br>
When `false`, the license plates are still detected and sent as JSON string to local port 8070, but are not shown on the terminal window.

### **PRINT_ON_RENDER**

When the boolean PRINT_ON_RENDER is `true`, vehicles, license plates and OCR outcomes are drawn into the output window.

### **_FOLDER**

Locations where debug images are stored. When the folder name is `none` the storage is inhibited.<br>
&nbsp;&nbsp;&nbsp;&nbsp;`FoI_FOLDER`: Every frame being analysed.<br>
&nbsp;&nbsp;&nbsp;&nbsp;`VEHICLES_FOLDER`: Every vehicle found.<br>
&nbsp;&nbsp;&nbsp;&nbsp;`PLATES_FOLDER`: Every lisence plate found.<br>
&nbsp;&nbsp;&nbsp;&nbsp;`JSONS_FOLDER`: OCR json files.<br>
&nbsp;&nbsp;&nbsp;&nbsp;`RENDERS_FOLDER`: Frames shown with boxes around found items.<br><br>
:point_right:&nbsp;&nbsp;Note that the amount of data stored can be _**huge**_! Within a few minutes, the SD card can be completely written.

### **HEURISTIC_ON**

When the boolean HEURISTIC_ON is set to `true` or `false`, the characters found are sorted by position, and duplicates are removed. This ensures that the correct OCR is printed.

### **THRESHOLD**

The threshold appied to the darknet model mentioned.

------------
### **Results**

- **JSON Files**: Results are saved in the `JSONS_FOLDER` directory.
- **Rendered Images**: Detected vehicles, license plates, and OCR outputs are saved as images (if enabled in the config).
- **Terminal Output**: License plates are printed on the CLI when `PRINT_ON_CLI` is enabled.

### **Examples**

------------

|IMAGE|DETECTIONS|RESULTS|
| :------------ | :------------ | :------------ |
|[![](https://web.inf.ufpr.br/vri/wp-content/uploads/sites/7/2019/09/predictions-vd.jpg)](https://web.inf.ufpr.br/vri/wp-content/uploads/sites/7/2019/09/predictions-vd.jpg)   | [![](https://web.inf.ufpr.br/vri/wp-content/uploads/sites/7/2019/09/predictions-lpd.jpg)](https://web.inf.ufpr.br/vri/wp-content/uploads/sites/7/2019/09/predictions-lpd.jpg)  | [![](https://web.inf.ufpr.br/vri/wp-content/uploads/sites/7/2019/09/predictions-lpr.jpg)](https://web.inf.ufpr.br/vri/wp-content/uploads/sites/7/2019/09/predictions-lpr.jpg)  |

------------


### **ELI-ALPR Image Output**

- The output image highlights ELI-ALPR's performance, showcasing its ability to detect and recognize cars, bikes, and their license plates. Each plate is accurately identified, with bounding boxes drawn around the plates, and the recognized plate number displayed above the corresponding vehicle. This demonstrates the system’s effectiveness in both vehicle and license plate detection.

[ELI-ALPR_Academic_Image_output.webm](https://github.com/user-attachments/assets/e2f5cc77-7a99-4fde-b8a7-d9826a0effdc)


### **References**

For a deeper understanding of the YOLO-based approach and how it enhances ALPR performance, please refer to the original research papers

[1] R. Laroca, L. A. Zanlorensi, G. R. Gonçalves, E. Todt, W. R. Schwartz, D. Menotti, “An Efficient and Layout-Independent Automatic License Plate Recognition System Based on the YOLO Detector,” IET Intelligent Transport Systems, vol. 15, no. 4, pp. 483-503, 2021
https://web.inf.ufpr.br/vri/publications/layout-independent-alpr/

[2] R. Laroca, E. Severo, L. A. Zanlorensi, L. S. Oliveira, G. R. Gonçalves, W. R. Schwartz, D. Menotti, “A Robust Real-Time Automatic License Plate Recognition Based on the YOLO Detector,” in International Joint Conference on Neural Networks (IJCNN), July 2018, pp. 1–10.
https://web.inf.ufpr.br/vri/publications/laroca2018robust/


## **Acknowledgments**

- **OpenDataCam**  https://github.com/opendatacam/opendatacam/blob/release-v3.0.2/README.md
