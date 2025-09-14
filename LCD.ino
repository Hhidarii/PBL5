#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ================== WiFi & Google Sheet ==================
const char* ssid = "Dandelion";
const char* password = "hoangvy@";
const char* sheetUrl = "https://script.google.com/macros/s/AKfycbwoEs44NkR78XEY7O0UTbpNm2Hf9zV1xSlbFTKprl0F0ds0dQKSV22eQU0bimMjMzI6yw/exec";

WebServer server(80);

// ================== LCD ==================
LiquidCrystal_I2C lcd(0x27, 20, 4);  // địa chỉ I2C 0x27, LCD 20x4

// ================== Struct dữ liệu ==================
struct BenhNhan {
  String ten;
  String phong;
  String madon;
};
BenhNhan ds[50];
int tongBN = 0;
int hienTai = -1;

// ================== Hàm cập nhật LCD ==================
void capNhatLCD(const String &Pname, const String &Proom, const String &Code) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ISO.AREA SHIP-ROBOT");
  lcd.setCursor(0, 1);
  lcd.print("Name: " + Pname);
  lcd.setCursor(0, 2);
  lcd.print("Room: " + Proom);
  lcd.setCursor(0, 3);
  lcd.print("Slot: " + Code);
}

// ================== Đọc dữ liệu Google Sheet ==================
void fetchData() {
  HTTPClient http;
  Serial.println("[*] Đang tải dữ liệu từ Google Sheet...");
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  http.begin(sheetUrl);
  int httpCode = http.GET();

  if (httpCode == 200) {
    String payload = http.getString();
    Serial.println("[+] Dữ liệu JSON nhận được:");
    Serial.println(payload);

    DynamicJsonDocument doc(8192);
    DeserializationError error = deserializeJson(doc, payload);

    if (!error) {
      tongBN = 0;
      for (JsonObject row : doc.as<JsonArray>()) {
        ds[tongBN].ten = row["ten"].as<String>();
        ds[tongBN].phong = row["phong"].as<String>();
        ds[tongBN].madon = row["madon"].as<String>();

        Serial.printf("BN %d: %s | %s | %s\n", tongBN + 1,
                      ds[tongBN].ten.c_str(),
                      ds[tongBN].phong.c_str(),
                      ds[tongBN].madon.c_str());
        tongBN++;
        if (tongBN >= 50) break;
      }
      Serial.printf("[+] Tải thành công %d bệnh nhân.\n", tongBN);
    } else {
      Serial.println("[!] Lỗi parse JSON");
    }
  } else {
    Serial.printf("[!] HTTP Error: %d\n", httpCode);
  }
  http.end();
}

// ================== HTML render ==================
String hienThiThongTin() {
  if (tongBN == 0) return "Không có dữ liệu từ Google Sheet.";

  if (hienTai >= 0 && hienTai < tongBN) {
    return "Tên: " + ds[hienTai].ten + "<br>Phòng: " + ds[hienTai].phong + "<br>Mã đơn: " + ds[hienTai].madon;
  }
  if (hienTai >= tongBN) return "Đã giao xong tất cả bệnh nhân.";

  return "Ấn Bắt đầu để giao bệnh nhân đầu tiên.";
}

void handleRoot() {
  String html = "<html><head><meta charset='utf-8'><title>Robot giao đơn</title></head><body>";
  html += "<h2>Thông tin giao đơn</h2>";
  html += "<p>Địa chỉ IP ESP32: " + WiFi.localIP().toString() + "</p>";
  html += "<p>" + hienThiThongTin() + "</p>";
  html += "<a href='/start'><button>Bắt đầu</button></a> ";
  html += "<a href='/next'><button>Đã giao</button></a>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleStart() {
  if (tongBN > 0) {
    hienTai = 0;
    Serial.println("[*] Bắt đầu giao - Hiện tại BN 1");
    capNhatLCD(ds[hienTai].ten, ds[hienTai].phong, ds[hienTai].madon);
  } else {
    hienTai = -1;
    Serial.println("[!] Không có dữ liệu bệnh nhân");
    capNhatLCD("No Data", "-", "-");
  }
  handleRoot();
}

void handleNext() {
  if (hienTai >= 0 && hienTai < tongBN) {
    hienTai++;
    Serial.printf("[*] Chuyển sang BN %d\n", hienTai + 1);

    if (hienTai < tongBN) {
      capNhatLCD(ds[hienTai].ten, ds[hienTai].phong, ds[hienTai].madon);
    } else {
      capNhatLCD("Hoan tat", "-", "-");
    }
  }
  handleRoot();
}

// ================== Setup ==================
void setup() {
  Serial.begin(115200);
  Wire.begin();
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Dang ket noi WiFi...");

  WiFi.begin(ssid, password);
  Serial.print("Đang kết nối WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n[+] Kết nối thành công!");
  Serial.print("IP ESP32: ");
  Serial.println(WiFi.localIP());

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi OK: ");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP().toString());

  fetchData();

  server.on("/", handleRoot);
  server.on("/start", handleStart);
  server.on("/next", handleNext);
  server.begin();
  Serial.println("[+] WebServer đã sẵn sàng!");
}

// ================== Loop ==================
void loop() {
  server.handleClient();
}
