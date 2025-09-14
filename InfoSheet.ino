#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <ArduinoJson.h>

const char* ssid = "Dandelion";
const char* password = "hoangvy@";

// Link Web App đã publish (Deploy as web app → Anyone with link)
const char* sheetUrl = "https://script.google.com/macros/s/AKfycbwoEs44NkR78XEY7O0UTbpNm2Hf9zV1xSlbFTKprl0F0ds0dQKSV22eQU0bimMjMzI6yw/exec";

WebServer server(80);

struct BenhNhan {
  String ten;
  String phong;
  String madon;
};

BenhNhan ds[50];   // tối đa 50 bệnh nhân
int tongBN = 0;
int hienTai = -1;

// ================== Đọc dữ liệu Google Sheet ==================
void fetchData() {
  HTTPClient http;
  Serial.println("[*] Đang tải dữ liệu từ Google Sheet...");

  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);  // cho phép theo dõi redirect
  http.begin(sheetUrl);
  int httpCode = http.GET();

  if (httpCode == 200) {
    String payload = http.getString();
    Serial.println("[+] Dữ liệu JSON nhận được:");
    Serial.println(payload);

    DynamicJsonDocument doc(8192);  // tăng dung lượng bộ nhớ JSON
    DeserializationError error = deserializeJson(doc, payload);

    if (!error) {
      tongBN = 0;
      for (JsonObject row : doc.as<JsonArray>()) {
        ds[tongBN].ten = row["ten"].as<String>();
        ds[tongBN].phong = row["phong"].as<String>();
        ds[tongBN].madon = row["madon"].as<String>();

        Serial.printf("BN %d: %s | %s | %s\n", tongBN+1,
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
  } else {
    hienTai = -1;
    Serial.println("[!] Không có dữ liệu bệnh nhân");
  }
  handleRoot();
}

void handleNext() {
  if (hienTai >= 0 && hienTai < tongBN) {
    hienTai++;
    Serial.printf("[*] Chuyển sang BN %d\n", hienTai+1);
  }
  handleRoot();
}

// ================== Setup ==================
void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  Serial.print("Đang kết nối WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n[+] Kết nối thành công!");
  Serial.print("IP ESP32: ");
  Serial.println(WiFi.localIP());

  fetchData();  // tải dữ liệu Google Sheet

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
