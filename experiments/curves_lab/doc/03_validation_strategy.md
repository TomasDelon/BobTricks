# Scientific Validation Strategy

## 1. Objetivo

No basta con que una curva "se vea linda". El módulo tiene que demostrar:

- exactitud algebraica básica;
- coherencia geométrica;
- robustez numérica;
- utilidad práctica para futuras integraciones.

## 2. Bucle científico

Para cada nueva feature del módulo:

1. definir la propiedad matemática esperada;
2. construir un test mínimo que la aísle;
3. predecir el resultado numérico antes de ejecutar;
4. ejecutar;
5. medir error;
6. aceptar o corregir;
7. si el resultado no coincide, cuestionar también el test.

## 3. Propiedades que se validan en esta iteración

### Bézier

1. Interpolación de extremos
   - `B(0)=P0`
   - `B(1)=Pn`

2. Derivadas en extremos
   - cuadrática: `B'(0)=2(P1-P0)`, `B'(1)=2(P2-P1)`
   - cúbica: `B'(0)=3(P1-P0)`, `B'(1)=3(P3-P2)`

3. Equivalencia Bernstein vs de Casteljau
   - mismas posiciones para varios `t`

4. Subdivisión
   - el punto de split coincide exactamente con la curva original en ese `t`

### Catmull-Rom

5. Interpolación
   - `C(0)=P1`
   - `C(1)=P2`

6. Conversión a Bézier
   - extremos preservados

7. Robustez ante puntos repetidos
   - sin NaN ni división por cero cuando hay tramos degenerados

### Longitud de arco

8. Curva recta
   - una Bézier colineal debe tener longitud exacta o casi exacta

9. Monotonía
   - `s(t)` debe ser monótona creciente
   - `t(s)` debe ser monótona creciente

10. Extremos
   - `t(0)=0`
   - `t(L)=1`

## 4. Métricas de aceptación

Tolerancias usadas en esta primera iteración:

- posiciones: `1e-9` a `1e-10` para identidades exactas simples;
- igualdad de evaluadores: `1e-12` en casos suaves;
- longitud de arco recta: `1e-9`;
- monotonicidad: estricta salvo ruido numérico mínimo.

## 5. Qué falta validar más adelante

- error de remapeo por longitud de arco en curvas muy curvadas;
- estabilidad con segmentos casi colapsados;
- error de flattening adaptativo para render;
- continuidad `C1/G1/C2` entre tramos;
- comparación contra ejemplos de literatura o implementaciones de referencia.

## 6. Regla para futuras extensiones

Toda nueva curva o spline debe venir con:

- test de extremos;
- test de derivadas;
- test de continuidad si aplica;
- test de longitud o sampling si pretende usarse para timing.
