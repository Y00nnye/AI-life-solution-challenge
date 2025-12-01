#include <WiFi.h>
#include <esp_now.h>

// ====== WiFi 접속 정보 ======
const char* WIFI_SSID = "SK_FB22_2.4G";
const char* WIFI_PASS = "CKK2A@7626";

// ====== 설정 ======
const int PIR_PIN = 5;      // PIR 센서 OUT 핀
const int LED_PIN = 18;     // LED 연결 핀 (네가 실제로 쓴 핀 번호로 맞춰줘!)

const uint8_t NODE_ID = 3;

uint8_t HUB_MAC[] = { 0x68, 0xFE, 0x71, 0x80, 0x10, 0xC0 };

// ====== 전송할 데이터 구조 ======
typedef struct __attribute__((packed)) {
  uint8_t nodeId;
  uint8_t pirState;  // 0 or 1
} PirPayload;

PirPayload payload;

// ====== 콜백 ======
void OnDataSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  Serial.print("Send status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "OK" : "FAIL");
}

// ====== SETUP ======
void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(PIR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);     // LED 핀 설정

  // WiFi + 같은 핫스팟 접속
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("WiFi connected, IP = ");
  Serial.println(WiFi.localIP());
  Serial.print("My MAC: ");
  Serial.println(WiFi.macAddress());

  // ESP-NOW 초기화
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    while (true) delay(100);
  }

  esp_now_register_send_cb(OnDataSent);

  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, HUB_MAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    while (true) delay(100);
  }

  Serial.println("PIR Node started.");
}

// ====== LOOP ======
void loop() {
  int pirRaw = digitalRead(PIR_PIN);
  bool motion = (pirRaw == HIGH);

  payload.nodeId   = NODE_ID;
  payload.pirState = motion ? 1 : 0;

  esp_err_t result = esp_now_send(HUB_MAC, (uint8_t *)&payload, sizeof(payload));

  //여기서 LED도 같이 제어
  digitalWrite(LED_PIN, motion ? HIGH : LOW);

  Serial.print("PIR raw=");
  Serial.print(pirRaw);
  Serial.print(", Motion=");
  Serial.print(motion ? "1 (DETECTED)" : "0 (NO)");
  Serial.print("  /  Send: ");
  Serial.println(result == ESP_OK ? "OK" : "ERROR");

  delay(500);
}
