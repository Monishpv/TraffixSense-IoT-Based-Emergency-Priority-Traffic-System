# 🚦 TraffixSense

TraffixSense is an IoT-based Emergency Priority Traffic Management System developed using ESP32, RFID, OLED Display, and a real-time web dashboard.

The system automatically controls traffic signals and provides priority access for emergency vehicles like ambulances using RFID-based detection.

When an emergency vehicle is detected, the corresponding traffic lane immediately turns green while other lanes remain red, helping reduce response time and improve traffic management efficiency.

## 🔥 Key Features

- 🚑 RFID-based ambulance priority system
- 🌐 Real-time traffic monitoring dashboard
- 📡 ESP32 WiFi connectivity
- 📟 OLED live traffic status display
- 🔄 Automatic traffic signal control
- 🚨 Manual emergency override support
- ⏱ Configurable traffic signal timing
- 🔁 Auto reset after emergency clearance

## 🛠 Technologies Used

### Hardware
- ESP32
- RFID Module
- OLED Display
- LEDs
- Breadboard

### Software
- Arduino IDE
- Embedded C++
- HTML/CSS/JavaScript
- ESP32 WebServer

## ⚙️ Working Process

1. Traffic lights run automatically in normal mode.
2. RFID detects emergency vehicle access.
3. Emergency mode activates instantly.
4. Required lane gets green signal priority.
5. Dashboard and OLED update in real time.
6. System resets back to automatic mode after completion.

## 📌 Applications

- Smart Cities
- Emergency Traffic Systems
- Intelligent Transportation
- IoT-Based Automation Projects



---

# 🛠 Hardware Components

| Component | Quantity |
|---|---|
| ESP32 | 1 |
| RFID Module | 1 |
| OLED Display (SSD1306) | 1 |
| LEDs (Red/Yellow/Green) | Multiple |
| Resistors | Multiple |
| Breadboard | 1 |
| Jumper Wires | Multiple |
| Power Supply | 1 |

---

# 💻 Software Requirements

- Arduino IDE
- ESP32 Board Package
- Adafruit GFX Library
- Adafruit SSD1306 Library

---

# 📦 Required Libraries

Install these libraries in Arduino IDE:

```txt
WiFi.h
WebServer.h
Wire.h
Adafruit_GFX.h
Adafruit_SSD1306.h
```

---

# ⚙️ Installation

## 1. Install Arduino IDE

Download:

https://www.arduino.cc/en/software

---

## 2. Install ESP32 Board

Open Arduino IDE:

```text
File → Preferences
```

Add this URL in:

```text
Additional Boards Manager URLs
```

```text
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```

Then:

```text
Tools → Board → Boards Manager
```

Search:

```text
ESP32
```

Install ESP32 board package.

---

## 3. Install Required Libraries

Go to:

```text
Sketch → Include Library → Manage Libraries
```

Install:

- Adafruit GFX
- Adafruit SSD1306

---

# 📂 Project Setup

Clone repository:

```bash
git clone https://github.com/YOUR_USERNAME/TraffixSense.git
```

Open:

```text
traffixsense.ino
```

in Arduino IDE.

---

# 🔧 Important Configuration

## Change WiFi Credentials

Find this section:

```cpp
const char* ssid = "YOUR_WIFI";
const char* password = "YOUR_PASSWORD";
```

Replace with your WiFi name and password.

Example:

```cpp
const char* ssid = "MyWiFi";
const char* password = "mypassword";
```

---

# 📡 Uploading Code to ESP32

## Select Board

```text
Tools → Board → ESP32 Dev Module
```

## Select COM Port

```text
Tools → Port → COMx
```

## Upload

Click:

```text
Upload
```

---

# 🌐 Access Web Dashboard

After uploading:

1. Open Serial Monitor
2. Set baud rate to:

```text
9600
```

3. Copy the ESP32 IP address

Example:

```text
192.168.1.5
```

4. Open browser:

```text
http://192.168.1.5
```

---

# 🚑 RFID Emergency System

## RFID Tap Pattern

| Tap Count | Direction |
|---|---|
| 1 Tap | North |
| 2 Taps | South |
| 3 Taps | East |

---

# 🔄 System Workflow

1. Traffic signals operate automatically
2. RFID ambulance signal detected
3. Emergency mode activates
4. Priority green signal provided
5. Dashboard updates in real time
6. System auto-resets after 30 seconds

---

# 📷 Project Screenshots

Add screenshots inside:

```text
/images

```

<img width="1355" height="458" alt="image" src="https://github.com/user-attachments/assets/b33d2384-cc05-4fa0-a60e-d4c861bd52c0" />
<img width="898" height="660" alt="image" src="https://github.com/user-attachments/assets/26c3212e-d608-409c-938f-883f184fe6c6" />
<img width="484" height="403" alt="image" src="https://github.com/user-attachments/assets/f08b2589-b9f5-4668-a85f-8ae5bed88f86" />


Example:

- Dashboard Screenshot
- Hardware Setup
- Circuit Diagram

---

# 📁 Recommended Folder Structure

```text
TraffixSense/
│
├── README.md
├── LICENSE
├── images/
│   ├── dashboard.png
│   ├── hardware.jpg
│   └── circuit.png
│
├── code/
│   └── traffixsense.ino
│
└── docs/
    └── report.pdf
```

---

# 🚀 Future Improvements

- AI-based traffic prediction
- Vehicle density detection
- Mobile application
- Cloud monitoring
- GPS ambulance tracking
- Camera-based traffic analysis

---


---

# 📄 License

This project is developed for educational and research purposes.
