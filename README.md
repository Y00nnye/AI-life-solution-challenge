# 🏠 SafeGuard AI  
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
- 7일 센서 로그 기반  
- 일상 패턴 + 위험 시나리오 포함  

### 주요 Feature  
`weekday, hour, is_weekend, FSR_bed, PIR_living, Door_bathroom…`  

### 📊 시각화  
- Feature Importance  
- Confusion Matrix  
- ROC / PR Curve  

**해석:**  
> “언제, 어디에 오래 머무르는가”가 위험 판단의 핵심  

## (3) 실행 파일 구성 (.py)

AI 서버는 `main.py`(FastAPI) 기준으로 동작하며, 학습된 모델 파일과 같은 폴더에서 실행됩니다.

- `main.py`  
  FastAPI 기반 실시간 추론 서버  
  (`POST /predict` 엔드포인트에서 센서 상태 JSON을 입력받아 예측 수행)

- `activity_model.pkl`  
  RandomForest 학습 모델 (AI_life_pattern_7days에서 저장한 모델)

- `scaler.pkl` (사용했다면)  
  학습 시 사용한 StandardScaler 등 전처리 객체

- `dashboard.py`  
  `realtime_log.csv`를 1초 단위로 읽어 실시간 상태를 보여주는 Streamlit 대시보드

- `requirements.txt`  
  FastAPI, scikit-learn, pandas, streamlit 등 프로젝트에 필요한 파이썬 패키지 목록

- AI 서버 진행
uvicorn main:app --host 0.0.0.0 --port 8000


---

## (4) 실행 방법 (로컬 테스트 기준)

📦 설치 및 실행 가이드 (How to Run)
본 시스템은 Python 3.8+ 환경에서 구동됩니다. AI 서버와 대시보드를 실행하기 위해 아래 절차를 따라주세요.

1. 환경 설정 (Installation)
프로젝트를 클론(Clone)하고 필수 라이브러리를 설치합니다.

Bash

# 1. 저장소 클론
git clone https://github.com/사용자ID/SafeGuard-AI.git
cd SafeGuard-AI/server

# 2. 필수 패키지 설치
pip install -r requirements.txt
(requirements.txt 주요 패키지: fastapi, uvicorn, streamlit, scikit-learn, pandas)

2. AI 서버 실행 (Backend)
ESP32로부터 센서 데이터를 수신하고 AI 모델(activity_model.pkl)을 통해 위험 여부를 판단하는 FastAPI 서버를 실행합니다.

Bash

# server 폴더 내에서 실행
uvicorn main:app --reload --host 0.0.0.0 --port 8000
실행 확인: 터미널에 Application startup complete 메시지가 뜨면 성공입니다.

접속 주소: http://localhost:8000 (API Docs: http://localhost:8000/docs)

3. 관제 대시보드 실행 (Frontend)
실시간 센서 데이터와 위험 감지 현황을 시각화하는 Streamlit 대시보드를 실행합니다. (새 터미널 창 사용)

Bash

# server 폴더 내에서 실행
streamlit run dashboard.py
실행 시 브라우저가 자동으로 열리며 대시보드 화면(http://localhost:8501)이 표시됩니다.

4. (선택) 시뮬레이션 테스트
하드웨어(ESP32)가 없는 환경에서도 작동을 검증할 수 있도록 가상 데이터 전송기를 제공합니다.

Bash

# 가짜 센서 데이터 생성 및 전송
python test_sender.py
실행하면 정상(Normal)과 위험(Danger) 시나리오 데이터가 번갈아 서버로 전송되며, 대시보드에서 실시간 변화를 확인할 수 있습니다.

💡 팁: 하드웨어 연결 시 주의사항
IP 주소 설정: ESP32 코드(esp32_sensor.ino) 내의 serverUrl을 서버 컴퓨터의 IP 주소(예: http://192.168.0.x:8000/predict)로 수정해야 합니다.

네트워크: 서버(PC)와 ESP32는 동일한 Wi-Fi(또는 핫스팟)에 연결되어 있어야 통신이 가능합니다
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



