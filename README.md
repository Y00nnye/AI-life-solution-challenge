# 🏠 사생활 침해 없는 1인 가구 위험 감지 AI (AI Life Solution)

카메라 없이 **비식별 센서(FSR 압력센서, PIR, 리드 스위치)** 데이터만을 사용하여,  
1인 가구의 생활 패턴을 분석하고 **고독사, 실신, 감금 등 위험 상황**을 실시간으로 탐지하는 AI 시스템입니다.

## 🛠 시스템 구성 (Architecture)
1. **IoT Edge**: ESP32 + 센서 (압력, 동작, 문열림) → 데이터 수집 및 전송
2. **AI Server**: FastAPI (Python) + Random Forest 모델 → 실시간 이상 행동(Anomaly) 추론
3. **Dashboard**: Streamlit → 실시간 관제 및 피드백(Human-in-the-loop) 루프 구현

## 🚀 주요 기능
- **실시간 위험 감지**: 화장실 장기 체류, 침대/바닥 장기 무반응 감지
- **사생활 보호**: 카메라를 사용하지 않아 거부감 없는 안전 모니터링
- **AI 피드백 루프**: 오탐 발생 시 사용자가 대시보드에서 피드백을 주면 데이터가 축적되어 진화

## 📦 설치 및 실행 방법 (How to run)

### 1. 환경 설정
```bash
pip install -r requirements.txt

2. AI 서버 실행 (Backend)
Bash

uvicorn main:app --reload --host 0.0.0.0 --port 8000

3. 대시보드 실행 (Frontend)
Bash

streamlit run dashboard.py
📂 파일 설명
main.py: AI 추론 서버 (FastAPI)
dashboard.py: 실시간 관제 대시보드 (Streamlit)
esp32_sensor.ino: IoT 센서 데이터 전송 코드
activity_model.pkl: 사전 학습된 Random Forest 모델
retrain.py: 피드백 데이터 기반 재학습 코드 (Demo)
test_sender.py: 가짜 센서 데이터 생성기 (Test용)
