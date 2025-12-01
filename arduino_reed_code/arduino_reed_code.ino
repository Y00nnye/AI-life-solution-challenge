#include <WiFi.h>
#include <esp_now.h>

// ====== 설정 ======
const char* WIFI_SSID = "SK_FB22_2.4G";
const char* WIFI_PASS = "CKK2A@7626";

const int REED_PIN = 5;   // 리드스위치 연결 핀

// 이 노드의 고유 ID (예: 리드스위치 노드 → 2)
const uint8_t NODE_ID = 2;

// 허브 ESP32의 MAC 주소
uint8_t HUB_MAC[] = { 0x68, 0xFE, 0x71, 0x80, 0x10, 0xC0 };

// ====== 전송할 데이터 구조 ======
typedef struct __attribute__((packed)) {
  uint8_t nodeId;      // 노드 ID
  uint8_t reedState;   // 0 or 1
} ReedPayload;

ReedPayload payload;

// ====== 콜백 (보내기 완료시) ======
void OnDataSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  Serial.print("Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

// ====== 셋업 ======
void setup() {
  Serial.begin(115200);
  delay(1000);

  // 리드스위치 입력
  // 한쪽을 GND, 한쪽을 REED_PIN에 연결하고, 내부 풀업 사용
  pinMode(REED_PIN, INPUT_PULLUP);

  // 🔹 WiFi를 스테이션 모드로 + 허브와 같은 핫스팟에 접속
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

  // ESP-NOW 초기화
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    while (true) delay(100);
  }

  // 전송 완료 콜백 등록
  esp_now_register_send_cb(OnDataSent);

  // 허브(피어) 등록
  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, HUB_MAC, 6);
  peerInfo.channel = 0;      // 0이면 현재 WiFi 채널 사용
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    while (true) delay(100);
  }

  Serial.println("Reed Switch Node started.");
}

// ====== 메인 루프 ======
void loop() {
  // 1) 리드스위치 읽기
  int raw = digitalRead(REED_PIN);

  // 배선 기준:
  //  - 리드스위치 한쪽: GND
  //  - 다른 한쪽: REED_PIN (INPUT_PULLUP)
  //  => 닫히면 LOW(0), 열리면 HIGH(1)
  bool isClosed = (raw == LOW);   // magnet 가까이 = 닫힘

  // 2) 페이로드 채우기
  payload.nodeId    = NODE_ID;
  payload.reedState = isClosed ? 1 : 0;   // 닫힘=1, 열림=0 로 정의

  // 3) 허브로 전송
  esp_err_t result = esp_now_send(HUB_MAC, (uint8_t *)&payload, sizeof(payload));

  // 4) 디버깅 출력
  Serial.print("Reed raw=");
  Serial.print(raw);
  Serial.print(", State=");
  Serial.print(isClosed ? "1(CLOSED)" : "0(OPEN)");
  Serial.print("  /  Send: ");
  Serial.println(result == ESP_OK ? "OK" : "ERROR");

  delay(500);  // 0.5초마다 전송
}
