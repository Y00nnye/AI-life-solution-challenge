#include <WiFi.h>
#include <esp_now.h>

// ====== WiFi 설정 ======
const char* WIFI_SSID = "SK_FB22_2.4G";
const char* WIFI_PASS = "CKK2A@7626";

// ====== 압력센서 핀 ======
const int FSR_PIN = 34;     
const int FSR_THRESHOLD = 3000;   // ★ 기준값 설정!

const uint8_t NODE_ID = 1;

// 허브 MAC 주소
uint8_t HUB_MAC[] = { 0x68, 0xFE, 0x71, 0x80, 0x10, 0xC0 };

// ====== 전송할 데이터 구조 ======
typedef struct __attribute__((packed)) {
  uint8_t  nodeId;
  uint8_t  fsrState;   // ★ 0 또는 1만 보낼 거니까 uint8_t 로 변경
} FsrPayload;

FsrPayload payload;

void OnDataSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  Serial.print("Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(FSR_PIN, INPUT);

  // ====== WiFi ======
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(200);
  }
  Serial.println();
  Serial.print("WiFi connected, IP = ");
  Serial.println(WiFi.localIP());
  Serial.print("My MAC: ");
  Serial.println(WiFi.macAddress());

  // ====== ESP-NOW ======
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    while(true) delay(100);
  }

  esp_now_register_send_cb(OnDataSent);

  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, HUB_MAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    while(true) delay(100);
  }

  Serial.println("FSR Node started.");
}

void loop() {
  // 1) RAW 값 읽기
  int fsrValue = analogRead(FSR_PIN);

  // 2) 기준에 따라 0/1 생성
  uint8_t state = (fsrValue >= FSR_THRESHOLD) ? 1 : 0;

  // 3) 페이로드 채우기
  payload.nodeId = NODE_ID;
  payload.fsrState = state;

  // 4) 전송
  esp_err_t result = esp_now_send(HUB_MAC, (uint8_t *)&payload, sizeof(payload));

  // 5) 디버깅 출력
  Serial.print("FSR raw = ");
  Serial.print(fsrValue);
  Serial.print("  ->  State = ");
  Serial.print(state);
  Serial.print("  / Send: ");
  Serial.println(result == ESP_OK ? "OK" : "ERROR");

  delay(300);
}
