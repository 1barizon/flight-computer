from enum import Enum, auto
import pandas as pd
import numpy as np


class State(Enum):
    IDLE = auto()
    ASCENT = auto()
    DESCENT = auto()
    LANDED = auto()


class RocketFSM:
    def __init__(self):
        self.state = State.IDLE

        # memória para detectar eventos
        self.prev_vz = 0
        self.prev_acc = 0

    def update(self, h, vz, ax, ay, az):
        acc = np.sqrt(ax**2 + ay**2 + az**2)
        event = None

        # ================= EVENTOS =================

        # 🚀 Liftoff
        if self.state == State.IDLE and acc > 12:
            event = "liftoff"
            self.state = State.ASCENT

        # 🔥 Burnout (queda brusca de aceleração)
        if self.state == State.ASCENT:
            if self.prev_acc > 10 and acc < 5:
                event = "burnout"

        # 🏔️ Apogeu (mudança de sinal do vz)
        if self.state == State.ASCENT:
            if self.prev_vz > 0 and vz <= 0:
                event = "apogee"
                self.state = State.DESCENT

        # 🪂 Freefall
        if self.state == State.DESCENT:
            if vz < -5 and acc < 12:
                event = "freefall"

        # 🎯 Paraquedas
        if self.state == State.DESCENT:
            if h < 100 and vz < 0:
                event = "parachute"

        # 🛬 Pouso
        if self.state == State.DESCENT:
            if abs(vz) < 0.5 and h < 2:
                event = "landed"
                self.state = State.LANDED

        # salva histórico
        self.prev_vz = vz
        self.prev_acc = acc

        return event, self.state
    


fsm = RocketFSM()

file = "extras/FSM_tester/dados_filtrados.csv"


df = pd.read_csv(file)

df["vz"] = 0.0
df["event"] = ""

for i in range(len(df)):
    row = df.iloc[i]

    height = float(row["altp"])
    millis = float(row["millis"])
    ax, ay, az = float(row["ax"]), float(row["ay"]), float(row["az"])

    # -------- velocidade vertical --------
    if i == 0:
        vz = 0.0
    else:
        prev = df.iloc[i - 1]
        dt = millis - float(prev["millis"])

        if dt > 0:
            vz = (height - float(prev["altp"])) / dt
        else:
            vz = 0.0

    vz = np.clip(vz, -200, 200)
    df.loc[i, "vz"] = vz

    # -------- FSM --------
    event, state = fsm.update(height, vz, ax, ay, az)

    if event:
        df.loc[i, "event"] = event
        print(f"{event.upper()} @ {millis:.2f}s | h={height:.1f} | vz={vz:.1f} | state={state}")
