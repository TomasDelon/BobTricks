# Proposed Architecture for the Curves Module

## 1. Principio central

Separar cinco capas:

1. datos de curva;
2. evaluación matemática;
3. reparametrización por longitud de arco;
4. composición por tramos;
5. adaptadores de uso en cámara/render/animación.

En esta iteración se implementan las capas 1 a 3.

## 2. Tipos implementados ahora

### `QuadraticBezier2`

Uso previsto:

- limbs simples;
- overlays;
- trayectorias visuales suaves pero baratas.

API principal:

- `evalByBernstein(t)`
- `evalByCasteljau(t)`
- `derivative(t)`
- `secondDerivative(t)`
- `sample(t)`
- `split(t)`

### `CubicBezier2`

Uso previsto:

- cámara;
- paths de cuerpo;
- formas suaves con control local razonable;
- base común para conversión desde otras representaciones.

API principal:

- evaluación por Bernstein y de Casteljau;
- derivadas primera y segunda;
- curvatura;
- subdivisión.

### `CatmullRomSegment2`

Uso previsto:

- curvas que deben interpolar puntos dados;
- edición basada en key points;
- authored camera rails y motion guides.

Decisión:

- default `alpha = 0.5` centrípeta;
- evaluación Hermite interna;
- conversión a Bézier cúbica para unificar sampling/render futuro.

### `ArcLengthLUT2`

Uso previsto:

- avanzar a velocidad aproximadamente constante;
- remapear tiempo a distancia;
- sampling uniforme espacialmente.

API principal:

- `totalLength()`
- `parameterAtArcLength(s)`
- `parameterAtFraction(u)`
- `arcLengthAtParameter(t)`
- `uniformParametersByArcLength(n)`

## 3. Qué no se implementa todavía

- B-splines por tramos;
- NURBS;
- PH curves;
- fitting/global optimization;
- constraints cinemáticas o físicas;
- adaptación 3D;
- flattening/render adaptativo con cota de error visible;
- curvas con torsión o rotation-minimizing frames.

Eso queda deliberadamente fuera para mantener una primera entrega sólida y validable.

## 4. Cómo escalar esto luego

### Fase siguiente plausible

Añadir:

- `PiecewiseCurve2`;
- `CubicHermite2`;
- `UniformCubicBSpline2`;
- conversores `CatmullRom -> CubicBezier chain`;
- sampler adaptativo por error geométrico.

### Integraciones futuras

1. Cámara
   - un rail compuesto por segmentos;
   - evaluación por longitud de arco para velocidad constante;
   - easing externo desacoplado del path.

2. Cuerpo
   - trayectorias swing-foot o mano;
   - perfiles de pose authored;
   - blending entre control points.

3. Render procedural
   - brazos o piernas dibujados como curva a partir de hombro/codo/mano;
   - sampling con grosor y normales.

4. Herramientas de debug
   - overlays de tangente;
   - curvatura;
   - puntos de control;
   - error de aproximación.

## 5. Reglas de diseño para no degradar el módulo

- no acoplar la matemática a SDL, ImGui o Box2D;
- no mezclar evaluación con render;
- no esconder parámetros de continuidad;
- no usar nombres ambiguos (`spline` genérico sin tipo concreto);
- no introducir heurísticas temporales dentro del núcleo geométrico;
- toda extensión nueva debe venir con tests matemáticos.
