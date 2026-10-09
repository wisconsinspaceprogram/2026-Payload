import sys
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
 
g = 9.80665
baro_noise = 0.5
imu_noise = 0.05
 
 
def simulate(seed=42, dt=0.01, baro_every_n_steps=4, imu_offset=0.15,
             apogee=10000, pad_time=5.0, burn_time=8.0):
    rng = np.random.default_rng(seed)
 
    a_ = burn_time**2 / (2 * g)
    b_ = 0.5 * burn_time**2
    a_net = (-b_ + np.sqrt(b_**2 + 4 * a_ * apogee)) / (2 * a_)
    burn_end = pad_time + burn_time
    flight_time = burn_end + a_net * burn_time / g + 10.0
 
    t = np.arange(0, flight_time, dt)
    n = len(t)
    accel = np.zeros(n)
    accel[(t >= pad_time) & (t < burn_end)] = a_net
    accel[t >= burn_end] = -g
 
    vel = np.zeros(n)
    alt = np.zeros(n)
    for i in range(n - 1):
        alt[i + 1] = alt[i] + vel[i] * dt + 0.5 * accel[i] * dt**2
        vel[i + 1] = vel[i] + accel[i] * dt
 
    imu = accel + g + imu_offset + rng.normal(0, imu_noise, n)
    baro = np.full(n, np.nan)
    baro[::baro_every_n_steps] = alt[::baro_every_n_steps] + rng.normal(0, baro_noise, len(baro[::baro_every_n_steps]))
    return t, imu, baro, alt, vel
 
 
def load_csv(path):
    d = np.genfromtxt(path, delimiter=",", names=True)
    truth = d["rtk_alt"] if "rtk_alt" in d.dtype.names else None
    return d["t"], d["accel_z"], d["baro_alt"], truth, None
 
 
def run_kf(t, imu, baro):
    dt = np.median(np.diff(t))
    x = np.array([baro[np.isfinite(baro)][0], 0.0, 0.0])
    P = np.diag([1.0, 1.0, 0.5]) ** 2
    H = np.array([[1.0, 0.0, 0.0]])
    F = np.array([[1, dt, -0.5 * dt**2],
                  [0, 1,  -dt],
                  [0, 0,   1]])
    B = np.array([0.5 * dt**2, dt, 0])
    Q = np.outer(B, B) * imu_noise**2
    Q[2, 2] = 0.002**2 * dt
    R = baro_noise**2
 
    estimate = np.zeros((len(t), 3))
    for i in range(len(t)):
        x = F @ x + B * (imu[i] - g)
        P = F @ P @ F.T + Q
        if np.isfinite(baro[i]):
            S = (H @ P @ H.T)[0, 0] + R
            K = (P @ H.T)[:, 0] / S
            x = x + K * (baro[i] - x[0])
            IKH = np.eye(3) - np.outer(K, H[0])
            P = IKH @ P @ IKH.T + R * np.outer(K, K)
        estimate[i] = x
    return estimate
 
 
if len(sys.argv) > 1:
    t, imu, baro, alt, vel = load_csv(sys.argv[1])
else:
    t, imu, baro, alt, vel = simulate()
 
estimate = run_kf(t, imu, baro)
 
fig, ax = plt.subplots(3, 1, figsize=(10, 8), sharex=True)
ax[0].plot(t, baro, ".", ms=1, alpha=0.3, label="baro")
if alt is not None:
    ax[0].plot(t, alt, "k", label="truth")
ax[0].plot(t, estimate[:, 0], "r", label="Kalman")
ax[0].set_ylabel("Altitude (m)"); ax[0].legend()
if vel is not None:
    ax[1].plot(t, vel, "k")
ax[1].plot(t, estimate[:, 1], "r")
ax[1].set_ylabel("Velocity (m/s)")
ax[2].plot(t, estimate[:, 2], "r")
ax[2].set_ylabel("Accel bias (m/s^2)"); ax[2].set_xlabel("Time (s)")
 
if alt is not None:
    ok = np.isfinite(alt)
    kf_err = (estimate[:, 0] - alt)[ok]
    baro_err = (baro - alt)[ok & np.isfinite(baro)]
    print(f"Baro   RMS error: {np.sqrt(np.mean(baro_err**2)):.3f} m")
    print(f"Kalman RMS error: {np.sqrt(np.mean(kf_err**2)):.3f} m   max: {np.max(np.abs(kf_err)):.3f} m")
print(f"Final bias estimate: {estimate[-1, 2]:.4f}")
plt.tight_layout(); plt.savefig("rocket_kf_simple.png", dpi=120)









