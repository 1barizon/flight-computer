import serial
import time
import pandas as pd
import math

# Config
porta = '/dev/ttyUSB0'
baud = 115200
PARACHUTE_ALTITUDE = 100.0

# FSM
IDLE, ASCENT, DESCENT, LANDED = 0, 1, 2, 3
state_names = ["IDLE", "ASCENT", "DESCENT", "LANDED"]
current_state = IDLE

prev_altp = 0.0
prev_vz = 0.0
prev_acc = 0.0
first_reading = True

# Carrega dados CSV
file = "extras/FSM_tester/dados_filtrados.csv"
df = pd.read_csv(file)

def compute_acc(ax, ay, az):
    return math.sqrt(ax*ax + ay*ay + az*az)

def process_row(millis, altp, ax, ay, az):
    global current_state, prev_altp, prev_vz, prev_acc, first_reading

    vz = 0.0
    if not first_reading:
        dt = millis - prev_millis
        if dt > 0:
            vz = (altp - prev_altp)/dt
    else:
        first_reading = False

    acc = compute_acc(ax, ay, az)
    event = None

    # FSM
    if current_state == IDLE and acc > 12.0:
        current_state = ASCENT
        event = "LIFTOFF"

    if current_state == ASCENT:
        if prev_acc > 10.0 and acc < 5.0:
            event = "BURNOUT"
        if prev_vz > 0 and vz <= 0:
            current_state = DESCENT
            event = "APOGEE"

    if current_state == DESCENT:
        if vz < -5.0 and acc < 12.0:
            event = "FREEFALL"
        if altp < PARACHUTE_ALTITUDE and vz < 0:
            event = "PARACHUTE"
        if abs(vz) < 0.5 and altp < 2.0:
            current_state = LANDED
            event = "LANDED"

    if event:
        print(f"{event} @ {millis:.2f}s | h={altp:.1f} | vz={vz:.1f} | state=State.{state_names[current_state]}")

    prev_altp = altp
    prev_vz = vz
    prev_acc = acc

# Simulação de envio para Arduino
prev_millis = df["millis"].iloc[0]

for i, row in df.iterrows():
    millis = float(row["millis"])
    altp = float(row["altp"])
    ax = float(row["ax"])
    ay = float(row["ay"])
    az = float(row["az"])

    process_row(millis, altp, ax, ay, az)
    prev_millis = millis