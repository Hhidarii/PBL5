// --- Cấu hình chân ---
const int encoderA = 36;   // Encoder pha A
const int encoderB = 39;   // Encoder pha B

const int motorINA = 32;   // Điều khiển xuôi
const int motorINB = 33;   // Điều khiển ngượcz
// EN được nối sẵn với 5V

volatile long encoderValue = 0;

void IRAM_ATTR updateEncoder() {
  // Khi có cạnh lên của pha A
  // Xác định chiều bằng pha B
  if (digitalRead(encoderA) > digitalRead(encoderB)) {
    encoderValue++;
  } else {
    encoderValue--;
  }
}

void setup() {
  Serial.begin(115200);
  
  pinMode(encoderA, INPUT_PULLUP);
  pinMode(encoderB, INPUT_PULLUP);
  
  pinMode(motorINA, OUTPUT);
  pinMode(motorINB, OUTPUT);
  
  // Gắn interrupt cho pha A, khi có rising edge
  attachInterrupt(digitalPinToInterrupt(encoderA), updateEncoder, RISING);
  
  Serial.println("Test motor & encoder JGA25");
}

void loop() {
  // Quay thuận
  digitalWrite(motorINA, HIGH);
  digitalWrite(motorINB, LOW);
  delay(3000);
  
  // Dừng
  digitalWrite(motorINA, LOW);
  digitalWrite(motorINB, LOW);
  delay(1000);
  
  // Quay ngược
  digitalWrite(motorINA, LOW);
  digitalWrite(motorINB, HIGH);
  delay(3000);
  
  // Dừng
  digitalWrite(motorINA, LOW);
  digitalWrite(motorINB, LOW);
  delay(1000);
  
  // In encoder mỗi giây
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint >= 1000) {
    lastPrint = millis();
    noInterrupts();
    long val = encoderValue;
    interrupts();
    Serial.print("Encoder: ");
    Serial.println(val);
  }
}
