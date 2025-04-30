# ELI-ALPR on PapersWithCode with SoA YOLO in Darknet</p>

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
|**ELI-ALPR**|Efficient Layout-Independent Automatic License Plate Recognition|
|**SoA**|State of the Art|
|**YOLO**|You Only Look Once|
|**OCR**|Optical Character Recognition|
|**BBOX**|Bounding Box|
|**GPU**|Graphics Processing Unit|
|**CUDA**|Compute Unified Device Architecture|

---

### **Overview**
This repository presents an implementation of Automatic License Plate Recognition (ALPR), built upon state-of-the-art YOLO models (from the research listed on Papers with Code) using the Darknet framework. Our system achieves an impressive accuracy of 96.9%, as reported in the original research.

---
### **ELI-ALPR Pipeline Diagram**
Below is the pipeline diagram that illustrates the key components and workflow of the ELI-ALPR system:
```bash
(Vehicle Detection) -> (License Plate Detection) -> (License Plate Recognition)
```
![Pipeline_flowchart_image](https://github.com/user-attachments/assets/ac0853d0-282c-4a45-b926-34f9418eb572)

### **Pipeline Features**
The ELI-ALPR system works in a three-stage pipeline based on the original research papers:

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
### **Examples of Outputs from the Original Research**
------------

|IMAGE|DETECTIONS|RESULTS|
| :------------ | :------------ | :------------ |
|[![](https://web.inf.ufpr.br/vri/wp-content/uploads/sites/7/2019/09/predictions-vd.jpg)](https://web.inf.ufpr.br/vri/wp-content/uploads/sites/7/2019/09/predictions-vd.jpg)   | [![](https://web.inf.ufpr.br/vri/wp-content/uploads/sites/7/2019/09/predictions-lpd.jpg)](https://web.inf.ufpr.br/vri/wp-content/uploads/sites/7/2019/09/predictions-lpd.jpg)  | [![](https://web.inf.ufpr.br/vri/wp-content/uploads/sites/7/2019/09/predictions-lpr.jpg)](https://web.inf.ufpr.br/vri/wp-content/uploads/sites/7/2019/09/predictions-lpr.jpg)  |

------------


### **ELI-ALPR Image Output**

- The output image highlights **ELI-ALPR**'s performance, showcasing its ability to detect and recognize cars, bikes, and their license plates. Each plate is accurately identified, with bounding boxes drawn around the plates, and the recognized plate number displayed above the corresponding vehicle. This demonstrates the system’s effectiveness in both vehicle and license plate detection.

[ELI-ALPR_Academic_Image_output.webm](https://github.com/user-attachments/assets/e2f5cc77-7a99-4fde-b8a7-d9826a0effdc)


### **References**

For a deeper understanding of the YOLO-based approach and how it enhances ALPR performance, please refer to the original research papers

[1] R. Laroca, L. A. Zanlorensi, G. R. Gonçalves, E. Todt, W. R. Schwartz, D. Menotti, “An Efficient and Layout-Independent Automatic License Plate Recognition System Based on the YOLO Detector,” IET Intelligent Transport Systems, vol. 15, no. 4, pp. 483-503, 2021
https://web.inf.ufpr.br/vri/publications/layout-independent-alpr/

[2] R. Laroca, E. Severo, L. A. Zanlorensi, L. S. Oliveira, G. R. Gonçalves, W. R. Schwartz, D. Menotti, “A Robust Real-Time Automatic License Plate Recognition Based on the YOLO Detector,” in International Joint Conference on Neural Networks (IJCNN), July 2018, pp. 1–10.
https://web.inf.ufpr.br/vri/publications/laroca2018robust/


## **Acknowledgments**

- **OpenDataCam**  https://github.com/opendatacam/opendatacam/blob/release-v3.0.2/README.md
