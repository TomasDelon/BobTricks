# BobTricks V4 — Locomotion Roadmap (rev 2)

> Revisión completa tras validar que el enfoque XCoM-gated walking era conceptualmente
> incorrecto. La nueva arquitectura unifica walking y recovery en un único trigger geométrico,
> inspirado en BobTricks_V3 (Rule 1) y stickman2 (emergency_trigger).

---

## Por qué falló el enfoque anterior

El V4 inicial usaba MoS (Margin of Support) para gatillar pasos:
- WalkNominal requería `mos >= 0`
- Durante single-support el soporte es un **punto** → MoS siempre negativo
- WalkNominal nunca disparaba después del primer paso
- Solo CaptureDynamic corría, pero CaptureDynamic no entiende walking direction
- La I_width impedía crossover de pies (bug estructural)

---

## Filosofía nueva — un solo trigger geométrico

**Invariante:** el pie trasero no puede quedarse demasiado atrás de la pelvis.

No hay distinción entre "walking" y "recovery". La misma lógica los maneja:
- Caminando: pie trasero cae lentamente → trigger → paso → alternancia
- Recovery: pie trasero cae rápido → trigger temprano → paso correctivo

No hay Falling FSM. No hay T_paso_minimo. No hay CaptureDynamic/RepairWidth/WalkNominal.

---

## Fases 1–4 — COMPLETADAS

Pies, IK, soporte, balance (XCoM display), step-plan struct: implementados y correctos.
No se tocan.

---

## Fase 5 — Walking (reescritura)

### 5.1 Variables de referencia

```
L  = body_height_m / 5   = 0.36 m
g  = 9.81 m/s²
```

### 5.2 Trigger unificado

```
rear_foot  = argmin_i( foot_i.pos.x * facing )   // el más atrás en dirección de marcha
front_foot = el otro

behind_dist = (pelvis.x - rear_foot.pos.x) * facing   // >= 0 siempre si rear es correcto

// Regla 1 — normal (requiere movimiento):
normal_trigger   = |v_x| > eps_v   AND   behind_dist > k_trigger * L

// Regla 2 — emergencia (CM escapó de todo soporte):
outside_support  = (cm.x - front_foot.pos.x) * facing > 0
emergency_trigger = outside_support   AND   behind_dist_this >= behind_dist_other

fire = (normal_trigger OR emergency_trigger) AND !step_active
```

Cuando `fire` es true, el pie trasero es siempre el que se mueve.

### 5.3 Target del paso

```
x_desired = cm.x + facing * (k_step * L + |v_x| * T_ant)
```

Target CM-relativo (no pelvis-relativo). Luego clamp de feasibility:
```
facing > 0:
    x_land ∈ [foot_stance.x + ε,  foot_stance.x + d_max_corr * L]
facing < 0:
    x_land ∈ [foot_stance.x - d_max_corr * L,  foot_stance.x - ε]
```

Sin I_reach: el spring-damper adapta la altura del CM a cualquier posición del pie.

### 5.4 Validación matemática completa

#### A — El trigger no dispara en reposo

Con pies simétricos en `d_pref = 0.9 L`:
```
rear_foot.x  = pelvis.x - d_pref/2 = pelvis.x - 0.45 L
behind_dist  = 0.45 L
k_trigger    = 0.70 L
0.45 L < 0.70 L  →  no dispara en reposo  ✓
```

#### B — Separación de pies en estado estacionario

Definición:  `D = (k_trigger + k_step) * L + |v| * T_ant`  (avance por paso)

Por simetría del ciclo alternante, la separación estacionaria es `D / 2`.

| v (m/s) | D (m) | Separación D/2 | En L  | ∈ [d_min·L, d_max·L]? |
|---------|-------|----------------|-------|------------------------|
| 0.5     | 0.651 | 0.326 m        | 0.905 | ✓ (0.75–1.20)          |
| 1.0     | 0.726 | 0.363 m        | 1.008 | ✓                      |
| 1.5     | 0.801 | 0.401 m        | 1.113 | ✓                      |
| 2.0     | 0.876 | 0.438 m        | 1.217 | ✓ (límite d_max=1.20)  |

Con L=0.36 m, k_trigger=0.70, k_step=0.90, T_ant=0.15 s.

#### C — Cooldown natural (no hace falta T_paso_minimo)

Al heel-strike, behind_dist del nuevo pie trasero es:
```
behind_dist_after = (k_trigger - k_step)/2 * L + v * (T_swing - T_ant/2)
                  = -0.10 * 0.36/2 + v * (T_swing - 0.075)
                  = -0.018 + v * (T_swing - 0.075)
```

Con v=1 m/s, T_swing≈0.22 s:
```
behind_dist_after = -0.018 + 0.145 = 0.127 m
k_trigger * L     = 0.252 m
0.127 < 0.252  →  no dispara inmediatamente tras heel-strike  ✓
```

Tiempo hasta el siguiente trigger:
```
wait = (0.252 - 0.127) / 1.0 ≈ 0.125 s   (a v=1 m/s)
```

Período total de un paso: `T_swing + wait ≈ 0.22 + 0.125 = 0.345 s`
Velocidad resultante: `D / (2 * 0.345) = 0.726 / 0.69 ≈ 1.05 m/s`  ← consistente ✓

#### D — Emergency recovery no hace re-trigger infinito

Para que no re-dispare inmediatamente, el pie aterrizado debe estar por delante del CM:
```
Condición:  v * (T_swing - T_ant) < k_step * L
Con T_swing_min=0.18, T_ant=0.15:
            v * 0.03 < k_step * L = 0.324
            v < 10.8 m/s   ✓  (terminal_v = 1 m/s con move_force=4, friction=4)
```

#### E — Tamaño del paso vs. d_max_correction

```
D_max (v=2 m/s) = 0.876 m
d_max_corr * L  = 1.8 * 0.36 = 0.648 m

D_max/2 = 0.438 m (por paso individual) < 0.648 m  ✓

El paso individual (de un pie) es D/2, que siempre cabe en el reach.
El target x_desired = cm.x + 0.363 m @v=1  (CM-relativo):
  clamp ∈ [foot_stance.x + ε,  foot_stance.x + 0.648 m]  ✓
  El clamp no activa en estado estacionario a velocidades normales.
```

#### F — Velocidad terminal con nueva config

```
move_force = 4 N/kg,  floor_friction = 4 s⁻¹
terminal_v = 4 / 4 = 1.0 m/s
XCoM ahead = v / ω₀ = 1.0 / sqrt(9.81/0.9) ≈ 1.0 / 3.3 = 0.30 m
Support right = front_foot.x ≈ pelvis.x + D/2 = pelvis.x + 0.31 m
0.30 < 0.31  →  MoS ≥ 0 en steady state  ✓  (balance estable al caminar)
```

---

## Parámetros (WalkConfig)

| Param       | Significado                                         | Valor |
|-------------|-----------------------------------------------------|-------|
| `k_trigger` | dispara cuando pie trasero está este factor×L detrás | 0.70 |
| `k_step`    | pie aterriza este factor×L adelante del CM           | 0.90 |
| `T_ant`     | anticipación de velocidad para el target (s)         | 0.15 |

Eliminados respecto al diseño original: `v_ref`, `k_bal`, `T_paso_minimo`.
Nota: la revisión 2 del roadmap usaba `k_reach = 0.60` (pelvis-relativo). El valor
efectivo en código es `k_step = 0.90` (CM-relativo), ajustado empíricamente para
obtener un walking estable. La spec en `LOCOMOTION_SPEC.md` refleja el estado real.

---

## Fase 6 — CM vertical (oscilación de marcha)

Actualmente el CM usa spring-damper a altura fija. Durante la marcha, el CM real
sube en mid-stance y baja en double-support (compás del péndulo invertido).

Implementación: añadir bob en `y_CM_target` cuando hay un paso activo:
```
phi  = (sim_time - plan.t_start) / plan.duration     ∈ [0,1]
A    = k_bob * L * tanh(|v_x| / v_ref_bob)
y_CM_target += A * sin(π * phi)                       // sube en mid-swing
```

> Esta fase es opcional para el primer commit funcional.
> El personaje camina correctamente sin ella; solo le falta el bobbing visual.

---

## Arquitectura final de StepPlanner

### G1 — shouldStep  (nueva firma)
```cpp
bool shouldStep(const Vec2&      pelvis,
                const FootState& foot_L,
                const FootState& foot_R,
                const CMState&   cm,
                const StepPlan&  plan,
                const StandingConfig& stand_cfg,
                const WalkConfig&     walk_cfg,
                double L,
                double facing,
                bool*  out_emergency = nullptr);
```
Sin BalanceState, sin trigger_type, sin sim_time, sin last_heel_strike_t.

### G2 — planStep  (nueva firma)
```cpp
StepPlan planStep(bool            swing_right,
                  bool            emergency,
                  const FootState& foot_swing,
                  const FootState& foot_stance,
                  const CMState&   cm,
                  const StepConfig&    step_cfg,
                  const StandingConfig& stand_cfg,
                  const WalkConfig&    walk_cfg,
                  const Terrain&   terrain,
                  double sim_time,
                  double L,
                  double facing,
                  Vec2   pelvis_toeoff);
```

### G3 — evalSwingFoot  (sin cambios)

---

## Orden de implementación

### Bloque A — Configuración  (sin cambio visual)
A1. Actualizar `WalkConfig` en AppConfig.h
A2. Actualizar config.ini
A3. Actualizar ConfigIO.cpp (read/write nuevos params)

### Bloque B — StepPlanner G1+G2  (sin cambio visual)
B1. Actualizar StepPlanner.h (nuevas firmas)
B2. Reescribir shouldStep en StepPlanner.cpp
B3. Reescribir planStep en StepPlanner.cpp

### Bloque C — Application.cpp  (**VALIDACIÓN VISUAL**)
C1. Quitar Falling FSM (o reducirlo a log-only)
C2. Conectar nuevas firmas de shouldStep/planStep
→ El personaje debe caminar al mantener Q o D

### Bloque D — DebugUI  (**VALIDACIÓN VISUAL menor**)
D1. Panel balance: quitar slider de mos_threshold o hacerlo read-only
D2. Panel step: mostrar behind_dist y emergency flag en vez de trigger_type

### Bloque E — CM bobbing  (**VALIDACIÓN VISUAL**)
E1. Añadir oscilación de y_CM_target durante swing
→ El personaje debe oscilar verticalmente al caminar (efecto visual)

---

## Estado actual

| Fase | Estado |
|------|--------|
| 1 — FootState + SupportState + bootstrap | ✅ DONE |
| 2 — IK + rodillas + StandingPose | ✅ DONE |
| 3 — BalanceComputer + DebugUI | ✅ DONE |
| 4 — StepPlanner (structs y G3) | ✅ DONE |
| 5 — Walking trigger unificado | ✅ DONE |
| 6 — CM bobbing | ✅ DONE |
