# dashboard.py (오류 수정 최종본: Rerun 방식 적용)
import streamlit as st
import pandas as pd
import time
import os
import csv

# 1. 페이지 설정
st.set_page_config(page_title="1인 가구 AI 관제 & 피드백", page_icon="🚨", layout="wide")

st.title("🚨 AI 생활패턴 위험 감지 모니터링")
st.markdown("---")

LOG_FILE = "realtime_log.csv"
FEEDBACK_FILE = "feedback_data.csv"

# 2. 데이터 로드 함수
def load_data():
    if not os.path.exists(LOG_FILE):
        return pd.DataFrame(columns=["timestamp", "PIR", "Bathroom", "Bed", "Dining", "Entrance", "Door", "Status", "Risk"])
    try:
        return pd.read_csv(LOG_FILE)
    except:
        return pd.DataFrame()

# 3. 피드백 저장 함수
def save_feedback(row_data, actual_label):
    fieldnames = ["PIR_living", "FSR_bathroom", "FSR_bed", "FSR_dining", "FSR_entrance", "Door_bathroom", "sim_label"]
    file_exists = os.path.exists(FEEDBACK_FILE)
    
    with open(FEEDBACK_FILE, mode='a', newline='') as f:
        writer = csv.DictWriter(f, fieldnames=fieldnames)
        if not file_exists:
            writer.writeheader()
        
        data_to_save = {
            "PIR_living": row_data["PIR"],
            "FSR_bathroom": row_data["Bathroom"],
            "FSR_bed": row_data["Bed"],
            "FSR_dining": row_data["Dining"],
            "FSR_entrance": row_data["Entrance"],
            "Door_bathroom": row_data["Door"],
            "sim_label": actual_label
        }
        writer.writerow(data_to_save)

# --- 사이드바 ---
st.sidebar.title("🔧 AI 모델 관리")
st.sidebar.info("AI가 잘못 판단했을 때, 아래 버튼으로 신고하면 AI가 더 똑똑해집니다.")

if st.sidebar.button("🚀 피드백 반영하여 재학습 시작"):
    st.sidebar.warning("재학습 프로세스를 시작합니다... (데모용)")
    time.sleep(1)
    st.sidebar.success("✅ 모델 업데이트 완료! (v1.1)")

# --- 메인 화면 로직 (while True 제거됨) ---
df = load_data()

# 컨테이너 하나 잡기
placeholder = st.container()

with placeholder:
    if df.empty:
        st.info("⏳ 데이터 수신 대기 중... (서버와 센서를 확인하세요)")
    else:
        latest = df.iloc[-1]
        
        # === [핵심] 피드백 UI ===
        col_fb1, col_fb2 = st.columns([3, 1])
        
        status = latest["Status"]
        color = "red" if status == "DANGER" else "green"
        
        col_fb1.markdown(f"### 현재 상태: <span style='color:{color}'>{status}</span>", unsafe_allow_html=True)
        col_fb1.text(f"감지 시간: {latest['timestamp']}")
        
        # 피드백 버튼 영역
        with col_fb2:
            st.write("판단이 틀렸나요?")
            # timestamp를 키로 사용하되, Rerun 구조라 중복 에러 안 남
            ts_key = str(latest["timestamp"])
            
            # 버튼 클릭 시 처리
            if st.button("네, 이건 '정상'입니다", key=f"norm_{ts_key}"):
                save_feedback(latest, 0)
                st.toast("✅ 피드백 저장: '정상' 패턴으로 학습 데이터에 추가됨")
                time.sleep(1) # 토스트 메시지 보여줄 시간 벌기
                st.rerun()    # 버튼 누르면 즉시 새로고침
                
            if st.button("네, 이건 '위험'입니다", key=f"dang_{ts_key}"):
                save_feedback(latest, 1)
                st.toast("🚨 피드백 저장: '위험' 패턴으로 학습 데이터에 추가됨")
                time.sleep(1)
                st.rerun()

        st.divider()

        # 그래프 및 통계
        kpi1, kpi2, kpi3, kpi4 = st.columns(4)
        kpi1.metric("PIR (거실)", latest["PIR"])
        kpi2.metric("Bath 압력", latest["Bathroom"])
        kpi3.metric("Bed 압력", latest["Bed"])
        kpi4.metric("위험 확률", f"{latest['Risk']*100:.1f}%")
        
        # 그래프 데이터 준비
        if not df.empty:
            chart_data = df.tail(50).copy()
            # timestamp 컬럼이 있는지 확인 후 인덱스 설정
            if "timestamp" in chart_data.columns:
                chart_data = chart_data.set_index("timestamp")
                st.line_chart(chart_data[["Bathroom", "Bed", "Dining", "Entrance"]])

# === [중요] 자동 새로고침 로직 ===
# 1초 쉬고, 자기 자신을 다시 실행(Rerun)함
time.sleep(1)
st.rerun()