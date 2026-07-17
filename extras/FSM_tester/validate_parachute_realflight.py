"""
validate_parachute_realflight.py

Valida a Opcao A (deploy no apogeu) contra o voo REAL em dados_filtrados.csv
(~200m de apogeu, fornecido pelo usuario).

O que o script faz (port fiel do C++):
  1. Le o CSV real (colunas: millis,lat,lon,sat,alt,data,hora,altp,p,ax,ay,az,...)
  2. Nivela altp ao solo: altp_rel = altp - altp[0]
     (o BMP585 no firmware faz isso via base_pressure no begin())
  3. Aplica IIR alpha=0.2 nos eixos (como FlightStateMachine::update)
  4. Calcula vz por diferenciacao numerica com dt REAL (como BMP585Sensor.cpp)
  5. Roda a FSM da Opcao A (detectLiftoff/Apogee/Parachute + confirmacao)
  6. Reporta apogeu e deploy (tempo + altitude)

Objetivo: provar que o firmware (qualquer voo >= altura minima) abre o
paraquedas no apogeu, nao perto do solo, e respeita PARACHUTE_MIN_ALTITUDE.
"""

import csv
import math

# ── Thresholds (espelho de config.h Opcao A) ────────────────────────────────
LIFTOFF_ACCEL_THRESHOLD = 15.0
APOGEE_MAX_VZ = 1.0
APOGEE_AZ_THRESHOLD = -0.1
FILTER_ALPHA = 0.2
PARACHUTE_MIN_ALTITUDE = 50.0
PARACHUTE_CONFIRM_VZ = -2.0
PARACHUTE_CONFIRM_CYCLES = 3

IDLE, ASCENT, DESCENT, LANDED = 0, 1, 2, 3
NAME = {IDLE: "IDLE", ASCENT: "ASCENT", DESCENT: "DESCENT", LANDED: "LANDED"}

def total_accel(ax, ay, az):
    return math.sqrt(ax * ax + ay * ay + az * az)

def smooth(value, prev, alpha=FILTER_ALPHA):
    if prev is None:
        return value
    return alpha * value + (1.0 - alpha) * prev


class FSM:
    def __init__(self):
        self.state = IDLE
        self.liftoff = self.burnout = self.apogee = self.freefall = False
        self.parachute = False
        self.fax = self.fay = self.faz = None
        self.first = True
        self.para_confirm = 0
        self.entered = 0.0
        self.t = 0.0
        self.h = 0.0
        self.vz = 0.0

    def detect_liftoff(self):
        return total_accel(self.fax, self.fay, self.faz) > LIFTOFF_ACCEL_THRESHOLD

    def detect_apogee(self, vz):
        return (abs(vz) < APOGEE_MAX_VZ and self.faz < APOGEE_AZ_THRESHOLD)

    def detect_parachute(self, height, vz):
        return (height > PARACHUTE_MIN_ALTITUDE and vz < PARACHUTE_CONFIRM_VZ)

    def transition(self, nxt):
        self.state = nxt
        self.entered = self.t
        if nxt == DESCENT:
            self.para_confirm = 0

    def update(self, height, ax, ay, az, vz):
        self.h = height
        self.vz = vz
        if self.first:
            self.fax, self.fay, self.faz = ax, ay, az
            self.first = False
        else:
            self.fax = smooth(ax, self.fax)
            self.fay = smooth(ay, self.fay)
            self.faz = smooth(az, self.faz)
        acc = total_accel(self.fax, self.fay, self.faz)

        if self.state == IDLE:
            if not self.liftoff and self.detect_liftoff():
                self.liftoff = True
                self.transition(ASCENT)
        elif self.state == ASCENT:
            if not self.burnout and height >= 5.0 and vz > 0.5 and \
               (self.faz < -8.0 or acc < 2.0):
                self.burnout = True
            if not self.apogee and self.detect_apogee(vz):
                self.apogee = True
                self.transition(DESCENT)
        elif self.state == DESCENT:
            if not self.freefall and height >= 5.0 and vz < -5.0 and acc < 11.5:
                self.freefall = True
            if not self.parachute:
                if self.detect_parachute(height, vz):
                    self.para_confirm += 1
                    if self.para_confirm >= PARACHUTE_CONFIRM_CYCLES:
                        self.parachute = True
                else:
                    self.para_confirm = 0
        return self.parachute


def load_real_flight(path):
    rows = []
    with open(path) as f:
        r = csv.DictReader(f)
        for row in r:
            try:
                t = float(row["millis"])
                altp = float(row["altp"])
                ax = float(row["ax"])
                ay = float(row["ay"])
                az = float(row["az"])
            except (ValueError, KeyError):
                continue
            rows.append((t, altp, ax, ay, az))
    # Nivela ao solo (como base_pressure no firmware)
    if rows:
        base = rows[0][1]
        rows = [(t, altp - base, ax, ay, az) for (t, altp, ax, ay, az) in rows]
    return rows


def run(path):
    rows = load_real_flight(path)
    if not rows:
        print("Sem dados validos")
        return
    print(f"\n=== VOO REAL: {path} ===")
    print(f"  amostras={len(rows)}  t0={rows[0][0]:.2f}s  tN={rows[-1][0]:.2f}s")

    fsm = FSM()
    apogee_t = apogee_h = None
    deploy_t = deploy_h = None
    prev_t = None
    prev_alt = None

    for (t, altp, ax, ay, az) in rows:
        # vz por diferenciacao com dt real
        if prev_t is not None and (t - prev_t) > 0:
            dt = t - prev_t
            vz = (altp - prev_alt) / dt
            vz = max(min(vz, 200.0), -200.0)
        else:
            vz = 0.0
        fsm.t = t
        deployed = fsm.update(altp, ax, ay, az, vz)
        if fsm.apogee and apogee_t is None:
            apogee_t, apogee_h = t, altp
        if deployed and deploy_t is None:
            deploy_t, deploy_h = t, altp
        prev_t, prev_alt = t, altp

    print(f"  liftoff={fsm.liftoff} apogeu={fsm.apogee} parachute={fsm.parachute}")
    print(f"  APOGEU : t={apogee_t}s  h={apogee_h:.1f}m")
    print(f"  DEPLOY : t={deploy_t}s  h={deploy_h:.1f}m")
    if apogee_t and deploy_t:
        dt_ap = (deploy_t - apogee_t) * 1000.0
        print(f"  -> deploy {dt_ap:.0f} ms APOS o apogeu, a {deploy_h:.1f}m "
              f"(piso={PARACHUTE_MIN_ALTITUDE}m)")
        ok = (deploy_t > apogee_t) and (deploy_h > PARACHUTE_MIN_ALTITUDE)
        print(f"  >> {'PASS' if ok else 'FAIL'}: abre no apogeu e acima do piso")
    else:
        print("  >> Sem apogeu/deploy detectado")


if __name__ == "__main__":
    base = "/home/vinicius/Documentos/projects/flight-computer/extras/FSM_tester/"
    run(base + "dados_filtrados.csv")
    run(base + "dados_simulados.csv")
