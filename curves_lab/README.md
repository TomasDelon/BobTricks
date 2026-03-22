# Curves Lab

Módulo nuevo y aislado para estudiar y prototipar curvas paramétricas sin tocar el código existente de `BobTricks_V4`.

Objetivos de esta primera iteración:

- separar representación de curva, evaluación, muestreo y validación;
- soportar Bézier cuadrática y cúbica;
- soportar Catmull-Rom centrípeta;
- soportar aproximación de longitud de arco y remapeo `s -> t`;
- dejar tests matemáticos independientes del resto del proyecto.

Este laboratorio está pensado para futuras integraciones en:

- trayectorias de cámara;
- trayectorias de pelvis, manos o pies;
- render procedural de brazos y piernas;
- paths de debug y tooling;
- futuros sistemas de timing por longitud de arco.

## Estructura

- `doc/01_scientific_foundations.md`
- `doc/02_architecture.md`
- `doc/03_validation_strategy.md`
- `src/`
- `tests/`

## Compilación independiente

Desde la raíz de `BobTricks_V4`:

```bash
g++ -std=c++20 \
  curves_lab/src/Bezier2.cpp \
  curves_lab/src/CatmullRom2.cpp \
  curves_lab/src/ArcLength.cpp \
  curves_lab/tests/test_curves.cpp \
  -I src \
  -I curves_lab/src \
  -O2 \
  -lm \
  -o curves_lab/tests/test_curves

./curves_lab/tests/test_curves
```

No requiere SDL, ImGui, Box2D ni modificaciones del `Makefile`.
