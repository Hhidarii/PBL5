#include <WiFi.h>
#include <WebServer.h>
#include <PS2X_lib.h>

#define PS2_DAT 5
#define PS2_CMD 6
#define PS2_SEL 7
#define PS2_CLK 4

const char* ssid = "Dandelion";
const char* password = "hoangvy@";

PS2X ps2x;
WebServer server(80);

int error = 0;
byte type = 0;
byte vibrate = 0;
String logBuffer;

void addLog(String msg) {
  Serial.println(msg);
  logBuffer += msg + "\n";
  if (logBuffer.length() > 3000) logBuffer.remove(0, 1500);
}

void handleRoot() {
  String page = "<html><head><meta charset='utf-8'><title>PS2 Log</title>";
  page += "<script>setInterval(()=>{fetch('/log').then(r=>r.text()).then(t=>{document.getElementById('log').textContent=t});},500);</script>";
  page += "</head><body><h2>PS2 Controller Log</h2><pre id='log'></pre></body></html>";
  server.send(200, "text/html", page);
}

void handleLog() {
  server.send(200, "text/plain", logBuffer);
}

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected! IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/log", handleLog);
  server.begin();

  while (1) {
    error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, false, false);
    if (error == 0) { addLog("Controller found"); break; }
    delay(500);
  }
  type = ps2x.readType();
  addLog("Type: " + String(type));
}

void loop() {
  server.handleClient();
  if (error == 1) return;
  ps2x.read_gamepad(false, vibrate);

  if (ps2x.Button(PSB_START)) addLog("Start");
  if (ps2x.Button(PSB_SELECT)) addLog("Select");
  if (ps2x.Button(PSB_PAD_UP)) addLog("Up " + String(ps2x.Analog(PSAB_PAD_UP)));
  if (ps2x.Button(PSB_PAD_DOWN)) addLog("Down " + String(ps2x.Analog(PSAB_PAD_DOWN)));
  if (ps2x.Button(PSB_PAD_LEFT)) addLog("Left " + String(ps2x.Analog(PSAB_PAD_LEFT)));
  if (ps2x.Button(PSB_PAD_RIGHT)) addLog("Right " + String(ps2x.Analog(PSAB_PAD_RIGHT)));

  if (ps2x.NewButtonState()) {
    if (ps2x.Button(PSB_CIRCLE)) addLog("Circle");
    if (ps2x.Button(PSB_TRIANGLE)) addLog("Triangle");
    if (ps2x.Button(PSB_SQUARE)) addLog("Square");
    if (ps2x.Button(PSB_CROSS)) addLog("Cross");
  }

  int lx = ps2x.Analog(PSS_LX);
  int ly = ps2x.Analog(PSS_LY);
  int rx = ps2x.Analog(PSS_RX);
  int ry = ps2x.Analog(PSS_RY);
  addLog("LX:" + String(lx) + " LY:" + String(ly) + " RX:" + String(rx) + " RY:" + String(ry));

  delay(100);
}
