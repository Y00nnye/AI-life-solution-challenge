#include <WiFi.h>
#include <HTTPClient.h>

// ================= [1. 와이파이 설정 (수정 필수)] =================
const char* ssid = "SK_FB22_2.4G";       // 예: "SK_WiFi_GIGA"
const char* password = "CKK2A@7626"; // 예: "12345678"

// ================= [2. 서버 주소 설정 (수정 필수)] =================
// 아까 확인한 내 컴퓨터 IP 주소를 넣으세요. (포트 8000 유지)
// 주의: 컴퓨터와 ESP32는 "같은 와이파이"에 연결되어 있어야 합니다.
const char* serverUrl = "http://192.168.45.231:8000/predict";

// ================= [3. 핀 번호 설정 (회로에 맞게 수정)] =================
// 친구가 연결한 핀 번호로 숫자를 바꿔야 합니다.
// (ADC 핀: 32, 33, 34, 35, 36, 39번 추천)
const int PIN_PIR_LIVING   = 5; // 거실 동작감지센서 (Digital)
const int PIN_REED_BATH    = 5; // 화장실 문 리드스위치 (Digital)

const int PIN_FSR_BATH     = 34; // 화장실 매트 압력 (Analog)
const int PIN_FSR_BED      = 34; // 침대 압력 (Analog)
const int PIN_FSR_DINING   = 34; // 식탁 압력 (Analog)
const int PIN_FSR_ENTRANCE = 34; // 현관 매트 압력 (Analog)

void setup() {
  Serial.begin(115200);

  // 핀 모드 설정
  pinMode(PIN_PIR_LIVING, INPUT);
  // 리드스위치는 회로 방식에 따라 INPUT_PULLUP 또는 INPUT 사용
  pinMode(PIN_REED_BATH, INPUT_PULLUP); 

  // 와이파이 연결
  WiFi.begin(ssid, password);
  Serial.println("\n🔥 ESP32 시작! 와이파이 연결 중...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ 와이파이 연결 성공!");
  Serial.print("내 IP 주소: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // --- 1. 센서 값 읽기 ---
  int pir = digitalRead(PIN_PIR_LIVING);
  
  // 리드스위치 (0:닫힘, 1:열림 인지 확인 필요. 반대면 !digitalRead(...) 사용)
  int door = digitalRead(PIN_REED_BATH); 

  // 압력센서 값 읽기 (0 ~ 4095)
  // 값이 너무 튀면 map() 함수 등으로 조절 가능하지만, 일단 원본 전송
  float fsr_bath = analogRead(PIN_FSR_BATH);
  float fsr_bed = analogRead(PIN_FSR_BED);
  float fsr_dining = analogRead(PIN_FSR_DINING);
  float fsr_entrance = analogRead(PIN_FSR_ENTRANCE);

  // 시리얼 모니터에 출력 (디버깅용)
  Serial.printf("센서상태 -> PIR:%d | Door:%d | Bath:%.0f | Bed:%.0f | Din:%.0f | Ent:%.0f\n", 
                pir, door, fsr_bath, fsr_bed, fsr_dining, fsr_entrance);

  // --- 2. JSON 데이터 만들기 ---
  // 파이썬 서버가 요구하는 이름과 똑같아야 함!
  String jsonPayload = "{";
  jsonPayload += "\"PIR_living\": " + String(pir) + ",";
  jsonPayload += "\"FSR_bathroom\": " + String(fsr_bath) + ",";
  jsonPayload += "\"FSR_bed\": " + String(fsr_bed) + ",";
  jsonPayload += "\"FSR_dining\": " + String(fsr_dining) + ",";
  jsonPayload += "\"FSR_entrance\": " + String(fsr_entrance) + ",";
  jsonPayload += "\"Door_bathroom\": " + String(door);
  jsonPayload += "}";

  // --- 3. 서버로 전송 (HTTP POST) ---
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");

    int httpResponseCode = http.POST(jsonPayload);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("📡 서버 응답: " + response); // 여기서 NORMAL / DANGER 확인
    } else {
      Serial.print("❌ 전송 실패 (에러코드): ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("⚠️ 와이파이 끊김");
  }

  // --- 4. 대기 (1초) ---
  delay(1000);
}