# 🏠 Safeguard AI  
사생활 침해 없는 1인 가구 위험 감지 시스템  

카메라 없이도 1인 가구의 생활 패턴을 분석해  
**실신·고독사·욕실 사고 등 위험 상황을 실시간 감지하는 AI 기반 안전 모니터링 시스템**입니다.  

**센서:** FSR 압력센서 · PIR 모션센서 · Reed 문열림 센서  
**하드웨어:** ESP32 센서 노드 + ESP32 허브  
**소프트웨어:** FastAPI 서버 + RandomForest 모델 + Streamlit 대시보드 + Blynk 앱  
**UX:** Figma 기반 상용 서비스 디자인(SafeGuard AI)  

---

# 1. 🎯 문제 정의 및 목표  

## 📌 사회적 배경  
- 1인 가구 증가 → **고독사, 욕실 사고, 장기 방치 문제 급증**  
- 기존 CCTV 방식은 **사생활 침해·설치 거부감·비용 문제**로 확산 어려움  

## 🎯 프로젝트 목표  
- **비식별 센서(0/1 또는 압력만)** 기반 위험 탐지  
- **저비용 ESP32 + 오픈소스**로 구현  
- **서비스 확장 가능한 UX·서비스 구조 제시**

---

# 2. 🏗 전체 시스템 구조  

## (1) 하드웨어 — IoT 센서 노드 & 허브  

### 🔹 ESP32 센서 노드 (여러 개)  
- FSR 압력센서: 침대 / 욕실 / 현관 / 식탁 의자  
- PIR 모션센서: 거실 움직임  
- Reed Switch: 욕실·현관 문 열림  
- ESP-NOW 기반 초저지연 전송  
- 모든 데이터 **0/1 또는 압력값 → 비식별**  

### 🔹 ESP32 허브 (1개)  
- 모든 센서 패킷 수신 → 상태 변수화  
- `PIR_living`, `FSR_bathroom`, `FSR_bed`, `Door_bathroom` 등 구조화  
- FastAPI **/predict** 로 JSON POST  
- 동시에 **Blynk 앱에 실시간 표시**  

### 📌 핵심 구조  
센서 노드 → ESP-NOW → 허브 ESP32 → FastAPI → AI 모델 → Dashboard/UI  

---

# 3. 🤖 AI 서버 및 모델  

## (1) FastAPI 기반 AI 서버  
- 엔드포인트: `POST /predict`  
- 입력(JSON): ESP32 허브 → FastAPI  
- 처리 과정  
  - Feature 변환(요일/시간/주말/점유시간)  
  - 학습 당시 scaler + feature 동일하게 적용  
  - RandomForest(`activity_model.pkl`) 로 예측  
  - 결과: prediction, risk_score, status  
  - `realtime_log.csv` 자동 기록 → Streamlit에서 사용  

## (2) 학습 데이터 & 성능  

### 📘 학습 데이터  
- 3~7일 센서 로그 기반  
- 일상 패턴 + 위험 시나리오 포함  

### 주요 Feature  
`weekday, hour, is_weekend, FSR_bed, PIR_living, Door_bathroom…`  

### 📊 시각화  
- Feature Importance  
- Confusion Matrix  
- ROC / PR Curve  

**해석:**  
> “언제, 어디에 오래 머무르는가”가 위험 판단의 핵심  

---

# 4. 📡 실시간 모니터링 UI  
_(Streamlit Dashboard + Blynk 앱)_  

## (1) 🖥 Streamlit Dashboard — 관제/관리자용  
- 1초마다 realtime_log.csv 반영  
- 현재 상태(NORMAL/DANGER) 표시  
- FSR/PIR 그래프 시계열  
- “정상입니다 / 위험입니다” 버튼 → **피드백 기반 모델 개선**  

## (2) 📱 Blynk 앱 — 기술 구현 UI  
- 센서 상태를 원형 인디케이터로 시각화  
- ESP32 허브와 직접 연동  
- 위험 발생 시 즉시 사용자 선택(정상/위험) 가능  
- 실제 스마트폰 기반 **기능 데모**  

---

# 5. 🎨 Figma 기반 상용 서비스 디자인  
_(SafeGuard AI — 사용자 / 보호자 앱)_  

## (1) 시작 화면  
- SafeGuard AI 로고  
- “프라이버시 우선 안전 모니터링”  
- 사용자 모드 / 보호자 모드 선택  

## (2) 사용자 앱 (독거 가구 당사자)  
- 화면 전체 대형 버튼  
  - ✔ “괜찮아요”  
  - ✔ “위험해요”  
- “위험해요” 클릭 시  
  - 보호자 전화  
  - 119 비상 연결  
- 고령자 친화적 UI (큰 글씨·높은 대비)  

## (3) 보호자 앱  
- AI 위험 감지 → 알림 푸시  
- 센서별 상태 카드  
- 최근 패턴 변화 그래프  
- 사용자 “위험해요” 신고 기록  
- 추후 확장: 주간 리포트, 활동 분석  
  
> “Streamlit/Blynk을 넘어 상용 서비스 수준의 UX까지 포함해 실제 돌봄 서비스로 확장 가능한 방향을 설계  

---

# 6. 🌟 프로젝트 의의  

### ✔ 프라이버시 보호  
카메라 없이 위험 감지 가능 → 거부감↓ 수용성↑  

### ✔ 엔드투엔드 구현  
센서 → ESP-NOW → 허브 ESP32 → FastAPI → AI → Dashboard/UI  
**모든 요소가 실제 동작하는 완성형 프로토타입**  

### ✔ 확장성  
Figma UX 포함 → 상용 서비스까지 발전 가능  



