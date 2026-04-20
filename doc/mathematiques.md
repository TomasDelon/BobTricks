# Mathématiques de BobTricks

Ce document détaille toutes les formules utilisées dans la simulation,
dans l'ordre où elles apparaissent dans le pipeline de `SimulationCore::step()`.

---

## 1. Géométrie du squelette

### Unité de longueur L

Toute la géométrie du personnage est exprimée en multiples de **L**, la
longueur d'un segment de membre :

```
L = body_height_m / 5
```

Pour un personnage de 1,80 m : L = 0,36 m.
Les cinq segments sont : cuisse, tibia, torse bas, torse haut, tête — chacun vaut L.

### Hauteur nominale du centre de masse

Le CM est à hauteur `h_nominal` au-dessus du sol quand le personnage est en
posture préférée (pieds écartés de `d_pref × L`) :

```
h_pelvis  = sqrt( (2L)² − (d_pref·L / 2)² )     [Pythagore, triangle isocèle]
h_nominal = h_pelvis + cm_pelvis_ratio · L
```

Avec les valeurs par défaut (d_pref = 0,90 ; ratio = 0,75) :
`h_nominal ≈ 2,70 L ≈ 0,972 m` pour un personnage de 1,80 m.

> **Pourquoi pas `(2 + ratio)·L` ?** Cette formule suppose les jambes
> parfaitement verticales (`d_pref = 0`). Avec un écartement non nul, le
> bassin descend de Pythagore et la formule naïve place le CM trop haut,
> ce qui casse le critère de portée C4 du contrôleur debout.

### Position du bassin

Le bassin est décalé du CM par le ratio pelvis avec un angle de lean θ :

```
pelvis.x = cm.x − d · sin(θ)
pelvis.y = cm.y − d · cos(θ)
où  d = cm_pelvis_ratio · L
    θ = theta_max_deg · tanh(vx / v_ref)     [lean filtré]
```

Le `tanh` sature le lean à haute vitesse et donne une réponse douce autour de zéro.

---

## 2. Modèle de pendule inversé (IP)

Le CM est modélisé comme la masse d'un **pendule inversé** à jambes télescopiques.
Ce modèle simplifié (Linear Inverted Pendulum, LIP) prédit bien la marche humaine
à condition d'en respecter les hypothèses : CM à hauteur constante, jambe de
longueur variable.

### Intégration de la physique (Euler semi-implicite)

À chaque pas fixe `dt` :

```
1. Calcul de l'accélération horizontale :
   ax = input_accel − floor_friction · vx     (si touche maintenue)
   ax = −floor_friction · vx                  (glissement libre)

2. Intégration :
   vx += ax · dt
   x  += vx · dt

3. Composante verticale (contrainte douce tanh) :
   vy_want = vy_max · tanh( (y_tgt − y) / d_soft )
   vy      = vy + (vy_want − vy) · (1 − exp(−vy_tau · dt))
   y      += vy · dt
```

La contrainte verticale est un **attracteur non-linéaire** plutôt qu'un ressort
rigide. Le `tanh` évite les oscillations de Hooke et sature la correction à
`vy_max` quelle que soit l'erreur — le CM ne peut pas « tomber en chute libre »
vers sa cible.

### Setpoint vertical du CM

La hauteur cible `y_tgt` est la somme de plusieurs termes :

```
y_tgt = ref_ground
      + h_nominal             (hauteur nominale au-dessus du sol)
      − speed_drop            (abaissement à haute vitesse)
      − slope_drop            (abaissement en montée)
      − downhill_crouch · L   (accroupissement en descente)
      + cm_height_offset      (ajustement manuel)
      − jump_preload_depth    (enfoncement avant saut)
```

Chaque terme est détaillé dans `AppConfig::WalkConfig`.

---

## 3. XCoM et capture point (Hof, 2008)

### Définition

Le **centre de masse extrapolé** (XCoM, noté ξ) prédit où le CM s'arrêtera
si aucune nouvelle force n'est appliquée :

```
ω₀ = sqrt(g / h_ref)          [fréquence propre du pendule à hauteur h_ref]
ξ  = x_cm + ẋ_cm / ω₀        [XCoM = position + vitesse normalisée]
```

`h_ref` est la hauteur courante du CM au-dessus du centre du polygone de support.

### Marge de stabilité (MoS)

```
MoS = min( ξ − x_gauche,  x_droite − ξ )
```

- MoS > 0 : le CM peut théoriquement s'arrêter seul au-dessus du support.
- MoS < 0 : un pas est nécessaire avant de tomber.

### Déclenchement des pas

Deux types de déclencheurs coexistent (`StepTriggerType`) :

| Type | Condition | Signification |
|------|-----------|---------------|
| **Normal** | pied arrière trop loin du bassin : `|pelvis.x − stance.x| > d_rear_max · L` | Marche normale, gestion de la cadence |
| **Emergency** | ξ dépasse le pied avant : `ξ > front_foot.x` (ou `< ` si orienté gauche) | Récupération d'équilibre |

### Calcul de la cible d'atterrissage

En marche, le pied atterrit à :

```
x_target = ξ + facing · margin · L
```

où `margin = walk_cfg.stability_margin`. Ce placement garantit que le nouveau
polygone de support contient ξ avec une marge positive.

Sur pente, la portée horizontale effective est réduite par la géométrie :

```
max_reach_horizontal = 0.95 · reach_radius / sqrt(1 + slope²)
```

---

## 4. IK analytique de jambe à deux segments

### Problème

Données : position du bassin **P**, cible du pied **F**, longueur de segment **L**.
Cherché : position du genou **K**.

### Solution (loi des cosinus / triangle isocèle)

```
d = |PF|                        (distance bassin–pied)
d = clamp(d, ε, 2L − ε)        (éviter singularité extension totale)

h_k = sqrt( L² − (d/2)² )      (hauteur du genou au-dessus du milieu PF)

M = (P + F) / 2                 (milieu de PF)
e = (F − P) / d                 (vecteur unitaire PF)
n = rotate90(e)                 (perpendiculaire CCW)
n = −n  si  n.x · facing < 0   (forcer le genou vers l'avant)

K = M + h_k · n
```

Le genou se place toujours à la même distance (`h_k`) du milieu du segment PF,
sur la perpendiculaire orientée vers l'avant. C'est la seule solution
physiquement sensée pour une jambe humaine.

---

## 5. IK analytique de bras à deux segments

### Différence avec la jambe

Pour un bras, les deux segments ont des longueurs différentes
(`upper_len ≠ fore_len`), donc le problème n'est plus un triangle isocèle.

### Solution (formule générale)

```
dist = |target − shoulder|
a    = (upper² − fore² + dist²) / (2 · dist)   [projection de l'épaule sur l'axe]
h    = sqrt(upper² − a²)                        [hauteur du coude hors de l'axe]

mid  = shoulder + dir · a
perp = rotate90(dir)                            [perpendiculaire dans le plan]

coude = mid + signe · h · perp
```

Le signe (`+` ou `−`) est choisi en comparant les deux solutions à la
**préférence de pliure** (`bend_preference`) et au **coude précédent**
(stabilité temporelle).

---

## 6. Arc de swing (trajectoire du pied)

Le pied en vol suit une **parabole** entre sa position de décollage
`swing_start` et sa cible `swing_target`, paramétrée par `swing_t ∈ [0,1]` :

```
x(t) = lerp(swing_start.x, swing_target.x, t)
y(t) = lerp(swing_start.y, swing_target.y, t)
      + swing_h_clear · 4·t·(1−t)             [parabole, pic à t=0.5]
```

### Calcul de la hauteur de dégagement (`swing_h_clear`)

```
h_terrain = max(terrain.height_at(x_sample) − y_linear) sur 3 points intermédiaires
h_slope   = h_clear_slope_factor · L · (dy / dx)   [montée = plus de dégagement]
h_speed   = h_clear_speed_factor · L · (vx / vmax)

swing_h_clear = max(h_terrain + h_clear_min,
                    h_base + h_slope + h_speed)
```

### Ralentissement sur terrain pentu (`swing_speed_scale`)

Un pas avec dénivelé vertical important couvre une arc plus long que sa
projection horizontale. Le paramètre d'avancement est ralenti en proportion :

```
arc_len         = sqrt(dx² + dy²)
swing_speed_scale = max(0.30,  hdist / arc_len)
```

Ainsi, `swing_t` avance plus lentement pour les pas raides — le pied prend
naturellement plus de temps sur terrain incliné.

---

## 7. Référence de sol glissante

### Problème

Sur terrain accidenté, quelle hauteur de sol utiliser comme référence pour
le setpoint vertical du CM ? Utiliser le point exact sous le CM cause des
oscillations au passage de chaque bosse.

### Solution : fenêtre d'échantillonnage avec glissement exponentiel

Une fenêtre `[x_back, x_fwd]` centrée sur le bassin échantillonne le terrain
de chaque côté. Les extrémités de la fenêtre glissent vers leur cible avec
une constante de temps `tau_slide` :

```
x_back_target = pelvis.x − w_back · L   (arrière fixe)
x_fwd_target  = pelvis.x + (w_fwd + t_look · vx) · L   (avant + lookahead)

x_back += (x_back_target − x_back) · (1 − exp(−dt / tau_slide))
x_fwd  += (x_fwd_target  − x_fwd)  · (1 − exp(−dt / tau_slide))

ref_ground = (terrain.height_at(x_back) + terrain.height_at(x_fwd)) / 2
ref_slope  = (fwd.y − back.y) / (fwd.x − back.x)
```

Le lissage exponentiel élimine les discontinuités sans introduire de délai
notable à marche normale.

---

## 8. Mode course — modèle SLIP approximé

Le modèle **SLIP** (Spring-Loaded Inverted Pendulum) est le modèle biomécanique
de référence pour la course. BobTricks en utilise une approximation cinématique :

- Le CM suit un arc parabolique entre deux appuis (**phase de vol**).
- L'oscillation verticale est pilotée par `run_phase`, une phase continue
  qui avance à la cadence de la foulée.
- Le `run_blend` interpole continuellement marche ↔ course.

### Cadence et timing de foulée

```
cadence_spm = lerp(cadence_spm_min, cadence_spm_max, speed_ratio)   [162–176 spm]
step_period = moyenne( stride_len / (2·vx),  60 / cadence_spm )
contact_time = lerp(0.26, 0.22, speed_ratio)
flight_time  = step_period − contact_time
```

La durée de contact décroît avec la vitesse (biomécanique réelle : moins de
temps au sol = plus de rigidité de jambe).

---

## 9. Prédiction d'atterrissage (saut)

Pendant le vol, le simulateur prédit à chaque pas où le CM atterrira par
**intégration balistique en avant** :

```
x(t) = x₀ + vx · t
y(t) = y₀ + vy · t − ½·g·t²

Condition d'atterrissage :  pelvis_y(t) ≤ terrain.height_at(x(t)) + 2L
```

La recherche est linéaire avec un pas `dt_probe = 1/120 s`, max 240 itérations
(2 secondes). Les cibles de pieds sont recalculées à chaque frame pendant le vol
et convergent progressivement vers les positions finales via `smooth01` :

```
smooth01(x) = 3x² − 2x³     [courbe S — dérivée nulle aux extrémités]
```

---

## 10. Terrain procédural (méthode angle-walk)

Le terrain est généré par accumulation d'angles aléatoires :

```
angle_courant += delta_angle   [delta tiré aléatoirement dans {small, large}]
angle_courant  = clamp(angle_courant, −slope_max, +slope_max)

x_nouveau = x + segment_len · cos(angle_courant)
y_nouveau = y + segment_len · sin(angle_courant)
```

Les contraintes `height_min` / `height_max` reflètent l'angle vers zéro si la
hauteur s'approche des bornes. Le résultat est une polyligne continue avec une
pente bornée et une hauteur bornée, sans discontinuités de normale.

---

## 11. Courbes de Bézier (renderer spline)

### Bézier cubique (degré 3)

```
B(t) = (1−t)³·P0 + 3(1−t)²t·P1 + 3(1−t)t²·P2 + t³·P3
B'(t) = 3(1−t)²·(P1−P0) + 6(1−t)t·(P2−P1) + 3t²·(P3−P2)
```

Paramétrisation des membres :
- **Jambe** : P0 = bassin, P1/P2 = points de contrôle autour du genou, P3 = pied.
- **Bras** : idem avec épaule/coude/main.
- **Torse** : spline continue bassin → torse → tête.

Les courbes sont échantillonnées en `samples_per_curve` points (défaut : 24)
puis rendues par `StrokeRenderer` comme polyligne à largeur constante.

---

## Résumé des dépendances mathématiques

```
Terrain procédural
    └─> Référence de sol (sliding window)
            └─> Setpoint y_tgt du CM
                    └─> Intégration Euler IP
                            └─> XCoM / MoS (Hof 2008)
                                    └─> Déclencheur de pas
                                            └─> Cible d'atterrissage
                                                    └─> Arc de swing parabolique
                                                            └─> IK de jambe (loi des cosinus)
                                                                    └─> CharacterState (rendu)
```
