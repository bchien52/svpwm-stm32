<h1 align="center">SVPWM Implementation on STM32F103</h1>

<div align="center">
  <a href="https://www.youtube.com/watch?v=xKNJFl4AInI" target="_blank">
    <img src="https://img.youtube.com/vi/xKNJFl4AInI/maxresdefault.jpg" alt="Thumbnail" width="600" />
  </a>
  <p><em>You can click through the thumbnail or the link below to watch the demo:</em></p>
  <p><em><a href="https://www.youtube.com/watch?v=xKNJFl4AInI">https://www.youtube.com/watch?v=xKNJFl4AInI</a></em></p>
</div>

## Table of Contents

1. [Project Overview](#1-project-overview)
2. [Circuit Design](#2-circuit-design)
3. [Project Structure](#3-project-structure)
4. [Algorithm Pipeline (9 Steps)](#4-algorithm-pipeline-9-steps)
5. [High-Level Flow](#5-high-level-flow)

## 1. Project Overview

This is a **PBL4 (Project-Based Learning 4)** project. It implements a three-phase inverter control system on an **STM32F103** microcontroller, using the **Space Vector PWM (SVPWM)** algorithm to generate the six gate signals for a three-phase bridge.

- **MCU:** STM32F103, TIM1 configured in center-aligned PWM mode with complementary outputs and hardware dead-time insertion.
- **Gate driver:** IR2112 (high-side/low-side driver), one IR2112 per half-bridge, three in total - each pair drives one phase leg.
- **Power MOSFET:** IRFZ44N, 6 total (2 per phase leg: 1 high-side, 1 low-side), forming a standard three-phase inverter topology.
- **Power stage:** 3 half-bridges, 6 MOSFETs total.
- **Control algorithm:** SVPWM, computed in software (`svpwm.c`/`svpwm.h`) and updated every timer update event (~100 µs), open-loop (fixed $V_d$, $V_q$, no current or position feedback).
- **Goal:** generate a rotating three-phase voltage vector at a target frequency/amplitude, using SVPWM instead of standard sinusoidal PWM for better DC-bus utilization.

## 2. Circuit Design

| Description | Image                             |
|-------------|-----------------------------------|
| Schematic   | ![Schematic](img/schematic.png)   |
| PCB layout  | ![PCB Layout](img/pcb_layout.png) |
| 3D render   | **Top:**<br>![3D Top](img/3d_top.png)<br>**Bottom:**<br>![3D Bottom](img/3d_bottom.png) |

**Disclaimer:** The above drawings and designs are for reference purposes only. During actual construction and assembly, parameters, components, or circuit layouts may be adjusted as necessary.

## 3. Project Structure

```
SVPWM/
├── img/                    # Image for README
├── delay.c / delay.h
├── gpio_driver.c / .h
├── main.c                  # GPIO/TIM1 configuration, main
├── svpwm.c / svpwm.h       # SVPWM algorithm implementation
├── SVPWM.uvprojx           # Main Keil uVision Project File
├── SVPWM.uvoptx            # Target and Debugger settings
└── README.md               # Project documentation

```

## 4. Algorithm Pipeline (9 Steps)

### Step 1 - Inverse Park: $(V_d, V_q) \rightarrow (V_\alpha, V_\beta)$

$$
\begin{aligned}
V_\alpha &= V_d \cos\theta - V_q \sin\theta \\
V_\beta &= V_d \sin\theta + V_q \cos\theta
\end{aligned}
$$

This rotates the voltage vector from the synchronously-rotating $d$-$q$ frame into the stationary $\alpha$-$\beta$ frame, using the instantaneous electrical angle $\theta$.

### Step 2 - Modified Inverse Clarke: $(V_\alpha, V_\beta) \rightarrow$ three reference axes

$$
\begin{aligned}
V_{ref1} &= V_\beta \\
V_{ref2} &= -\tfrac{1}{2}V_\beta + \tfrac{\sqrt{3}}{2}V_\alpha \\
V_{ref3} &= -\tfrac{1}{2}V_\beta - \tfrac{\sqrt{3}}{2}V_\alpha
\end{aligned}
$$

This projects $(V_\alpha, V_\beta)$ onto three axes spaced $120°$ apart, without the $2/3$ scaling of the classical Clarke transform - it is only used to determine sign/relative magnitude for sector detection, not the true instantaneous phase voltages.

### Step 3 - Sector identification

Define the sign indicators

$$
a = \mathbb{1}[V_{ref1} > 0], \qquad
b = \mathbb{1}[V_{ref2} > 0], \qquad
c = \mathbb{1}[V_{ref3} > 0]
$$

$$
N = a + 2b + 4c, \qquad N \in \{1,2,3,4,5,6\}
$$

$N$ is mapped to the sector index $n \in \{1,\dots,6\}$ via the standard SVPWM lookup:

| $N$ | 1 | 3 | 2 | 6 | 4 | 5 |
|---|---|---|---|---|---|---|
| Sector $n$ | 1 (S3) | 2 (S1) | 3 (S5) | 4 (S4) | 5 (S6) | 6 (S2) |

### Step 4 - Intermediate normalized time quantities

$$
K = \frac{\sqrt{3}}{V_{dc}}
$$

$$
T_X = K \, V_{ref1}, \qquad
T_Y = -K \, V_{ref3}, \qquad
T_Z = -K \, V_{ref2}
$$

(If $V_{dc} \to 0$, all three are defined as $0$ to avoid division by zero.)

### Step 5 - Active-vector durations $T_1$, $T_2$ per sector

$$
(T_1, T_2) =
\begin{cases}
(-T_Z,\ T_X) & n = 1 \\
(T_Z,\ T_Y) & n = 2 \\
(T_X,\ -T_Y) & n = 3 \\
(-T_X,\ T_Z) & n = 4 \\
(-T_Y,\ -T_Z) & n = 5 \\
(T_Y,\ -T_X) & n = 6
\end{cases}
$$

$T_1$ is the duration of the active vector preceding the sector boundary, $T_2$ the one following it.

### Step 6 - Saturation and zero-vector duration $T_0$

Let $S = T_1 + T_2$. Then:

$$
(T_1, T_2, T_0) =
\begin{cases}
\left(\dfrac{T_1}{S},\ \dfrac{T_2}{S},\ 0\right) & \text{if } S > 1 \quad \text{(over-modulation)} \\[2mm]
\left(T_1,\ T_2,\ 1 - T_1 - T_2\right) & \text{if } S \le 1
\end{cases}
$$

This guarantees $T_1 + T_2 + T_0 = 1$, i.e. the durations always sum to one modulation period.

### Step 7 - Switching time instants (symmetric staircase)

$$
T_{aon} = \frac{T_0}{2}, \qquad
T_{bon} = T_{aon} + T_1, \qquad
T_{con} = T_{bon} + T_2
$$

These three cumulative instants are shared by all sectors before being assigned to a specific phase.

### Step 8 - Phase assignment

Let $(C_1, C_2, C_3)$ be the normalized duty cycles of phases A, B, C:

$$
(C_1, C_2, C_3) =
\begin{cases}
(T_{aon}, T_{bon}, T_{con}) & n = 1 \\
(T_{bon}, T_{aon}, T_{con}) & n = 2 \\
(T_{con}, T_{aon}, T_{bon}) & n = 3 \\
(T_{con}, T_{bon}, T_{aon}) & n = 4 \\
(T_{bon}, T_{con}, T_{aon}) & n = 5 \\
(T_{aon}, T_{con}, T_{bon}) & n = 6
\end{cases}
$$

### Step 9 - Conversion to timer compare values (CCR)

$$
CCR_1 = \left\lfloor C_1 \cdot \mathrm{ARR} \right\rfloor, \qquad
CCR_2 = \left\lfloor C_2 \cdot \mathrm{ARR} \right\rfloor, \qquad
CCR_3 = \left\lfloor C_3 \cdot \mathrm{ARR} \right\rfloor
$$

subject to the saturation constraint

$$
CCR_k = \min(CCR_k,\ \mathrm{ARR}), \qquad k = 1,2,3
$$

These three values are loaded into `TIM1->CCR1/2/3` on the next update event (preload + center-aligned mode 1 ensures a glitch-free duty update).

## 5. High-Level Flow

$$
(V_d, V_q, \theta)
\xrightarrow{\text{Inverse Park}}
(V_\alpha, V_\beta)
\xrightarrow{\text{Mod. Inv. Clarke}}
(V_{ref1}, V_{ref2}, V_{ref3})
\xrightarrow{\text{sign} \to n}
(T_X, T_Y, T_Z)
\xrightarrow{n}
(T_1, T_2)
\xrightarrow{\text{saturate}}
(T_{aon}, T_{bon}, T_{con})
\xrightarrow{\text{permute by } n}
(C_1, C_2, C_3)
\xrightarrow{\times \mathrm{ARR}}
(CCR_1, CCR_2, CCR_3)
$$