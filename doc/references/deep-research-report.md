# Deep research sobre recuperación del equilibrio, IK planar y control físico para un bípedo 2D tipo stickman

## Resumen ejecutivo

Los huecos que señalas están bien elegidos porque atacan **el núcleo duro** de un bípedo físico: (a) cuándo un estado es *recuperable* o *irrecuperable* (capturability/capture point), (b) cómo pasar de objetivos “centroidales” (CoM/contacts/constraints) a una **postura completa** (IK con restricciones) y (c) cómo convertir esa postura en **fuerzas/torques** estables en un motor rígido discreto (PD/impedance/active ragdoll). La literatura más útil para ti no es la más “bonita” matemáticamente, sino la que te da **criterios operables** (regiones de captura, step planning reactivo, prioridades de tareas, y PD estable a dt grandes). citeturn2search3turn0search9turn12search3turn11search5

En *push recovery* y *stepping recovery*, los trabajos fundacionales sobre **Capture Point** y **N-step capturability** formalizan exactamente lo que pides (“recoverable vs unrecoverable” y “uno o varios pasos”), y se conectan bien con controladores reactivos (ankle/hip/step) y MPC para imponer restricciones (ZMP/CoP/support polygon). Para un stickman 2D, tu simplificación a plano no es un problema: muchas de estas técnicas nacieron con modelos simplificados 2D/CoM y luego se extendieron. citeturn2search3turn0search9turn11search11turn10search13

En el “puente” **CoM + contactos + constraints → joint targets**, la pista que mejor funciona en la práctica es adoptar una formulación de *task-space / stack-of-tasks / HQP*: defines tareas (CoM, pies, tronco), fijas prioridades y restricciones (contactos, límites articulares) y resuelves un IK/optimización por frame. En 2D puedes mantener lo esencial (prioridades + contactos) y reemplazar partes por IK analítica de pierna 2 eslabones cuando convenga, usando la optimización solo para coordinar tareas y constraints. citeturn3search2turn12search3turn12search22

Para *active ragdoll* y control PD en motores rígidos, el mayor riesgo práctico en engines discretos es la **inestabilidad numérica** al subir ganancias o usar dt relativamente grande. Por eso, los trabajos sobre **Stable PD (SPD)** son particularmente “cercanos a implementación”: proponen formular PD considerando el estado del próximo paso de integración para permitir ganancias altas sin explotar, incluso con integradores simples. Esto es oro para un stickman. citeturn11search9turn11search5turn4search7

Finalmente, separar “get-up” de “validación” es correcto: *get-up* es un problema de control/hibridación (contactos múltiples, planificación de secuencias), mientras que “realismo” requiere métricas: estabilidad dinámica (p. ej., MoS/XCoM), trayectorias de CoM/CoP/GRF y comparaciones estadísticas con datos humanos. Las mejores fuentes aquí combinan métodos de gráficos por simulación física con papers de biomecánica. citeturn6search15turn6search3turn7search10turn7search13

## Cómo traducir esta literatura a tu stickman 2D

Para evitar ruido, conviene mapear cada bloque a **artefactos implementables**:

Un pipeline práctico en 2D suele separar tres capas:

1) **Capa centroidal / equilibrio**: estado del CoM (posición/velocidad), contactos activos, estimación de región de soporte (point foot vs foot segment), y cálculo de un objetivo “recuperable” (capture point / capturability / foot placement). Aquí encajan Capture Point, capturability de N pasos y FPE. citeturn2search3turn0search9turn2search4turn2search16

2) **Capa de reconstrucción de postura**: conviertes objetivos (CoM target, foot target(s), trunk orientation) + constraints (contactos, soporte) en **joint targets**. Esto es exactamente “inverse kinetics / whole-body IK con tareas” (stack-of-tasks/HQP) y, en 2D, puedes mezclarlo con IK cerrada de pierna 2 eslabones. citeturn12search3turn3search2turn12search22

3) **Capa de control físico (torques/fuerzas)**: aplicas control de articulaciones (PD/impedance), robusto ante dt discreto e impactos. Para esto, SPD y controladores tipo SIMBICON/GBWC son referencias cercanas a implementación. citeturn11search5turn9search2turn11search2

En tu caso, el “mínimo viable” que más reduce riesgos es: **stepping-recovery + pose IK simple + SPD**. Con eso, puedes iterar en calidad (heel/toe, support polygon, prioridades, get-up) sin que la simulación sea frágil. citeturn2search3turn0search9turn11search5turn5search0

---

## Capturability, capture point, push recovery y stepping recovery

A continuación, una bibliografía priorizada con foco en: *recoverable vs unrecoverable*, uno o varios pasos, y stepping recovery.

**Capture Point: A Step toward Humanoid Push Recovery**  
Autores: entity["people","Jerry Pratt","humanoid robotics"]; John Carff; Sergey Drakunov; entity["people","Ambarish Goswami","robotics researcher"]  
Año: 2006  
Tipo: paper (IEEE Humanoids)  
Resumen: Introduce el **Capture Point** como punto del suelo donde el bípedo puede “pisar y parar” tras una perturbación, e introduce la noción de **Capture Region** (conjunto de puntos factibles) en modelos simplificados, incluyendo extensión con “flywheel” (momento angular) para ampliar la región de captura. Es una base directa para formalizar “recuperable vs irrecuperable” bajo hipótesis claras. citeturn2search3turn0search18  
Relevancia stickman/simulación: altísima; te da una receta concreta para stepping recovery en 2D (y el vocabulario estándar del área).  
Nivel de utilidad: **muy útil**.  
Enlace primario: citeturn2search3

**Capturability-based Analysis and Control of Legged Locomotion, Part 1: Theory and Application to Three Simple Gait Models**  
Autores: entity["people","Twan Koolen","humanoid robotics"]; Jerry Pratt; et al.  
Año: 2012  
Tipo: paper (IJRR; PDF de autor)  
Resumen: Formaliza **N-step capturability**: capacidad de detenerse sin caer usando N pasos o menos, y deriva **capture regions** y secuencias de control para modelos de marcha simplificados. Es de las fuentes más directas para tu necesidad “uno o varios pasos de recuperación” con criterio analítico. citeturn0search5turn0search9  
Relevancia stickman/simulación: altísima; puedes implementar N-step capturability en 2D como chequeo de “es recuperable con 1 paso / 2 pasos”.  
Nivel de utilidad: **muy útil**.  
Enlace primario: citeturn0search5

**Humanoid Push Recovery**  
Autores: entity["people","Benjamin Stephens","robotics researcher"]; entity["people","Christopher Atkeson","robotics researcher"]  
Año: 2007  
Tipo: paper (PDF de CMU)  
Resumen: Estudia tres estrategias de recuperación que luego tú vas a necesitar integrar: **ankle strategy**, **hip/internal joints** y **stepping**. Es valioso porque organiza el problema como un “menú de reacciones” en función de la perturbación y las limitaciones de contacto/soporte. citeturn11search11  
Relevancia stickman/simulación: muy alta; te sirve para diseñar el “árbol de decisiones”: si no cabe en soporte → stepping; si cabe → ankle/hip.  
Nivel de utilidad: **muy útil**.  
Enlace primario: citeturn11search11

**Push Recovery Control for Force-Controlled Humanoid Robots**  
Autores: Benjamin Stephens; Christopher Atkeson  
Año: 2011  
Tipo: tesis (PDF de CMU)  
Resumen: Documento de ingeniería muy explotable: conecta modelos simples (CoM) con controladores realizables en el robot, incluyendo stepping recovery, MPC (PR-MPC) y control de fuerzas/torques a cuerpo completo. Para ti, es especialmente útil por la estructura de sistema y los detalles de implementación/control bajo contactos. citeturn11search3turn2search2  
Relevancia stickman/simulación: altísima si quieres construir un pipeline coherente (planificador reactivo → controlador a torques).  
Nivel de utilidad: **muy útil**.  
Enlace primario: citeturn11search3

**Introduction of the Foot Placement Estimator: A Dynamic Measure of Balance for Bipedal Robotics**  
Autores: Derek L. Wight; Eric G. Kubica; David W. L. Wang  
Año: 2008  
Tipo: paper (ASME J. Computational and Nonlinear Dynamics; PDF)  
Resumen: Define el **Foot Placement Estimator (FPE)** como medida dinámica para decidir dónde colocar el pie y restaurar equilibrio desde estados desbalanceados. Complementa capture point/capturability desde otra óptica y es muy citado en control de bípedos. citeturn2search4turn2search7  
Relevancia stickman/simulación: alta; especialmente si tu bípedo es “planar con point feet” o si quieres un estimador alternativo a CP/ICP.  
Nivel de utilidad: **muy útil**.  
Enlace primario: citeturn2search4

**Foot placement for planar bipeds with point feet**  
Autores: entity["people","Pieter van Zutven","robotics researcher"]; et al.  
Año: 2012  
Tipo: paper (PDF)  
Resumen: Documento muy alineado con tu caso porque trata **planar bipeds con point feet** y foot placement. Es un puente práctico entre teoría de estabilidad y restricciones de un diseño planar simplificado. citeturn2search16  
Relevancia stickman/simulación: altísima (bípede 2D con pies puntuales es el baseline más común en motores 2D).  
Nivel de utilidad: **muy útil**.  
Enlace primario: citeturn2search16

**Generalizations of the Capture Point to Nonlinear Center of Mass Paths and Uneven Terrain**  
Autores: Oscar E. Ramos; Kris Hauser  
Año: 2015  
Tipo: paper (PDF)  
Resumen: Generaliza el CP clásico a trayectorias de CoM no lineales y terreno no plano. Aunque tu stickman es 2D, este trabajo es útil si quieres salir del “CoM horizontal constante” y mantener formalismo cuando añadas alturas, terrenos o constraints más complejos. citeturn2search15  
Relevancia stickman/simulación: media-alta; más útil “a futuro” cuando agregues terrenos/variaciones.  
Nivel de utilidad: **útil pero secundario**.  
Enlace primario: citeturn2search15

**Stabilization of the Capture Point Dynamics for Bipedal Walking Based on Model Predictive Control**  
Autores: Manuel Krause; et al.  
Año: 2012  
Tipo: paper (IFAC; ficha editorial)  
Resumen: Usa MPC sobre dinámicas del capture point para incorporar restricciones (p. ej., ZMP) de forma explícita. Útil si tu stepping recovery evoluciona a un regulador con constraints (support polygon, límites de torque). citeturn10search13  
Relevancia stickman/simulación: media; valioso si quieres formalizar constraints del soporte y transiciones.  
Nivel de utilidad: **útil pero secundario**.  
Enlace primario: citeturn10search13

**Three-dimensional bipedal walking control based on Divergent Component of Motion (DCM)**  
Autores: Johannes Englsberger; Christian Ott; Alin Albu-Schäffer  
Año: 2015  
Tipo: paper (IEEE; ficha)  
Resumen: Extiende DCM/“capture point” a 3D e introduce conceptos (VRP/eCMP) muy usados en locomoción moderna. Para 2D, no necesitas la complejidad completa, pero sirve como referencia del “endpoint” del enfoque CP/DCM en robótica avanzada. citeturn0search10turn0search17  
Relevancia stickman/simulación: secundaria (más 3D), pero útil como guía conceptual si un día escalas.  
Nivel de utilidad: **útil pero secundario**.  
Enlace primario: citeturn0search10

---

## IK planar para bípedos 2D y reconstrucción de postura desde CoM, contactos y support constraints

Este bloque responde a tu “puente”: **objetivos centroidales + contactos/soporte → targets articulares**.

**Inverse Kinetics for Center of Mass Position Control and Posture Optimization**  
Autores: Ronan Boulic; Ramon Mas; Daniel Thalmann  
Año: 1994  
Tipo: paper técnico (PDF EPFL)  
Resumen: Extiende IK incorporando distribución de masas para controlar directamente el **centro de masa** y optimizar postura bajo criterios de balance “estático” y realismo postural. Aunque es anterior a técnicas QP modernas, es una pieza fundacional para “CoM como objetivo” dentro de IK. citeturn12search3  
Relevancia stickman/simulación: alta; especialmente si quieres una reconstrucción “posture-aware” desde CoM en un modelo articulado.  
Nivel de utilidad: **muy útil**.  
Enlace primario: citeturn12search3

**An Inverse Kinematic Architecture Enforcing an Arbitrary Number of Strict Priority Levels**  
Autores: Paolo Baerlocher; Ronan Boulic  
Año: 2001  
Tipo: paper (PDF)  
Resumen: Presenta una arquitectura de IK con **múltiples niveles de prioridad**, que es exactamente la idea detrás de “stack-of-tasks”: tareas de alto nivel (pies/contactos) no pueden ser violadas por tareas de menor prioridad (postura estética). Esto es directamente aplicable a tu reconstrucción desde CoM/contactos/soporte. citeturn12search22  
Relevancia stickman/simulación: altísima como plantilla: (1) contactos/pies, (2) CoM, (3) torso, (4) postura preferida.  
Nivel de utilidad: **muy útil**.  
Enlace primario: citeturn12search22

**Hierarchical Quadratic Programming: Fast Online Humanoid Whole-Body Control**  
Autores: Alexandre Escande; et al.  
Año: 2014  
Tipo: paper (IJRR; PDF)  
Resumen: Propone HQP para control/IK de cuerpo completo con **múltiples constraints y contactos**, resolviendo prioridades de tareas eficientemente. Aunque el target original es humanoide 3D, el patrón se translada a 2D: tu “solver de postura” puede ser un QP con constraints de contacto y límite articular. citeturn3search2  
Relevancia stickman/simulación: alta; útil si tu stickman crece a múltiples contactos (dos pies, mano en suelo, etc.) y necesitas robustez.  
Nivel de utilidad: **muy útil**.  
Enlace primario: citeturn3search2

**Whole-body Motion Integrating the Capture Point in the Operational-Space Inverse Dynamics Control Framework**  
Autores: Oscar E. Ramos; et al.  
Año: 2014  
Tipo: paper (PDF)  
Resumen: Integra explícitamente CP dentro de un marco de control por tareas (operational-space inverse dynamics): si el balance se degrada, se determina un buen lugar para pisar y se controla CP como tarea/constraint. Es casi tu arquitectura deseada “equilibrio → reconstrucción → control”. citeturn0search15turn3search4  
Relevancia stickman/simulación: alta; especialmente si quieres que stepping recovery y postura estén en el mismo marco de tareas.  
Nivel de utilidad: **muy útil**.  
Enlace primario: citeturn0search15

**Whole-Body Dynamic Control: A Task-Based Approach in the Operational Space**  
Autores: entity["people","Oussama Khatib","robotics researcher"]  
Año: 2004  
Tipo: paper (PDF Stanford)  
Resumen: Formaliza control de cuerpo completo por tareas en espacio operacional y cómo desacoplar objetivos de tarea y postura. Es una base conceptual para “CoM/feet/trunk tasks” y para entender por qué la postura no debe ser un afterthought. citeturn12search1  
Relevancia stickman/simulación: media-alta; más útil para diseñar una jerarquía de tareas coherente que para código 2D directo.  
Nivel de utilidad: **útil pero secundario**.  
Enlace primario: citeturn12search1

**Learning Feasibility Constraints for Multi-contact Locomotion**  
Autores: Justin Carpentier; et al.  
Año: 2017  
Tipo: paper (PDF RSS)  
Resumen: Plantea cómo incorporar restricciones de factibilidad del CoM de un modelo completo en optimización/planificación reducida. Para ti, el valor está en el “warning”: no todo objetivo de CoM y contactos es cinemáticamente alcanzable; necesitas un chequeo (o aprendizaje) de factibilidad. citeturn3search20  
Relevancia stickman/simulación: alta si tu pipeline empieza a planificar pasos/CoM sin considerar límites reales de pierna 2 eslabones.  
Nivel de utilidad: **útil pero secundario**.  
Enlace primario: citeturn3search20

**Inverse kinodynamics: Editing and constraining kinematic trajectories with dynamics**  
Autores: Peter G. Kry; et al.  
Año: 2012  
Tipo: paper (Computer Graphics Forum; ficha)  
Resumen: Propone un workflow “animator-friendly” que respeta constraints espacio-temporales incorporando dinámica de forma encapsulada. Es relevante como puente entre animación e ingeniería: te muestra cómo imponer constraints (contactos, objetivos) sin perder estabilidad física. citeturn12search13  
Relevancia stickman/simulación: útil si tu objetivo es “pose reconstruction” que se vea animada pero siga siendo física.  
Nivel de utilidad: **útil pero secundario**.  
Enlace primario: citeturn12search13

---

image_group{"layout":"carousel","aspect_ratio":"16:9","query":["capture point humanoid push recovery diagram","whole-body inverse kinematics contact constraints center of mass","stable PD controller articulated character simulation","support polygon heel toe walking diagram"] ,"num_per_query":1}

---

## Active ragdoll, joint PD control y control físico de personajes en motores de cuerpos rígidos

Aquí la prioridad es: PD estable, controladores de locomoción física robustos, y técnicas híbridas “animación/targets + simulación”.

**Stable Proportional-Derivative Controllers**  
Autores: entity["people","Jie Tan","computer graphics researcher"]; Karen Liu; Greg Turk  
Año: 2011  
Tipo: paper (IEEE Computer Graphics and Applications; PDF)  
Resumen: Propone **Stable PD (SPD)**: una formulación PD que permite ganancias altas incluso con timesteps grandes, al calcular torques considerando el estado del próximo paso. Está explícitamente motivado por simulación física de personajes y tracking de pose en motores discretos. citeturn11search9turn11search5turn11search14  
Relevancia stickman/simulación: altísima; si tu control de articulaciones se vuelve inestable con dt del engine, SPD es una referencia directa.  
Nivel de utilidad: **muy útil**.  
Enlace primario: citeturn11search5

**Linear Time Stable PD Controllers for Physics-based Character Control**  
Autores: Zhaoming Yin; et al.  
Año: 2020  
Tipo: paper (Computer Graphics Forum; PDF)  
Resumen: Propone un algoritmo **lineal en tiempo** para computar SPD en sistemas articulados (apoyándose en formulaciones eficientes de dinámica de articulaciones). Es especialmente útil si tu stickman crece en DOFs y necesitas SPD sin coste prohibitivo. citeturn4search7turn4search16  
Relevancia stickman/simulación: alta; útil si implementas tu propio solver de torques o necesitas rendimiento.  
Nivel de utilidad: **muy útil**.  
Enlace primario: citeturn4search7

**SIMBICON: Simple Biped Locomotion Control**  
Autores: KangKang Yin; Kevin Loken; Michiel van de Panne  
Año: 2007  
Tipo: paper (SIGGRAPH; PDF)  
Resumen: Controlador físico de bípedo con pocos parámetros (“authorable”) que genera variedad de gaits y soporta perturbaciones. Aunque tú no quieras más inverted pendulum “general”, SIMBICON es clave porque es una receta práctica de locomoción física con controladores y switching. citeturn9search2turn9search13  
Relevancia stickman/simulación: muy alta; es el antecedente más directo de muchos controladores de bípedos “jugables”.  
Nivel de utilidad: **muy útil**.  
Enlace primario: citeturn9search2

**Generalized Biped Walking Control**  
Autores: Stelian Coros; Philippe Beaudoin; Michiel van de Panne  
Año: 2010  
Tipo: paper (ACM TOG / SIGGRAPH; PDF de UBC)  
Resumen: Integra tracking (PD), foot placement y correcciones para robustez, apuntando a generalización a estilos, proporciones y habilidades. Es muy útil como blueprint de un controlador híbrido de personaje físico (cercano a “active ragdoll bien hecho”). citeturn11search2turn11search10turn11search19  
Relevancia stickman/simulación: altísima; te da estructura de controlador “real-time + robust”.  
Nivel de utilidad: **muy útil**.  
Enlace primario: citeturn11search2

**Composite Control of Physically Simulated Characters**  
Autores: Uldarico Muico; Jovan Popović; Zoran Popović  
Año: 2011  
Tipo: paper (ACM TOG; PDF de autor)  
Resumen: Aborda un problema clave: el tracking de una sola trayectoria puede ser de alta calidad pero falla ante grandes perturbaciones. Propone control compuesto para mejorar robustez y transiciones. Es muy relevante para recovery (stumble, re-stabilize) y para mezclar comportamientos. citeturn9search3turn9search7turn9search22  
Relevancia stickman/simulación: alta; útil si quieres un sistema de “skills” (caminar, recuperar, levantarse) con transiciones robustas.  
Nivel de utilidad: **muy útil**.  
Enlace primario: citeturn9search3

**Ragdoll Matching**  
Autores: (tesis)  
Año: 2024  
Tipo: tesis (PDF)  
Resumen: Trabajo aplicado que describe el “active ragdoll controller” como seguimiento de posición/orientación de huesos objetivo mediante fuerzas/torques del motor físico. Es menos “seminal” pero cercano a implementación de pipeline (targets → fuerzas). citeturn4search0  
Relevancia stickman/simulación: útil si necesitas detalles prácticos de “cómo se arma” un active ragdoll en un engine.  
Nivel de utilidad: **útil pero secundario**.  
Enlace primario: citeturn4search0

---

## Modelos de contacto del pie, base de soporte, heel/toe contact y support polygon

En 2D, el mayor “trade-off” es: **pie puntual** (simple, pero recovery menos realista y contacto más frágil) vs **pie segmentado/soporte extendido** (más realista, pero requiere modelar heel/toe, CoP y contactos múltiples).

**Modelling the effect of ‘heel to toe’ roll-over contact on the dynamics of biped robots**  
Autores: P. Mahmoodi; et al.  
Año: 2013  
Tipo: paper (Elsevier; ficha)  
Resumen: Modela explícitamente el contacto “heel-to-toe roll-over” y estudia su influencia en la dinámica del bípedo. Es útil para justificar por qué un pie segmentado y la transición heel→toe cambian estabilidad y naturalidad del gait. citeturn5search0  
Relevancia stickman/simulación: alta si planeas pasar de “point-foot” a un pie con segmento y eventos (heel strike, toe-off).  
Nivel de utilidad: **muy útil**.  
Enlace primario: citeturn5search0

**Human-Like Walking with Heel Off and Toe Support for Biped Robot**  
Autores: Y. Liu; et al.  
Año: 2017  
Tipo: paper (MDPI; open access)  
Resumen: Presenta un control híbrido con fases de soporte talón/toe y transición (incluyendo fase subactuada), buscando reproducir características humanas. Es relevante como ejemplo de cómo estructurar la máquina de estados de contacto cuando introduces heel/toe. citeturn5search2  
Relevancia stickman/simulación: media-alta; útil como referencia de FSM de contacto y de por qué toe-support importa.  
Nivel de utilidad: **útil pero secundario**.  
Enlace primario: citeturn5search2

**Postural Stability of Biped Robots and the Foot-Rotation Indicator (FRI) Point**  
Autores: Ambarish Goswami  
Año: 1999  
Tipo: paper (IJRR; DOI + copias PDF)  
Resumen: Introduce el **Foot Rotation Indicator** para analizar cuándo el pie tenderá a rotar durante soporte simple, conectando estabilidad con distribución de fuerzas y geometría del pie. Es una pieza clásica para razonar sobre “¿puede el apoyo ser puntual o necesito base de soporte?”. citeturn10search11turn10search3turn10search15  
Relevancia stickman/simulación: alta; te da un criterio para decidir si un pie puntual te limita o si necesitas controlar la rotación del pie/torso.  
Nivel de utilidad: **muy útil**.  
Enlace primario: citeturn10search11

**Box2D documentation: Overview + FAQ**  
Autores: entity["people","Erin Catto","physics engine developer"]  
Año: 2025 (Box2D 3.1.0 en la documentación consultada)  
Tipo: documentación oficial  
Resumen: Box2D se presenta como simulación rígida 2D para juegos y explícitamente lo enmarca como “sistema de animación procedural” desde la perspectiva de un engine, además de describir su alcance (colisiones/queries/BVH). También documenta que la versión 3.1.0 está en C17 y su uso en juegos/engines. citeturn9search0turn9search1turn9search12  
Relevancia stickman/simulación: alta por tu contexto: problemas de contactos/jitter/fixtures y el modelo de joints/motors condicionan tu diseño de pie (point-foot vs segment).  
Nivel de utilidad: **muy útil**.  
Enlace primario: citeturn9search0

**BipedalWalker (entorno Box2D de Gymnasium)**  
Autores: proyecto Gymnasium (documentación)  
Año: s. f. (docs vivas)  
Tipo: documentación de entorno/simulación  
Resumen: Describe un bípedo 2D con 4 articulaciones en un entorno basado en Box2D, con variantes de terreno. Es útil como “baseline aplicado”: muestra qué sensado/acciones se consideran mínimos y ofrece un punto de comparación (incluye contacto de piernas con el suelo y dinámica simplificada). citeturn8search0  
Relevancia stickman/simulación: media; no es un paper de control, pero es un ejemplo “Box2D-like biped” ampliamente usado.  
Nivel de utilidad: **útil pero secundario**.  
Enlace primario: citeturn8search0

Nota práctica: problemas como “foot contact jitter” y “ragdoll jitter/no sleep” son comunes en engines tipo Box2D cuando hay constraints rígidos + contactos persistentes; no es literatura académica, pero aparece frecuentemente como síntoma de tuning (solver iterations, damping, friction, joint limits). citeturn8search6turn9search0

---

## Levantarse desde el suelo y validación del realismo físico/biomecánico

### Get-up y recuperación post-caída

**Reliable Standing-up Routines for a Humanoid Robot**  
Autores: Jan Stückler; et al.  
Año: 2009  
Tipo: paper (PDF)  
Resumen: Describe métodos para levantarse desde posturas **prone y supine**, con énfasis en rutinas fiables y con un número limitado de DOFs, usando simulación para analizar. Es de las referencias más “directas” para tu bloque get-up porque está orientado a “procedimientos robustos”, no solo a locomoción. citeturn6search15  
Relevancia stickman/simulación: alta; aunque sea 3D, la idea de secuencias por fases + contactos se aplica a 2D (roll → apoyar → empujar → ponerse de pie).  
Nivel de utilidad: **muy útil**.  
Enlace primario: citeturn6search15

**Rising Motion Controllers for Physically Simulated Characters**  
Autores: Benjamin J. Jones  
Año: 2011  
Tipo: tesis (PDF)  
Resumen: Se centra explícitamente en controladores para **recuperarse de caídas** y generar movimientos de levantarse en personajes simulados físicamente. Es relevante porque su framing es “cuando incluso un buen balance controller falla, necesitas un recovery controller”. citeturn6search3turn6search6  
Relevancia stickman/simulación: altísima; es casi exactamente tu caso “stickman se cae → necesita get-up”.  
Nivel de utilidad: **muy útil**.  
Enlace primario: citeturn6search3

**Falling and Landing Motion Control for Character Animation**  
Autores: entity["people","Sehoon Ha","computer graphics researcher"]; et al.  
Año: 2012  
Tipo: paper (PDF)  
Resumen: Genera caídas y aterrizajes ágiles en tiempo real mediante simulación física, y explícitamente incluye rolling y “volver a ponerse en pie” dentro de un controlador general. Útil para inspirar controladores de recuperación que no dependen de una única postura inicial. citeturn4search14  
Relevancia stickman/simulación: alta; sirve para diseñar recoveries físicos “directables” y robustos.  
Nivel de utilidad: **muy útil**.  
Enlace primario: citeturn4search14

**Synthesizing Get-Up Motions for Physics-based Characters**  
Autores: (ver ficha editorial; equipo de autores no incluido en snippet)  
Año: 2023  
Tipo: paper (Computer Graphics Forum; DOI)  
Resumen: Propone aprender/sintetizar controladores de get-up capaces de levantarse desde configuraciones caídas arbitrarias en distintos terrenos y estilos. Es relevante si te interesa un get-up “general”, no un script fijo. citeturn6search0  
Relevancia stickman/simulación: media-alta; puede inspirar tu diseño incluso si no haces RL.  
Nivel de utilidad: **útil pero secundario**.  
Enlace primario: citeturn6search0

**Learning Humanoid Standing-up Control (RSS 2025)**  
Autores: (ver paper RSS)  
Año: 2025  
Tipo: paper (Robotics: Science and Systems; PDF)  
Resumen: Enfocado en aprendizaje de standing-up robusto; útil como estado del arte reciente (especialmente si más tarde quieres transferir técnicas RL a tu pipeline). citeturn6search12  
Relevancia stickman/simulación: secundaria a corto plazo si buscas implementación clásica; relevante a largo plazo.  
Nivel de utilidad: **útil pero secundario**.  
Enlace primario: citeturn6search12

### Métricas y métodos para validar realismo físico/biomecánico de locomoción simulada

**The condition for dynamic stability (XCoM / Margin of Stability)**  
Autores: A. L. Hof; et al.  
Año: 2005  
Tipo: paper (PDF)  
Resumen: Introduce el concepto de **extrapolated center of mass (XCoM)** y recomienda el **margin of stability (MoS)** como distancia mínima del XCoM a los bordes de la base de soporte. Es una métrica clásica, fácil de computar y muy útil como señal cuantitativa de estabilidad en marcha. citeturn7search10turn7search14  
Relevancia stickman/simulación: alta para validación; puedes comparar MoS de tu stickman con valores plausibles o usarlo para detectar “casi-caídas”.  
Nivel de utilidad: **muy útil**.  
Enlace primario: citeturn7search10

**Notes on the margin of stability**  
Autores: C. Curtze  
Año: 2024  
Tipo: paper (Elsevier; ficha)  
Resumen: Revisa y aclara el concepto de MoS/XCoM, su interpretación y matices. Es útil para no cometer errores típicos al usar MoS como “única métrica” y para entender cuándo falla. citeturn7search6turn7search22  
Relevancia stickman/simulación: alta; si adoptas MoS, esta nota te ayuda a usarlo bien.  
Nivel de utilidad: **muy útil**.  
Enlace primario: citeturn7search6

**A Literature Review on Virtual Character Assessment**  
Autores: E. A. Ursu; et al.  
Año: 2012  
Tipo: informe/review (PDF)  
Resumen: Revisión sobre cómo evaluar personajes virtuales (incluyendo señales biomecánicas/perceptuales), útil como “meta-mapa” de métricas: desde trayectorias (CoM) hasta criterios perceptuales. Es especialmente relevante para tu “Validation_Principles” futuro. citeturn7search13  
Relevancia stickman/simulación: alta a futuro; te ayuda a elegir métricas sin reinventar el área.  
Nivel de utilidad: **útil pero secundario** (para ahora), **muy útil** (para validación futura).  
Enlace primario: citeturn7search13

**Center of pressure progression (biomecánica / CoP)**  
Ejemplos (open access): estudios con análisis de trayectoria de **CoP** durante la fase de apoyo y cómo varía con patrones de marcha. citeturn5search5turn5search18  
Relevancia stickman/simulación: útil si modelas pie segmentado (heel/toe) y quieres comparar progresión de CoP o al menos evitar patrones no plausibles.  
Nivel de utilidad: **útil pero secundario**.

---

### Conjunto mínimo de métricas recomendadas para tu dossier

Si quieres un set cuantitativo “poco costoso” pero informativo (para comparar versiones de controlador/engine), estas métricas se apoyan bien en la bibliografía y encajan en 2D:

- **MoS/XCoM** para estabilidad dinámica. citeturn7search10turn7search6  
- Eventos de contacto y secuencia heel→toe si modelas pie segmentado; o, si no, duración/consistencia de stance vs swing. citeturn5search0turn5search2  
- Trayectoria de CoM por ciclo y su variabilidad entre ciclos (consistencia). citeturn7search9  
- Coste de control (energía/torque integrado) como proxy de “movimiento forzado” vs “natural”. (En control físico, suele correlacionar con rigidez artificial). citeturn11search5turn11search2  

---

## Palabras clave de búsqueda afinadas para cada bloque

```text
Capturability / push recovery / stepping
  "capture point" Pratt Goswami 2006 pdf
  "N-step capturability" Koolen Pratt IJRR 2012 pdf
  "humanoid push recovery" Stephens Atkeson 2007 pdf
  "push recovery model predictive control" capture point biped pdf
  "foot placement estimator" Wight Kubica Wang 2008 pdf
  "planar biped point feet" foot placement 2012 pdf

IK planar + posture reconstruction desde CoM/contactos/constraints
  "inverse kinetics" center of mass posture optimization Boulic 1994 pdf
  "strict priority inverse kinematics" Baerlocher Boulic pdf
  "hierarchical quadratic programming" whole-body control contact constraints pdf
  "operational space inverse dynamics" stack of tasks contact constraints pdf

Active ragdoll / PD control
  "stable proportional-derivative controllers" Tan Liu Turk 2011 pdf
  "linear time stable PD" physics-based character control 2020 pdf
  SIMBICON 2007 pdf
  "generalized biped walking control" Coros van de Panne 2010 pdf
  "composite control physically simulated characters" Muico Popovic 2011 pdf

Foot contact models / support polygon / heel-toe
  "heel-to-toe rollover contact" biped robot dynamics 2013
  "foot rotation indicator" Goswami 1999
  "heel off toe support" biped robot 2017
  "center of pressure progression" walking stance phase

Get-up / fall recovery
  "standing-up routines" prone supine humanoid robot pdf
  "rising motion controllers" physically simulated characters 2011 pdf
  "get-up motions" physics-based characters 2023

Validation realism
  Hof 2005 "extrapolated center of mass" margin of stability pdf
  "notes on the margin of stability" Curtze 2024
  "virtual character assessment" review 2012 pdf
```

Estos prompts están diseñados para devolver PDFs primarios y evitar resultados genéricos (p. ej., “inverted pendulum” sin capturability/stepping). citeturn0search9turn11search5turn5search0turn6search15turn7search10