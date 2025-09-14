#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

// ====== Cấu hình mạng Wi-Fi của bạn ======
const char* ssid     = "Dandelion";    // <-- Đổi thành SSID router
const char* password = "hoangvy@";       // <-- Đổi thành mật khẩu router

// ====== Servo ======
Servo servo1, servo2, servo3, servo4;
const int SERVO1_PIN = 19;
const int SERVO2_PIN = 18;
const int SERVO3_PIN = 5;
const int SERVO4_PIN = 17;

WebServer server(80);

// Điều khiển servo
void controlServo(int id, int angle) {
  switch (id) {
    case 1: servo1.write(angle); break;
    case 2: servo2.write(angle); break;
    case 3: servo3.write(angle); break;
    case 4: servo4.write(angle); break;
  }
}

// Trang HTML
String htmlPage() {
  String page = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>ESP32 Servo Control</title>";
  page += "<style>button{padding:10px;margin:5px;font-size:18px;}</style></head><body>";
  page += "<h2>Điều khiển Servo (ESP32 Local IP)</h2>";
  for (int i = 1; i <= 4; i++) {
    page += "<h3>Servo " + String(i) + "</h3>";
    page += "<button onclick=\"location.href='/on" + String(i) + "'\">ON</button>";
    page += "<button onclick=\"location.href='/off" + String(i) + "'\">OFF</button>";
  }
  page += "</body></html>";
  return page;
}

void handleRoot() { server.send(200, "text/html", htmlPage()); }

void setup() {
  Serial.begin(115200);

  // Khởi tạo servo
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  servo1.setPeriodHertz(50);
  servo2.setPeriodHertz(50);
  servo3.setPeriodHertz(50);
  servo4.setPeriodHertz(50);

  servo1.attach(SERVO1_PIN, 500, 2500);
  servo2.attach(SERVO2_PIN, 500, 2500);
  servo3.attach(SERVO3_PIN, 500, 2500);
  servo4.attach(SERVO4_PIN, 500, 2500);

  // ====== Kết nối vào Wi-Fi router ======
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Dang ket noi Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nDa ket noi!");
  Serial.print("ESP32 IP: ");
  Serial.println(WiFi.localIP());

  // Đăng ký URL
  server.on("/", handleRoot);
  // Servo 1
  server.on("/on1", []() { controlServo(1, 120); server.sendHeader("Location", "/"); server.send(303); });
  server.on("/off1", []() { controlServo(1, 0);   server.sendHeader("Location", "/"); server.send(303); });
  // Servo 2
  server.on("/on2", []() { controlServo(2, 120); server.sendHeader("Location", "/"); server.send(303); });
  server.on("/off2", []() { controlServo(2, 0);   server.sendHeader("Location", "/"); server.send(303); });
  // Servo 3
  server.on("/on3", []() { controlServo(3, 120); server.sendHeader("Location", "/"); server.send(303); });
  server.on("/off3", []() { controlServo(3, 0);   server.sendHeader("Location", "/"); server.send(303); });
  // Servo 4
  server.on("/on4", []() { controlServo(4, 120); server.sendHeader("Location", "/"); server.send(303); });
  server.on("/off4", []() { controlServo(4, 0);   server.sendHeader("Location", "/"); server.send(303); });

  server.begin();
  Serial.println("Web server da khoi dong.");
}

void loop() {
  server.handleClient();
}
