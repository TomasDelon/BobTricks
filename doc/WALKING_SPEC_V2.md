# WALKING_SPEC_V2 — Especificación de locomotion CM-first

**Estado:** spec formal acordada. Base de la implementación en el worktree `walking-redesign`.
**No describe el estado actual del código. Es la referencia de lo que hay que implementar.**

---

## 1. Axiomas (no negociables)

| # | Axioma |
|---|---|
| A1 | El CM es la variable de estado primaria. Los pies, la pelvis y las piernas se reconstruyen a partir del CM. |
| A2 | El trigger de paso usa el estado dinámico del CM (XCoM), no distancias geométricas. |
| A3 | Soporte = contacto geométrico AND pierna IK-factible. Un pie en el suelo con pierna extendida más allá del límite no cuenta como soporte. |
| A4 | Un semi-ciclo no se interrumpe. Una vez que step_activo = true, permanece true hasta heel-strike. |
| A5 | El jugador controla el CM mediante move_force aplicada directamente al CM. |
| A6 | La fuerza IP y move_force son aditivas. |
| A7 | Ninguna fuerza se deriva de la posición de los pies hacia el CM. La pose se reconstruye a partir del CM. |
| A8 | Altura mínima del CM: 1L sobre el terreno (máxima posición agachada). |

---

## 2. Notación y cantidades derivadas

```
H    = body_height_m                          altura total del personaje
L    = H / 5                                  longitud de un segmento de pierna

h_nominal = computeNominalY(L, d_pref, cm_pelvis_ratio)
           = sqrt((2L)² − (d_pref·L/2)²) + cm_pelvis_ratio·L
           — altura del CM sobre el terreno en postura nominal
           — siempre positivo, independiente del terreno

h_real    = clamp(y_CM − terrain_h(x_foot_stance), L, h_nominal)
           — altura física real del CM sobre el pie de apoyo
           — clampeada entre L (máx. agachado) y h_nominal (de pie)
           — NUNCA usar como denominador sin este clamp

ω₀   = sqrt(g / h_real)                      frecuencia natural del péndulo invertido
ξ    = x_CM + ẋ_CM / ω₀                      XCoM (extrapolated center of mass)
φ(x) = slope_at(x)                           ángulo de pendiente del terreno en x
```

---

## 3. Dinámica vertical del CM

### 3.1 Ecuación

```
ÿ_CM = k_y · (y_target − y_CM) − c_y · ẏ_CM − g
```

### 3.2 Target de altura

```
y_target = terrain_h(x_CM) + h_nominal · cos(φ(x_CM))
```

- En terreno plano (φ = 0): y_target = terrain_h + h_nominal
- En pendiente: y_target se reduce proporcionalmente al coseno de la pendiente
- `h_nominal` es siempre positivo — y_target siempre está por encima del terreno

### 3.3 Clamp duro

```
si y_CM < terrain_h(x_CM) + L:
    y_CM = terrain_h(x_CM) + L
    si ẏ_CM < 0: ẏ_CM = 0
```

El CM nunca puede estar a menos de L del suelo.

---

## 4. Dinámica horizontal del CM

### 4.1 Ecuación general

```
ẍ_CM = (g / h_real) · (x_CM − x_CoP) + input_x − friction · ẋ_CM
```

Donde:
- `g / h_real`: fuerza del péndulo invertido (siempre positiva en denominador)
- `input_x`: ±move_force si tecla pulsada, 0 si no
- `friction · ẋ_CM`: amortiguamiento de suelo (actúa siempre en terreno)

### 4.2 Definición de x_CoP

| Modo | x_CoP |
|---|---|
| Standing (!step_activo) | x_CM — la fuerza IP es cero, el CM no se acelera solo |
| Walking (step_activo) | x_foot_stance — pie plantado activo (el que no oscila) |

**Consecuencia**: en standing sin tecla pulsada, `ẍ_CM = −friction · ẋ_CM`. El CM se detiene por fricción. Estable en plano.

### 4.3 Estabilidad en pendiente (modelo de tobillo abstracto)

En standing sobre pendiente, la gravedad tiene componente horizontal. Añadir:

```
// Solo durante standing mode (!step_activo y |ẋ_CM| < eps_v)
ẍ_CM += k_ankle · (x_support_center − x_CM)
```

Donde `x_support_center` = promedio de x de los pies plantados.
`k_ankle ≈ 0.5 · g / h_nominal`

Esto es el modelo abstracto del torque de tobillo — no es una fuerza desde el pie al CM, es parte de la dinámica del CM.

---

## 5. Modelo de soporte efectivo

### 5.1 Peso de soporte σ_i

Para cada pie i:

```
d_i  = || pelvis − foot_i ||          distancia euclidiana pelvis → pie
σ_i  = contact_i · clamp((2L − d_i) / δ_support, 0, 1)
```

Donde:
- `contact_i`: 1 si el pie está geométricamente en contacto con el terreno, 0 si no
- `δ_support ≈ 0.2·L`: banda de degradación antes del límite IK
- σ_i = 1: soporte completo
- σ_i = 0: pie no contribuye al soporte (fuera de alcance o sin contacto)

### 5.2 Soporte activo

Un pie contribuye al soporte si `σ_i > 0`.

### 5.3 Frontera de soporte efectivo

```
x_front_support = pie con mayor (x_i · facing) entre los pies con σ_i > 0
x_rear_support  = pie con menor (x_i · facing) entre los pies con σ_i > 0
```

Cuando σ del pie trasero → 0, la frontera trasera se retira automáticamente.

---

## 6. Trigger de paso

### 6.1 Condición

```
signed_excess = (ξ − x_front_support) · facing

fire = signed_excess > −m_step   AND   !step_activo
```

Donde `m_step ≈ 0.05·L` (margen de anticipación).

Interpretación:
- Dispara cuando el XCoM está a menos de `m_step` de la frontera delantera del soporte
- A velocidad cero, ξ ≈ x_CM — solo dispara si el CM ya está muy adelante
- A alta velocidad, ξ está muy adelante del CM — dispara antes

### 6.2 No hay condición de velocidad mínima

El XCoM ya incorpora la velocidad. No hace falta gate `|ẋ_CM| > eps_v`.
A velocidad cero con CM centrado, ξ ≈ x_CM, lejos de la frontera → no dispara. ✓

### 6.3 Pie que oscila

Siempre el pie trasero:

```
swing_foot = argmin_i { x_i · facing }   entre los pies con σ_i > 0 (o todos si ambos σ=0)
```

---

## 7. Planificación del paso

### 7.1 Target de aterrizaje

```
T_swing_est  = clamp(T_min + dist_coeff · |x_desired − x_swing| / L,  T_min,  T_max)
x_desired    = x_CM + ẋ_CM · T_swing_est + m_stab · L · facing
```

Donde `m_stab ≈ 0.9` (margen de estabilidad hacia adelante, en ×L).

### 7.2 Clamp de factibilidad

```
facing > 0:
    x_land ∈ [x_stance + ε,   x_stance + d_max_corr · L]

facing < 0:
    x_land ∈ [x_stance − d_max_corr · L,   x_stance − ε]
```

El pie de swing debe aterrizar delante del pie de stance (sin cruzarse) y dentro del alcance IK.

### 7.3 Target final

```
x_land = clamp(x_desired, intervalo_factible)
y_land = terrain_h(x_land)
```

---

## 8. Trayectoria del swing

Arco Bézier cuadrático desde la posición de despegue hasta `(x_land, y_land)` con punto medio elevado `h_clear = h_clear_ratio · L` sobre la línea recta.

Paramétrico en `u ∈ [0, 1]` usando quintic smoothstep para velocidad cero en los extremos.

Duración: `T_swing` calculada en §7.1.

**Invariante**: una vez iniciado el swing, `u` solo avanza. No se cancela ni reinicia hasta `u ≥ 1.0`.

### 8.1 Heel-strike (u ≥ 1.0)

```
foot_swing.pos    = plan.land_target
foot_swing.phase  = Planted
step_activo       = false
```

Inmediatamente recalcular soporte y σ_i.

---

## 9. Reconstrucción de pose

### 9.1 Ángulo de inclinación θ

```
θ_target = atan(slope_at(x_CM)) + tanh(ẋ_CM / v_ref) · θ_max
θ        += (θ_target − θ) · dt / τ_lean
```

- `τ_lean ≈ 0.15 s`: filtro de inercia corporal (evita saltos bruscos en esquinas del terreno)
- `θ_max ≈ 10°`: inclinación máxima por velocidad
- `v_ref ≈ 1.2 m/s`: velocidad de semisaturación

### 9.2 Posición de pelvis

```
pelvis = CM − cm_pelvis_ratio · L · (sin θ, cos θ)
```

### 9.3 IK de rodillas

Sin cambios respecto a `LegIK.cpp` existente: `computeKnee(pelvis, foot, L, facing)`.

---

## 10. Inicialización (bootstrap)

Al crear o resetear la simulación:

```
y_CM = terrain_h(x_CM_spawn) + h_nominal
```

Pies colocados simétricamente:
```
foot_L.pos = (x_CM − d_pref·L/2,  terrain_h(x_CM − d_pref·L/2))
foot_R.pos = (x_CM + d_pref·L/2,  terrain_h(x_CM + d_pref·L/2))
```

Esto garantiza que el CM arranca exactamente a la altura nominal sobre el terreno en el punto de spawn — nunca en el aire.

---

## 11. Parámetros

| Parámetro | Valor sugerido | Descripción |
|---|---|---|
| `k_y` | 200 s⁻² | Rigidez del spring vertical |
| `c_y` | 20 s⁻¹ | Amortiguamiento vertical |
| `friction` | 4 s⁻¹ | Fricción de suelo horizontal |
| `move_force` | 4 m/s² | Fuerza de input del jugador |
| `k_ankle` | 0.5·g/h_nominal | Torque de tobillo abstracto en standing |
| `δ_support` | 0.2·L | Banda de degradación del soporte σ_i |
| `m_step` | 0.05·L | Margen de anticipación del trigger |
| `m_stab` | 0.9 | Margen de estabilidad del target (×L) |
| `T_min` | 0.18 s | Duración mínima del swing |
| `T_max` | 0.30 s | Duración máxima del swing |
| `dist_coeff` | 0.20 s | Crecimiento de duración por unidad dist/L |
| `d_max_corr` | 1.8·L | Alcance máximo del paso |
| `h_clear_ratio` | 0.10 | Clearance del arco de swing (×L) |
| `τ_lean` | 0.15 s | Constante de tiempo del filtro de θ |
| `θ_max` | 10° | Inclinación máxima por velocidad |
| `v_ref` | 1.2 m/s | Velocidad de semisaturación del lean |

---

## 12. Orden de implementación

1. **CM vertical**: spring-damper con y_target terrain-aware y clamp duro
2. **Bootstrap**: spawn del CM a altura correcta sobre terreno
3. **CM horizontal**: IP + move_force + friction, con x_CoP según modo
4. **Standing en pendiente**: k_ankle activo cuando !step_activo
5. **Reconstrucción de pose**: θ filtrado, pelvis, IK (sin cambios en LegIK)
6. **Soporte σ_i**: contact AND alcance IK, frontera efectiva
7. **Trigger XCoM**: condición sobre ξ vs. frontera delantera
8. **Swing y heel-strike**: trayectoria Bézier, invariante de no-cancelación
9. **Foot placement**: fórmula simple §7.1
10. **Tuning**: ajustar parámetros con escenarios headless

---

## 13. Escenarios de validación

| ID | Escenario | Condición de éxito |
|---|---|---|
| V1 | Standing en plano, sin input | CM estable, sin pasos, sin oscilación |
| V2 | Standing en pendiente, sin input | CM estable (k_ankle activo), sin deslizamiento |
| V3 | Caminar en plano (key_right 3s) | ≥3 heel-strikes, avance > 1.5m, alternancia L/R |
| V4 | Caminar y soltar tecla | El paso activo completa, luego detención suave |
| V5 | Cambio de dirección | Facing cambia, pasos correctos, sin cruce de piernas |
| V6 | Subida de pendiente | Pasos más cortos, CM más bajo, sin extensión IK |
| V7 | Bajada de pendiente | Aterrizaje controlado, no overextension del pie trasero |
| V8 | Perturbación: CM empujado fuera del soporte | Paso de recuperación dentro de un ciclo |

---

## 14. Lo que este spec no cubre (para versiones futuras)

- Salto vertical (requiere fase airborne separada)
- Running (fase de vuelo real)
- Modificación del CM durante crouch activo del jugador
- Arm swing
- Aterrizaje tras caída libre
