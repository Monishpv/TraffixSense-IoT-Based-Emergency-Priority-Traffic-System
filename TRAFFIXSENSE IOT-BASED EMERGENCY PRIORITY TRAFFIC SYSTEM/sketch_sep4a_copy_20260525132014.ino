#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ===== WiFi Credentials =====
const char* ssid = "OnePlus Nord CE3";
const char* password = "MONISH28";

// ===== WebServer =====
WebServer server(80);

// ===== OLED Setup =====
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ===== Emergency LED GPIO2 =====
#define emergencyLED 2

// ===== Traffic Lights Pin Mapping =====
int traffic[3][3] = {
  {13, 12, 14},  // North (Red, Yellow, Green)
  {27, 26, 25},  // East
  {33, 32, 23}   // South
};
String directions[3] = {"North", "East", "South"};

// ===== State Control =====
int currentLight = 0;
unsigned long lastChange = 0;
int phase = 0; // 0=Green,1=Yellow,2=Red
bool emergencyMode = false;
int emergencyActive = -1;

// ===== Timing (per-direction, ms) =====
unsigned long greenTimeMs[3]  = {5000, 5000, 5000};
unsigned long yellowTimeMs[3] = {2000, 2000, 2000};
unsigned long redTimeMs[3]    = {1000, 1000, 1000};

// ===== Status helpers =====
int remainingSeconds = 0;

// ===== RFID Variables =====
String rfidMessage = "";
unsigned long lastRfidTime = 0;
bool rfidEmergency = false;

// ===== HTML PAGE =====
String htmlPage() {
  String page = R"rawliteral(
<html>
<head>
<title>Smart Traffic Project</title>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1"/>
<style>
:root{--bg:#0d1a26;--card:#181f26;--muted:#cfeee8;}
body{font-family:Arial;background:var(--bg);color:var(--muted);margin:0;padding:16px;display:flex;flex-direction:column;align-items:center;}
h1{margin:6px 0 14px 0;}
.container{display:flex;gap:18px;flex-wrap:wrap;justify-content:center;}
.signal{background:var(--card);padding:10px;border-radius:12px;width:150px;text-align:center;}
.lights{height:110px;display:flex;flex-direction:column;justify-content:space-around;align-items:center;padding-top:6px;}
.light{width:46px;height:46px;border-radius:50%;background:#071013;opacity:0.18;transition:all .12s;}
.red{background:linear-gradient(180deg,#990000,#ff5555);}
.yellow{background:linear-gradient(180deg,#b38f00,#ffd400);}
.green{background:linear-gradient(180deg,#007a00,#00ff66);}
.active{opacity:1;box-shadow:0 0 12px rgba(0,255,200,0.08);}
.controls{margin-top:8px;background:#06131a;padding:8px;border-radius:8px;}
input[type=number]{width:54px;padding:6px;border-radius:6px;border:1px solid #123;background:#041018;color:white}
.btn{padding:8px 10px;border-radius:8px;border:none;cursor:pointer;font-weight:600;margin-top:6px;}
.apply{background:#00b3ff;color:#001;}
.reset{background:#ff6b6b;color:#fff;margin-left:8px;}
.statusBar{width:100%;max-width:720px;margin-top:16px;background:#071b22;padding:10px;border-radius:8px;text-align:center;}
.small{font-size:13px;color:#9fb7bf;}
.rfidInfo{background:#1a2b3a;padding:10px;border-radius:8px;margin-top:10px;text-align:center;color:#ffcc00;}
</style>

<script>
async function update(){
  try{
    const res = await fetch('/status');
    const d = await res.json();

    ['red','yellow','green'].forEach(c=>{
      for(let i=0;i<3;i++){
        document.getElementById(c+i).className='light '+c;
      }
    });

    // set input values with durations in seconds
    for(let i=0;i<3;i++){
      document.getElementById('g'+i).value = Math.round(d.durations[i].green/1000);
      document.getElementById('y'+i).value = Math.round(d.durations[i].yellow/1000);
      document.getElementById('r'+i).value = Math.round(d.durations[i].red/1000);
    }

    if(d.emergency){
      let a = d.emergencyActive;
      document.getElementById('green'+a).classList.add('active');
      let reason = d.rfidEmergency ? '🚑 RFID Ambulance' : 'Emergency';
      document.getElementById('statusText').innerText = reason + ' — ' + d.directions[a] + ' Green';
      document.getElementById('timerText').innerText = '';
      
      // Update RFID info
      document.getElementById('rfidInfo').innerHTML = '🚑 RFID Ambulance Active<br>' + 
        d.directions[a] + ' has GREEN signal';
      document.getElementById('rfidInfo').style.backgroundColor = '#004d00';
    } else {
      let a = d.currentLight;
      let p = d.phase;
      let id = (p==0?'green':(p==1?'yellow':'red')) + a;
      document.getElementById(id).classList.add('active');
      document.getElementById('statusText').innerText = 'Auto — ' + d.directions[a] + ' (' + (p==0?'Green':(p==1?'Yellow':'Red')) + ')';
      document.getElementById('timerText').innerText = 'Time left: ' + d.remainingSeconds + 's';
      
      // Update RFID info
      document.getElementById('rfidInfo').innerHTML = 'RFID Ready<br>' +
        'Tap Pattern: 1=North, 2=South, 3=East';
      document.getElementById('rfidInfo').style.backgroundColor = '#1a2b3a';
    }
  } catch(e){
    document.getElementById('statusText').innerText = 'No response';
  }
}

function applyDir(i){
  let g = document.getElementById('g'+i).value || 0;
  let y = document.getElementById('y'+i).value || 0;
  let r = document.getElementById('r'+i).value || 0;
  fetch('/settimes?dir='+i+'&g='+g+'&y='+y+'&r='+r).then(()=>setTimeout(update,200));
}

function applyAll(){
  let g = document.getElementById('gAll').value || 0;
  let y = document.getElementById('yAll').value || 0;
  let r = document.getElementById('rAll').value || 0;
  fetch('/setall?g='+g+'&y='+y+'&r='+r).then(()=>setTimeout(update,200));
}

function emergency(dir){
  fetch('/traffic'+(dir+1)).then(()=>setTimeout(update,100));
}

function resetAuto(){
  fetch('/reset').then(()=>setTimeout(update,150));
}

setInterval(update,500);
window.onload = update;
</script>
</head>
<body>
<h1>🚦 Smart Traffic — Control Panel</h1>

<div id="rfidInfo" class="rfidInfo">
  RFID Ready<br>
  Tap Pattern: 1=North, 2=South, 3=East
</div>

<div class="container">
  <div class="signal">
    <div class="lights">
      <div id="red0" class="light red"></div>
      <div id="yellow0" class="light yellow"></div>
      <div id="green0" class="light green"></div>
    </div>
    <div class="small">North</div>
    <div class="controls">
      <div>G <input id="g0" type="number" min="0"> s</div>
      <div>Y <input id="y0" type="number" min="0"> s</div>
      <div>R <input id="r0" type="number" min="0"> s</div>
      <button class="btn apply" onclick="applyDir(0)">Apply North</button>
    </div>
  </div>

  <div class="signal">
    <div class="lights">
      <div id="red1" class="light red"></div>
      <div id="yellow1" class="light yellow"></div>
      <div id="green1" class="light green"></div>
    </div>
    <div class="small">East</div>
    <div class="controls">
      <div>G <input id="g1" type="number" min="0"> s</div>
      <div>Y <input id="y1" type="number" min="0"> s</div>
      <div>R <input id="r1" type="number" min="0"> s</div>
      <button class="btn apply" onclick="applyDir(1)">Apply East</button>
    </div>
  </div>

  <div class="signal">
    <div class="lights">
      <div id="red2" class="light red"></div>
      <div id="yellow2" class="light yellow"></div>
      <div id="green2" class="light green"></div>
    </div>
    <div class="small">South</div>
    <div class="controls">
      <div>G <input id="g2" type="number" min="0"> s</div>
      <div>Y <input id="y2" type="number" min="0"> s</div>
      <div>R <input id="r2" type="number" min="0"> s</div>
      <button class="btn apply" onclick="applyDir(2)">Apply South</button>
    </div>
  </div>
</div>

<div class="statusBar">
  <div style="margin-bottom:8px;">
    <button class="btn" onclick="emergency(0)">🚑 North Ambulance</button>
    <button class="btn" onclick="emergency(1)">🚑 East Ambulance</button>
    <button class="btn" onclick="emergency(2)">🚑 South Ambulance</button>
    <button class="btn reset" onclick="resetAuto()">🔄 Reset</button>
  </div>

  <div style="margin:8px 0;">
    Set all: G <input id="gAll" type="number" min="0"> s
    Y <input id="yAll" type="number" min="0"> s
    R <input id="rAll" type="number" min="0"> s
    <button class="btn apply" onclick="applyAll()">Apply All</button>
  </div>

  <div id="statusText">Loading...</div>
  <div id="timerText" class="small" style="margin-top:6px;"></div>
</div>
</body>
</html>
)rawliteral";
  return page;
}

// ===== STATUS API =====
String getStatus() {
  String s = "{";
  if (emergencyMode && emergencyActive >= 0) {
    s += "\"emergency\":true,";
    s += "\"rfidEmergency\":" + String(rfidEmergency ? "true" : "false") + ",";
    s += "\"emergencyActive\":"+String(emergencyActive)+",";
  } else {
    s += "\"emergency\":false,";
    s += "\"rfidEmergency\":false,";
    s += "\"currentLight\":"+String(currentLight)+",";
    s += "\"phase\":"+String(phase)+",";
    s += "\"remainingSeconds\":"+String(remainingSeconds)+",";
  }
  
  s += "\"durations\":[";
  for (int i=0;i<3;i++){
    s += "{\"green\":"+String(greenTimeMs[i])+",\"yellow\":"+String(yellowTimeMs[i])+",\"red\":"+String(redTimeMs[i])+"}";
    if(i<2) s += ",";
  }
  s += "],";
  s += "\"directions\":[\"North\",\"East\",\"South\"]";
  s += "}";
  return s;
}

// ===== Traffic Helpers =====
void setTrafficSingle(int index,int state){
  digitalWrite(traffic[index][0],LOW);
  digitalWrite(traffic[index][1],LOW);
  digitalWrite(traffic[index][2],LOW);
  if(state==0) digitalWrite(traffic[index][2],HIGH);
  else if(state==1) digitalWrite(traffic[index][1],HIGH);
  else digitalWrite(traffic[index][0],HIGH);
}

void showAllOnOLED(int active,const String &ph,int countdown){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  for(int i=0;i<3;i++){
    display.setCursor(0,i*18);
    display.print(directions[i]); display.print(": ");
    if(i==active){ display.print(ph); display.print(" "); display.print(countdown); display.print("s"); }
    else display.print("Red");
  }
  display.display();
}

void showEmergencyOLED(int active, bool fromRFID){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  if (fromRFID) {
    display.print("RFID Ambulance");
  } else {
    display.print("Emergency Ambulance");
  }
  for(int i=0;i<3;i++){
    display.setCursor(0,(i+1)*15);
    display.print(directions[i]); display.print(": ");
    if(i==active) display.print("Green");
    else display.print("Red");
  }
  display.display();
}

// ===== Auto Mode =====
void autoTraffic(){
  unsigned long now = millis();
  unsigned long duration = (phase==0 ? greenTimeMs[currentLight] : (phase==1 ? yellowTimeMs[currentLight] : redTimeMs[currentLight]));
  if (now - lastChange >= duration) {
    phase++;
    if (phase > 2) {
      phase = 0;
      currentLight++;
      if (currentLight > 2) currentLight = 0;
    }
    lastChange = now;
    duration = (phase==0 ? greenTimeMs[currentLight] : (phase==1 ? yellowTimeMs[currentLight] : redTimeMs[currentLight]));
  }

  for(int i=0;i<3;i++){
    if(i==currentLight) setTrafficSingle(i,phase);
    else setTrafficSingle(i,2);
  }

  unsigned long elapsed = now - lastChange;
  long remMs = (long)duration - (long)elapsed;
  if(remMs < 0) remMs = 0;
  remainingSeconds = (int)((remMs + 999) / 1000);

  showAllOnOLED(currentLight, (phase==0?"Green":phase==1?"Yellow":"Red"), remainingSeconds);
}

// ===== Emergency Mode =====
void emergencyTraffic(){
  for(int i=0;i<3;i++){
    if(i==emergencyActive) setTrafficSingle(i,0);
    else setTrafficSingle(i,2);
  }
  showEmergencyOLED(emergencyActive, rfidEmergency);
}

// ===== RFID Handler =====
void handleRFIDInput() {
  // Check for serial input from Arduino Nano
  while (Serial.available() > 0) {
    char c = Serial.read();
    
    if (c == '\n' || c == '\r') {
      // End of message - process it
      if (rfidMessage.length() > 0) {
        Serial.print("RFID Message: ");
        Serial.println(rfidMessage);
        
        // Check for tap pattern commands
        if (rfidMessage == "NORTH" || rfidMessage == "AMBULANCE_NORTH") {
          // 1 tap = North
          emergencyMode = true;
          emergencyActive = 0;  // North
          rfidEmergency = true;
          lastRfidTime = millis();
          Serial.println("Activated: North Emergency (1 tap)");
        }
        else if (rfidMessage == "SOUTH" || rfidMessage == "AMBULANCE_SOUTH") {
          // 2 taps = South
          emergencyMode = true;
          emergencyActive = 2;  // South
          rfidEmergency = true;
          lastRfidTime = millis();
          Serial.println("Activated: South Emergency (2 taps)");
        }
        else if (rfidMessage == "EAST" || rfidMessage == "AMBULANCE_EAST") {
          // 3 taps = East
          emergencyMode = true;
          emergencyActive = 1;  // East
          rfidEmergency = true;
          lastRfidTime = millis();
          Serial.println("Activated: East Emergency (3 taps)");
        }
        else if (rfidMessage == "RESET" || rfidMessage == "STOP") {
          // Reset to normal mode
          emergencyMode = false;
          emergencyActive = -1;
          rfidEmergency = false;
          lastChange = millis();
          Serial.println("RFID: Reset to normal mode");
        }
        
        rfidMessage = ""; // Clear message buffer
      }
    } else {
      // Add character to message (ignore newlines)
      if (c != '\n' && c != '\r') {
        rfidMessage += c;
      }
    }
  }
  
  // Auto-reset after 30 seconds of RFID emergency
  if (rfidEmergency && (millis() - lastRfidTime > 30000)) {
    emergencyMode = false;
    emergencyActive = -1;
    rfidEmergency = false;
    lastChange = millis();
    Serial.println("Auto-reset after 30 seconds");
  }
}

// ===== Handlers to set times =====
void handleSetTimes(){
  if(!server.hasArg("dir")) { server.send(400,"text/plain","Missing dir"); return; }
  int dir = server.arg("dir").toInt();
  if(dir < 0 || dir > 2) { server.send(400,"text/plain","Invalid dir"); return; }
  if(server.hasArg("g")) greenTimeMs[dir] = (unsigned long)server.arg("g").toInt() * 1000UL;
  if(server.hasArg("y")) yellowTimeMs[dir] = (unsigned long)server.arg("y").toInt() * 1000UL;
  if(server.hasArg("r")) redTimeMs[dir] = (unsigned long)server.arg("r").toInt() * 1000UL;

  if(dir == currentLight){
    unsigned long curDur = (phase==0?greenTimeMs[currentLight]:(phase==1?yellowTimeMs[currentLight]:redTimeMs[currentLight]));
    unsigned long now = millis();
    if(now - lastChange > curDur) lastChange = now;
  }

  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "");
}

void handleSetAll(){
  if(server.hasArg("g")){
    unsigned long g = (unsigned long)server.arg("g").toInt() * 1000UL;
    for(int i=0;i<3;i++) greenTimeMs[i] = g;
  }
  if(server.hasArg("y")){
    unsigned long y = (unsigned long)server.arg("y").toInt() * 1000UL;
    for(int i=0;i<3;i++) yellowTimeMs[i] = y;
  }
  if(server.hasArg("r")){
    unsigned long r = (unsigned long)server.arg("r").toInt() * 1000UL;
    for(int i=0;i<3;i++) redTimeMs[i] = r;
  }

  unsigned long now = millis();
  unsigned long curDuration = (phase==0?greenTimeMs[currentLight]:(phase==1?yellowTimeMs[currentLight]:redTimeMs[currentLight]));
  if(now - lastChange > curDuration) lastChange = now;

  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "");
}

// ===== Setup =====
void setup(){
  // Initialize Serial for RFID communication
  Serial.begin(9600, SERIAL_8N1, 16);  // RX on GPIO 16, 9600 baud
  
  // Initialize RFID variables
  rfidMessage = "";
  rfidEmergency = false;

  for(int i=0;i<3;i++)
    for(int j=0;j<3;j++){
      pinMode(traffic[i][j],OUTPUT);
      digitalWrite(traffic[i][j],LOW);
    }

  pinMode(emergencyLED, OUTPUT);
  digitalWrite(emergencyLED, LOW);

  if(!display.begin(SSD1306_SWITCHCAPVCC,0x3C)){ 
    Serial.println("SSD1306 init failed");
    for(;;) delay(1000);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,20); display.print("Smart Traffic");
  display.setCursor(0,35); display.print("Controller");
  display.display();
  delay(1000);

  WiFi.begin(ssid,password);
  Serial.print("Connecting to WiFi");
  while(WiFi.status() != WL_CONNECTED){
    delay(300);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", [](){ server.send(200,"text/html",htmlPage()); });
  server.on("/traffic1", [](){ 
    emergencyMode=true; 
    emergencyActive=0; 
    rfidEmergency = false; // Manual override
    server.send(200,"text/html",htmlPage()); 
  });
  server.on("/traffic2", [](){ 
    emergencyMode=true; 
    emergencyActive=1; 
    rfidEmergency = false; // Manual override
    server.send(200,"text/html",htmlPage()); 
  });
  server.on("/traffic3", [](){ 
    emergencyMode=true; 
    emergencyActive=2; 
    rfidEmergency = false; // Manual override
    server.send(200,"text/html",htmlPage()); 
  });
  server.on("/reset", [](){ 
    emergencyMode=false; 
    emergencyActive=-1; 
    rfidEmergency = false;
    lastChange = millis(); 
    server.send(200,"text/html",htmlPage()); 
  });
  server.on("/status", [](){ server.send(200,"application/json",getStatus()); });
  server.on("/settimes", handleSetTimes);
  server.on("/setall", handleSetAll);

  server.begin();
  
  lastChange = millis();
  
  Serial.println("\nESP32 Traffic Controller Ready");
  Serial.println("RFID Tap Pattern System:");
  Serial.println("• 1 tap = North Emergency");
  Serial.println("• 2 taps = South Emergency");
  Serial.println("• 3 taps = East Emergency");
  Serial.println("• Auto-reset after 30 seconds");
}

// ===== Loop =====
void loop() {
  server.handleClient();
  
  // Check for RFID input from Arduino Nano
  handleRFIDInput();
  
  if (emergencyMode && emergencyActive >= 0){
    digitalWrite(emergencyLED, HIGH);
    emergencyTraffic();
  } else {
    digitalWrite(emergencyLED, LOW);
    autoTraffic();
  }
}