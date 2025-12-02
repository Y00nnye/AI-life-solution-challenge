import requests
import time
import random

# 내 서버 주소 (같은 컴퓨터니까 localhost)
url = "http://127.0.0.1:8000/predict"

print("📡 가짜 센서 데이터 전송을 시작합니다! (Ctrl+C로 중단)")

try:
    while True:
        # 1. 가짜 데이터 생성 (랜덤)
        # 평소에는 침대(bed)에 누워있는 상황 연출
        fake_data = {
            "PIR_living": 0,
            "FSR_bathroom": 0.0,
            "FSR_bed": random.uniform(100.0, 150.0), # 100~150 사이 랜덤
            "FSR_dining": 0.0,
            "FSR_entrance": 0.0,
            "Door_bathroom": 0
        }
        
        # 가끔(10% 확률) 화장실에 감 (위험 상황 연출용)
        if random.random() < 0.1:
            fake_data["FSR_bed"] = 0.0
            fake_data["FSR_bathroom"] = 200.0 # 화장실 압력 높음
            fake_data["Door_bathroom"] = 1    # 문 닫힘(혹은 열림)

        # 2. 서버로 전송
        try:
            response = requests.post(url, json=fake_data)
            print(f"보냄: {fake_data} -> 응답: {response.json()['status']}")
        except Exception as e:
            print("서버 연결 실패 (서버 켜져 있나요?)")

        # 3. 1초 대기
        time.sleep(1)

except KeyboardInterrupt:
    print("\n전송을 멈춥니다.")