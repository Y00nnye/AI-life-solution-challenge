# main.py (최종 수정본: 실시간 로그 저장 기능 포함)

from fastapi import FastAPI, HTTPException
from pydantic import BaseModel
import joblib
import pandas as pd
import datetime
import csv
import os

# 1. 모델과 설정 파일 로드
try:
    model = joblib.load("activity_model.pkl")
    scaler = joblib.load("scaler.pkl")
    feature_cols = joblib.load("feature_cols.pkl")
    print("✅ 모델 로딩 완료!")
except Exception as e:
    print(f"❌ 모델 로딩 실패: {e}")
    print("같은 폴더에 .pkl 파일들이 있는지 확인해주세요.")

app = FastAPI()

# 2. ESP32가 보낼 데이터 형식 정의
class SensorInput(BaseModel):
    PIR_living: int       
    FSR_bathroom: float   
    FSR_bed: float
    FSR_dining: float
    FSR_entrance: float
    Door_bathroom: int    

# 3. 전역 변수 (상태 저장용)
current_state = {
    "prev_pir": 0,                
    "bathroom_occupied_count": 0  
}

# 4. 로그 파일 설정
LOG_FILE = "realtime_log.csv"

# 파일이 없으면 헤더(제목) 먼저 만들기
if not os.path.exists(LOG_FILE):
    with open(LOG_FILE, mode='w', newline='') as f:
        writer = csv.writer(f)
        writer.writerow(["timestamp", "PIR", "Bathroom", "Bed", "Dining", "Entrance", "Door", "Status", "Risk"])

@app.get("/")
def read_root():
    return {"status": "Server is running", "message": "1인 가구 위험 알림 AI 서버입니다."}

@app.post("/predict")
def predict(data: SensorInput):
    global current_state
    
    # --- A. 시간 정보 추출 ---
    now = datetime.datetime.now()
    hour = now.hour
    minute = now.minute
    weekday = now.weekday()
    is_weekend = 1 if weekday >= 5 else 0

    # --- B. Feature Engineering ---
    is_occupied = (data.FSR_bathroom > 5.0) or (data.Door_bathroom == 1)
    
    if is_occupied:
        current_state["bathroom_occupied_count"] += 1 
    else:
        current_state["bathroom_occupied_count"] = 0
        
    bathroom_occupied_minutes = current_state["bathroom_occupied_count"]

    pir_edge = 1 if (data.PIR_living == 1 and current_state["prev_pir"] == 0) else 0
    current_state["prev_pir"] = data.PIR_living

    # --- C. 모델 입력 데이터 만들기 ---
    input_dict = {
        'hour': hour,
        'minute': minute,
        'weekday': weekday,
        'is_weekend': is_weekend,
        'PIR_living': data.PIR_living,
        'FSR_bathroom': data.FSR_bathroom,
        'FSR_bed': data.FSR_bed,
        'FSR_dining': data.FSR_dining,
        'FSR_entrance': data.FSR_entrance,
        'Door_bathroom': data.Door_bathroom,
        'bathroom_occupied_minutes': bathroom_occupied_minutes,
        'PIR_edge': pir_edge
    }
    
    input_df = pd.DataFrame([input_dict])
    
    for col in feature_cols:
        if col not in input_df.columns:
            input_df[col] = 0.0
            
    input_df = input_df[feature_cols]

    # --- D. 예측 및 저장 ---
    try:
        X_scaled = scaler.transform(input_df)
        prediction = model.predict(X_scaled)[0] 
        
        probability = 0.0
        if hasattr(model, "predict_proba"):
            probability = model.predict_proba(X_scaled)[0][1]

        status_str = "DANGER" if prediction == 1 else "NORMAL"

        # === [중요] CSV 파일에 기록 남기기 ===
        timestamp_str = now.strftime("%Y-%m-%d %H:%M:%S")
        with open(LOG_FILE, mode='a', newline='') as f:
            writer = csv.writer(f)
            writer.writerow([
                timestamp_str, 
                data.PIR_living, 
                data.FSR_bathroom, 
                data.FSR_bed, 
                data.FSR_dining, 
                data.FSR_entrance, 
                data.Door_bathroom,
                status_str,
                probability
            ])
        # ===================================

        result = {
            "prediction": int(prediction), 
            "risk_score": float(probability),
            "status": status_str
        }
        
        # 서버 화면에도 출력
        print(f"[{timestamp_str}] 📥 입력 -> 판정: {status_str} (위험도: {probability:.2f})")
        
        return result

    except Exception as e:
        print(f"예측 에러: {e}")
        raise HTTPException(status_code=500, detail=str(e))