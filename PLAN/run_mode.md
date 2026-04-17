# Plan: Modo correr (tecla mantenida)

## Base científica

El modelo SLIP (*Spring-Loaded Inverted Pendulum*, Blickhan 1989) es el
estándar para correr bípedo. Su diferencia esencial con el péndulo invertido
(modo caminar) es:

| | Caminar (IP) | Correr (SLIP) |
|---|---|---|
| Fase de apoyo | pierna rígida, arco "vaulting" | pierna resorte, CM baja y sube |
| Doble apoyo | ~60 ms obligatorio | **cero** (fase de vuelo en su lugar) |
| Fase de vuelo | nunca | ~35% del ciclo |
| Colocación del pie | bajo el CM o detrás | delante del CM (amortiguación) |

Para este sistema no es necesario implementar SLIP completo. El resorte
vertical actual (`integrateVerticalMotion`) ya actúa como muelle si se lo
deja actuar durante el vuelo, y la fase de vuelo emerge si liberamos el pie
de apoyo antes de que aterrice el pie en swing.

---

## Principio de implementación (minimal SLIP aproximado)

La clave está en **cuándo se libera el pie de apoyo**:

- Caminar: el pie de apoyo se libera solo cuando el otro ya aterrizó
- Correr: el pie de apoyo se libera mientras el otro aún está en el arco
  (en torno a `swing_t ≈ 0.55`), creando un instante con ambos pies en el
  aire

El CM en ese instante ya tiene velocidad vertical positiva (el resorte lo
empujó durante la fase de apoyo). Al liberar el pie de apoyo, el resorte
vertical se desconecta (`airborne_final = true`) y el CM vuela balisticamente.
Cuando el pie en swing aterriza, el nuevo apoyo comienza.

Esto no requiere cambiar el modelo físico vertical — solo el timing del
trigger y un umbral de `swing_t` para la liberación.

---

## Archivos a tocar

| Archivo | Qué cambia |
|---|---|
| `src/core/simulation/InputFrame.h` | + `bool key_run` |
| `src/app/Application.cpp` | mapear Shift a `key_run` |
| `src/config/AppConfig.h` | + `RunConfig` struct |
| `data/config.ini` | + sección `[run]` |
| `src/core/character/CharacterState.h` | + `bool run_mode`, `bool run_flight_active` |
| `src/core/simulation/SimulationCore.cpp` | lógica de modo corrida |

---

## Paso 1 — Input y config (sin tocar física)

### `InputFrame.h`
```cpp
bool key_run = false;   // Shift mantenido
```

### `Application.cpp`
```cpp
case SDLK_LSHIFT:
case SDLK_RSHIFT: m_key_run = pressed; break;
// ...
input.key_run = m_key_run && (m_key_left || m_key_right);
```

### `AppConfig.h` — nuevo struct después de `WalkConfig`
```cpp
struct RunConfig {
    // Velocidad y pasos
    double max_speed         = 3.5;   // [m/s]  cap de velocidad corriendo
    double accel_factor      = 1.8;   // [×]    multiplica physics.accel
    double step_speed        = 9.0;   // [steps/s]  avance de swing_t
    double stability_margin  = 1.8;   // [×L]   destino del pie adelante del xi
    double max_step_L        = 2.5;   // [×L]   máximo paso desde pie de apoyo
    double d_rear_max        = 1.2;   // [×L]   umbral para trigger trasero
    double xcom_scale        = 0.75;  // lookahead del capture point

    // Fase de vuelo
    double flight_release_t  = 0.55;  // swing_t del pie activo en que se libera el apoyo
    double flight_vy_min     = 0.3;   // [m/s]  impulso vertical mínimo al despegar
                                      //        (si el CM no tiene suficiente vy, se lo añade)

    // Altura y rebote
    double leg_flex_coeff    = 0.18;  // más flexión de rodilla que caminando
    double bob_scale         = 4.5;   // arco IP más pronunciado (rebote más visible)
    double bob_amp           = 0.25;  // [×L]  amplitud máxima del rebote

    // Clearance del swing
    double h_clear_ratio     = 0.30;  // [×L]  base de lift (más alto que caminando)
    double h_clear_min_ratio = 0.15;  // [×L]  mínimo garantizado

    // Transición suave walk→run
    double blend_tau         = 0.12;  // [s]   constante de tiempo del blend
};
```

Añadir a `AppConfig`:
```cpp
RunConfig run;
```

---

## Paso 2 — CharacterState

```cpp
// En CharacterState.h
bool run_mode         = false;  // true mientras key_run activo y en movimiento
bool run_flight_active = false; // true durante la fase de vuelo corriendo
double run_blend      = 0.0;    // 0=walk, 1=run (transición suave)
```

---

## Paso 3 — SimulationCore: lógica del modo correr

En `SimulationCore::step`, los cambios se añaden **en puntos existentes**, no
se reescribe el flujo.

### 3a. Detección del modo

Al inicio de `step()`, antes de calcular física:

```cpp
const bool want_run = input.key_run
                   && std::abs(cm.velocity.x) > m_config.physics.stop_speed;
const double run_tau   = m_config.run.blend_tau;
const double run_alpha = 1.0 - std::exp(-dt / run_tau);
ch.run_blend += (want_run ? 1.0 : 0.0 - ch.run_blend) * run_alpha;
ch.run_mode   = (ch.run_blend > 0.5);

// Limpiar flight si dejamos de correr
if (!ch.run_mode) ch.run_flight_active = false;
```

### 3b. Parámetros efectivos (blend walk→run)

Antes de usar WalkConfig, computar valores mezclados:

```cpp
const double rb = ch.run_blend;  // [0,1]

const double eff_max_speed  = std::lerp(m_config.physics.walk_max_speed,
                                         m_config.run.max_speed, rb);
const double eff_step_speed = std::lerp(m_config.walk.step_speed,
                                         m_config.run.step_speed, rb);
const double eff_stab_margin = std::lerp(m_config.walk.stability_margin,
                                          m_config.run.stability_margin, rb);
const double eff_max_step_L  = std::lerp(m_config.walk.max_step_L,
                                          m_config.run.max_step_L, rb);
const double eff_bob_scale   = std::lerp(m_config.walk.bob_scale,
                                          m_config.run.bob_scale, rb);
const double eff_leg_flex    = std::lerp(m_config.walk.leg_flex_coeff,
                                          m_config.run.leg_flex_coeff, rb);
// etc.
```

Luego pasar estos valores efectivos a las funciones que los usan
(`advanceSwingFoot`, `computeStepLandingX`, `computeHeightTargetState`).
*Nota: actualmente esas funciones reciben `m_config.walk` directamente —
habrá que pasarles los valores efectivos o crear una `WalkConfig` mezclada.*

### 3c. Liberación del pie de apoyo (crear fase de vuelo)

Inmediatamente después del bloque `advanceSwingFoot`, añadir:

```cpp
if (ch.run_mode && !ch.run_flight_active) {
    const FootState* swing_f = ch.foot_left.swinging  ? &ch.foot_left
                             : ch.foot_right.swinging ? &ch.foot_right
                                                      : nullptr;
    if (swing_f && swing_f->swing_t >= m_config.run.flight_release_t) {
        FootState& stance = ch.foot_left.swinging ? ch.foot_right : ch.foot_left;
        if (stance.on_ground) {
            // Impulso vertical mínimo de despegue
            if (cm.velocity.y < m_config.run.flight_vy_min)
                cm.velocity.y = m_config.run.flight_vy_min;
            stance.pinned    = false;
            stance.on_ground = false;
            ch.run_flight_active = true;
        }
    }
}
```

Con esto, cuando `airborne_final` se vuelve `true` (el pelvis sube > 2L o
los dos pies se desprenden), el resorte vertical ya está desactivado por el
código existente — el CM vuela bajo la gravedad.

### 3d. Reset del flag de vuelo al aterrizar

Cuando un pie que estaba swinging aterriza (heel-strike detectado):

```cpp
if ((was_swinging_L && !ch.foot_left.swinging)
 || (was_swinging_R && !ch.foot_right.swinging)) {
    ch.run_flight_active = false;
}
```

### 3e. Trigger de paso durante vuelo (opcional para polish)

En el bloque de triggers, permitir que el trigger dispare durante vuelo si
estamos en modo correr, para que el siguiente paso se inicie tan pronto como
el pie de swing aterriza:

```cpp
const bool can_step = ch.feet_initialized
                   && ch.step_cooldown <= 0.0
                   && !any_swinging
                   && (!airborne_final || (ch.run_mode && ch.run_flight_active));
                   //                     ↑ en vuelo corriendo, preparar el paso
```

*Sin esto el personaje haría una pequeña pausa en cada aterrizaje. Con esto,
dispara el siguiente paso en el mismo frame que aterriza.*

---

## Paso 4 — Tune visual (AppConfig → config.ini)

Una vez que la fase de vuelo funciona, ajustar:

| Param | Walk actual | Run objetivo |
|---|---|---|
| `theta_max_deg` | 6° | ~12° (más lean) |
| `arm swing` | ya existe | amplitude mayor via ArmConfig |
| `h_clear_ratio` (run) | 0.10 | 0.30 |
| `bob_amp` (run) | 0.15L | 0.25L |
| `double_support_time` | 0.06s | 0 (no aplica, ya no hay DS) |

---

## Orden de implementación recomendado

1. **Paso 1** (input + RunConfig) — compila y no cambia nada visible
2. **Paso 2** (CharacterState flags) — ídem
3. **Paso 3a+3b** (blend de parámetros) — el personaje acelera más, pasos
   más largos, pero sin vuelo todavía. Verificar visualmente que el blend
   se ve suave.
4. **Paso 3c+3d** (liberación de stance + reset) — aparece la fase de vuelo.
   Es el cambio más arriesgado; hacer `make test` antes y después.
5. **Paso 3e** (trigger en vuelo) — polish de cadencia.
6. **Paso 4** (tune) — ajustar a ojo con la UI de debug.

---

## Riesgos y mitigaciones

| Riesgo | Mitigación |
|---|---|
| `airborne_final` no se activa (pelvis < 2L en vuelo corto) | Reducir umbral a 1.2L en run_mode, o usar los `on_ground` flags como detección de vuelo en vez del pelvis |
| El impulso `flight_vy_min` choca con la física vertical | Clampearlo: solo aplicar si `cm.velocity.y < flight_vy_min`, no añadir si ya es mayor |
| Transition walk→run corta los steps a medio camino | El blend suave en `run_blend` garantiza que la transición solo afecta a pasos futuros; el swing en curso usa los parámetros con que fue iniciado (swing_speed_scale y h_clear se almacenan por pie en FootState) |
| Tests de regresión fallan por nuevos parámetros | RunConfig solo se activa cuando `run_blend > 0`, nunca en los escenarios headless existentes que no pasan `key_run` |
