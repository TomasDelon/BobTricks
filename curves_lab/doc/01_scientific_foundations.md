# Scientific Foundations for Splines and Bézier Curves

## 1. Qué aporta `others/stickman.zip`

El código de `others/stickman.zip` tiene una intuición útil y varias limitaciones claras.

Lo útil:

- usa curvas Bézier cuadráticas para dibujar miembros a partir de tres puntos de control;
- separa visualmente la idea "articulación -> curva" de "articulación -> polilínea rígida";
- sugiere que una misma familia de curvas puede reutilizarse para torso, brazos y piernas.

Lo débil:

- la curva está acoplada al renderer SDL y al objeto `Stickman`;
- la evaluación es solo cuadrática y solo 2D;
- el muestreo es a resolución fija (`RES = 50`), sin criterio geométrico ni error;
- no hay noción de longitud de arco, velocidad tangencial ni remapeo temporal;
- no hay contratos explícitos de continuidad (`C0/C1/G1/G2`);
- no hay validación matemática independiente del render.

Conclusión: sirve como inspiración visual, pero no como base de arquitectura.

## 2. Qué conceptos científicos importan aquí

Para este proyecto importan cinco familias de ideas:

1. Representación polinómica local.
   Las curvas deben poder evaluarse, derivarse, subdividirse y muestrearse de forma robusta.

2. Continuidad geométrica.
   Para trayectorias de cámara o partes del cuerpo, `C0` no basta. Normalmente interesa al menos `C1`, y a veces `G1` o `C2`.

3. Parametrización.
   El parámetro `t` rara vez coincide con distancia recorrida. Si luego quieres timing estable o speed profiling, necesitas longitud de arco o una buena aproximación.

4. Interpolación vs aproximación.
   Catmull-Rom interpola puntos dados; Bézier y B-spline normalmente aproximan. Cada uso futuro del motor necesita elegir conscientemente una de las dos.

5. Control local.
   Para edición y animación, una modificación local no debería deformar toda la trayectoria.

## 3. Fuentes científicas recomendadas

### Fundamentos de Bézier y B-splines

1. Carl de Boor, *A Practical Guide to Splines*.
   Referencia clásica sobre splines y B-splines.
   Link: https://link.springer.com/book/9780387953663

2. Hartmut Prautzsch, Wolfgang Boehm, Marco Paluszny, *Bézier and B-Spline Techniques*.
   Buen texto moderno de CAGD con foco geométrico.
   Link: https://link.springer.com/book/10.1007/978-3-662-04919-8

### Catmull-Rom y parametrización

3. Cem Yuksel, Scott Schaefer, John Keyser, *On the Parameterization of Catmull-Rom Curves*.
   Resultado importante: la parametrización centrípeta evita cúspides y auto-intersecciones por segmento dentro de la familia `alpha-Catmull-Rom`.
   Project page + preprint:
   https://www.cemyuksel.com/research/catmullrom_param/

4. Cem Yuksel, Scott Schaefer, John Keyser, *Parameterization and Applications of Catmull-Rom Curves*, Computer-Aided Design, 2011.
   Extensión aplicada del trabajo anterior.
   Project page:
   https://www.cemyuksel.com/research/catmullrom_param/

### Longitud de arco, geometría diferencial y trayectorias

5. Rida T. Farouki, Carlotta Giannelli, Duccio Mugnaini, Alessandra Sestini,
   *Path planning with Pythagorean-hodograph curves for unmanned or autonomous vehicles*.
   Muy útil para entender cuándo vale la pena pasar de Bézier/splines generales a curvas con longitud de arco estructuralmente favorable.
   Link:
   https://escholarship.org/uc/item/1p74v16x

6. Rida T. Farouki, *Construction of G1 planar Hermite interpolants with prescribed arc lengths*.
   Fuente útil para diseño de trayectorias con restricciones geométricas más fuertes.
   Link:
   https://escholarship.org/uc/item/0nm112xb

### Ajustabilidad y variantes modernas

7. Juncheng Li, Chengzhi Liu, Shanjun Liu,
   *The quartic Catmull-Rom spline with local adjustability and its shape optimization*.
   Interesante para extensiones futuras con shape parameters.
   Link:
   https://advancesincontinuousanddiscretemodels.springeropen.com/articles/10.1186/s13662-022-03730-8

## 4. Decisiones científicas para este módulo

### 4.1 Elegir primero curvas polinómicas simples

En esta etapa conviene empezar por:

- Bézier cuadrática;
- Bézier cúbica;
- Catmull-Rom centrípeta;
- utilidades de longitud de arco.

No empezar por NURBS ni por PH curvas dentro del engine principal. Son valiosas, pero aumentan demasiado la complejidad si aún no existe una infraestructura simple y bien validada.

### 4.2 Elegir Catmull-Rom centrípeta, no uniforme

Si el futuro uso incluye seguir puntos clave del cuerpo o waypoints de cámara, la parametrización centrípeta es una mejor default que la uniforme.

Razón:

- reduce overshoot;
- evita cúspides y auto-intersecciones por segmento dentro de la familia analizada por Yuksel et al.;
- produce curvas más cercanas a la intuición del diseñador.

### 4.3 Tratar longitud de arco como aproximación numérica controlada

Para curvas polinómicas generales, la longitud de arco rara vez tiene forma cerrada útil en runtime.

Entonces la estrategia razonable es:

- derivada analítica exacta;
- integración numérica robusta para `s(t)`;
- tabla `t <-> s` para remapeo rápido.

Eso es suficiente para cámara, timing visual y gait helpers, sin sobrediseñar todavía.

## 5. Hipótesis de diseño que este módulo asume

- 2D primero, 3D después;
- representación y evaluación puramente matemáticas, sin renderer;
- un `Vec2` liviano alcanza para la primera iteración;
- longitud de arco aproximada con tolerancias controladas alcanza para usos visuales y de gameplay;
- la API debe permitir futuro crecimiento hacia splines por tramos y edición local.
