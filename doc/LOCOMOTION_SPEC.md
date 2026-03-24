# BobTricks — Spécification de locomotion

Ce document est la référence technique vivante de chaque régime locomoteur.

**Règle :** la spécification d'un régime doit être écrite ici avant que
l'implémentation ne commence. Quand un régime est implémenté, la section
correspondante reflète l'état réel du code, pas un plan.

Pour l'état d'avancement des phases, voir `doc/ROADMAP_locomotion.md`.  
Pour le modèle d'état et les invariants, voir `doc/STATE_MODEL.md`.

---

## Constantes et notations

```text
H  = body_height_m         (m)     — hauteur totale du personnage
L  = H / 5                 (m)     — unité de segment de jambe
g  = 9.81                  (m/s²)
ω₀ = sqrt(g / nominal_y)          — fréquence propre du pendule inversé
```

`nominal_y` est calculé par `computeNominalY(L, d_pref, cm_pelvis_ratio)` depuis
`src/core/physics/Geometry.h`. Il représente la hauteur nominale du CM
au-dessus du sol.

---

## Standing

### Description

Les deux pieds sont plantés. Le CM est récupérable sans déclencher un pas.

### Préconditions d'entrée

- `support.both_planted() == true`
- `|cm.velocity.x| <= standing.eps_v`
- `step_plan.active == false`

### Maintien

Une force de restauration douce ramène le CM vers le centre de support :

```text
k_restore = 0.5 * (g / nominal_y)
accel.x  += k_restore * (support.center_x - cm.position.x)
```

Le coefficient `0.5 * ω₀²` annule la moitié de l'instabilité du pendule
inversé → `standing` stable et silencieux.

### Sorties possibles

- → Walking : `|cm.velocity.x| > eps_v` ou déclenchement d'un pas
- → Jumping : entrée saut (prévu)
- → Falling : perte de support non récupérable (prévu)

---

## Walking

### Description

Séquence de pas alternés pilotée par un déclencheur géométrique unifié.
Pas de FSM Walking/Recovery distincte : le même déclencheur gère les deux.

### Trigger unifié (`StepPlanner`)

```text
rear_foot  = argmin_i( foot_i.pos.x * facing )
front_foot = l'autre pied

behind_dist = (pelvis.x - rear_foot.pos.x) * facing   ≥ 0

// Règle 1 — normale (nécessite du mouvement) :
normal_trigger    = |v_x| > eps_v   AND   behind_dist > k_trigger * L

// Règle 2 — urgence (CM hors de tout support) :
cm_reach          = cm.x + v_x / max(0.1, floor_friction)
outside_support   = (cm_reach - front_foot.pos.x) * facing > 0
emergency_trigger = outside_support   AND   behind_dist ≥ 0

fire = (normal_trigger OR emergency_trigger) AND !step_plan.active
```

Quand `fire` est vrai, c'est toujours le pied arrière qui part en swing.

### Paramètres (`WalkConfig`)

| Paramètre | Signification | Valeur par défaut |
|---|---|---|
| `k_trigger` | Derrière-dist en L déclenchant un pas | 0.70 |
| `k_step` | Reach de base du pied en L devant le CM | 0.90 |
| `T_ant` | Look-ahead de vitesse pour le target (s) | 0.15 |

### Target du pas (`planStep`)

```text
x_target = cm.x * facing + k_step * L + |v_x| * T_ant
```

Clamp dans `[front_foot.x + d_min * L,  front_foot.x + d_max_corr * L]`
(coordonnées monde, signé par `facing`). Puis clamp de reach (`I_reach`).

### Trajectoire swing (`evalSwingFoot`)

Arc Bézier quadratique paramétré par `u ∈ [0, 1]` :

```text
u      = (sim_time - plan.t_start) / plan.duration
swing  = Bezier(takeoff, midpoint, land_target, u)
```

Le point milieu est relevé de `h_clear_ratio * L` au-dessus de la ligne
droite `takeoff → land`.

### CM bobbing (phase 6)

Pendant un swing actif, la cible verticale du ressort est relevée :

```text
phi            = (sim_time - plan.t_start) / plan.duration   ∈ [0, 1]
A              = k_bob * L * tanh(|v_x| / v_ref_bob)
spring_target += A * sin(π * phi)
```

Amplitude nulle au décollage et à l'atterrissage ; maximale en mi-swing.

### Paramètres (`StepConfig`)

| Paramètre | Signification | Valeur par défaut |
|---|---|---|
| `h_clear_ratio` | Garde au sol du swing (× L) | 0.10 |
| `T_min` | Durée minimale du swing (s) | 0.18 |
| `T_max` | Durée maximale du swing (s) | 0.30 |
| `dist_coeff` | Croissance de durée par unité dist/L (s) | 0.20 |
| `d_max_correction` | Séparation max pour un pas correctif (× L) | 1.80 |
| `k_bob` | Amplitude max du bobbing (× L) | 0.10 |
| `v_ref_bob` | Vitesse de demi-saturation du bobbing (m/s) | 1.00 |

### Heel-strike et transition

Quand `u >= 1.0` :

- `swing.pos = plan.land_target`
- `swing.phase = Planted`
- `step_plan.active = false`
- `heel_strike_this_tick = true`
- `SupportState` recalculé immédiatement

### Validation mathématique

Les propriétés suivantes sont vérifiées analytiquement dans
`ROADMAP_locomotion.md` (révision 2) :

- A — pas de déclenchement au repos
- B — séparation des pieds en régime établi dans `[d_min, d_max]`
- C — cooldown naturel sans `T_paso_minimo`
- D — recovery d'urgence sans re-déclenchement infini
- E — taille du pas < `d_max_correction`
- F — vitesse terminale avec les paramètres courants

### Phases d'implémentation

Voir `doc/ROADMAP_locomotion.md` pour l'état des phases 1 à 6.

---

## Jumping

> Section réservée. Spec à écrire avant le début du Bloque 5B.

Points à définir :

- préconditions d'entrée (vitesse min, sol, délai post-landing)
- impulse verticale et gestion de `InputFrame.jump`
- détection de phase descendante vs ascendante
- condition d'atterrissage (seuil `cm.velocity.y` et hauteur terrain)
- transitions → `Standing` / `Walking` selon vitesse à l'atterrissage

---

## Falling

> Section réservée. Spec à écrire avant le début du Bloque 5B.

Points à définir :

- distinction `Jumping` vs `Falling` (intention vs perte de contrôle)
- sous-phases : instabilité aérienne → chute → sol (`grounded_after_fall`)
- détection de contact sol/corps (pas seulement CM)
- condition de transition vers `GettingUp`

---

## Running

> Section réservée. Spec à écrire avant le début du Bloque 5C.

Points à définir :

- phase de vol réelle (double unsupported)
- déclencheur de pas distinct du walking trigger
- dynamique du CM en running (SLIP-like vs autre modèle)
- transitions `Walking ↔ Running` (seuil de vitesse, continuité)

---

## Crouching

> Section réservée. Spec à écrire avant le début du Bloque 6.

Points à définir :

- implémentation comme `PostureMode::Crouching` (modificateur orthogonal)
- modification de `nominal_y` (hauteur CM cible réduite)
- compatibilité avec `Standing` et `Walking`
- contraintes sur la hauteur minimale du CM

---

## Getting-up

> Section réservée. Spec à écrire avant le début du Bloque 6.

Points à définir :

- précondition : `LocomotionMode::Falling`, sous-phase `grounded_after_fall`
- séquence procédurale : réorganisation support → montée CM/pelvis →
  stabilisation `Standing`
- durée et paramètres de la transition
- conditions d'échec (terrain trop incliné, espace insuffisant)

---

## Pose secondaire

> Section réservée. Spec à écrire lors du Bloque 5A.

Points à définir :

- driver de tête : offset de `torso_top`, cabeceo lié à `cm.velocity.y`
  et `LocomotionMode`
- driver de bras : swing opposé aux jambes, dérivé du cycle de pas
- comportement par régime (`walking`, `running`, `jumping`, `falling`)
- état interne de lissage dans `PoseController`
