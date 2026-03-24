# BobTricks — Modèle d'état

## Vue d'ensemble

L'état de simulation comporte trois niveaux conceptuels avec des cycles de vie
et des règles d'ownership différents.

| Niveau | Struct | Propriétaire | Cycle de vie |
|---|---|---|---|
| Physique | `CMState` | `SimulationCore` | intégré à chaque tick |
| Locomotion + contact | `CharacterState` | `SimulationCore` | mis à jour à chaque tick |
| Géométrie de pose | `PoseState` (prévu) | `PoseController` (prévu) | dérivé à chaque tick |

`SimState` est un snapshot en lecture seule exposé au rendu, à la télémétrie et
aux outils de debug. Il n'est jamais écrit depuis l'extérieur de
`SimulationCore`.

## CMState

```cpp
struct CMState {
    Vec2 position;      // position monde du centre de masse (m)
    Vec2 velocity;      // (m/s)
    Vec2 acceleration;  // (m/s²) — définie à chaque tick avant intégration
};
```

Autoritatif. Le CM est la quantité physique primaire. Les autres positions sont
dérivées de lui.

## CharacterState — niveau 1 : mode locomoteur

État actuel dans le code :

```cpp
enum class LocomotionState {
    Standing,   // deux pieds plantés, CM récupérable
    Walking,    // séquence de pas active
    Airborne,   // CM au-dessus du seuil de contact du ressort
    // Falling retiré — le step planner gère sa propre récupération
};
```

Extension prévue par le plan (Bloque 5A) :

```cpp
enum class LocomotionMode {
    Standing, Walking, Running,
    Jumping,    // phase de vol volontaire
    Falling,    // non contrôlé — sous-phases : instabilité aérienne → chute → sol
    GettingUp,  // transition procédurale du sol vers Standing
};

enum class PostureMode {
    Normal,
    Crouching,  // modificateur orthogonal — compatible avec Standing et Walking
};
```

`PostureMode` est orthogonal à `LocomotionMode`. Un personnage peut être
`Walking + Crouching` sans que `Crouching` soit une valeur du régime principal.

Les transitions entre valeurs de `LocomotionMode` auront des préconditions
physiques explicites. Elles doivent être écrites dans `LOCOMOTION_SPEC.md`
avant implémentation et appliquées par `LocomotionCoordinator` quand ce module
sera introduit.

## CharacterState — niveau 2 : support et contact

```cpp
struct SupportState {
    bool   left_planted;
    bool   right_planted;
    double support_width;    // distance entre les pieds plantés (m)
    double support_center_x; // milieu entre les pieds plantés (m)
};
```

Dérivé. Toujours reconstruit à partir de `FootState` par `updateSupportState()`.
Jamais écrit directement. Le type `double` est conservé partout pour rester
cohérent avec le reste du core.

```cpp
struct FootState {
    Vec2      pos;       // position monde actuelle
    double    ground_y;  // hauteur du terrain à l'abscisse du pied
    FootPhase phase;     // Planted ou Swing
};
```

## CharacterState — niveau 3 : géométrie de pose (état actuel)

Aujourd'hui, la pose est calculée dans `updateCharacterState()` et stockée
directement dans `CharacterState` :

- `pelvis`, `torso_center`, `torso_top` — chaîne de pose principale
- `knee_left`, `knee_right` — résultat de l'IK
- `foot_left_eff`, `foot_right_eff` — pieds clampés utilisés par l'IK et le rendu

État visé par le plan (Bloque 5A) :
- ces champs migrent vers un `PoseState` produit par `PoseController`
- la tête et les bras y sont ajoutés comme secondary motion procédurale
- l'état interne de lissage du controller reste privé et n'apparaît pas dans
  `SimState`

## SimState — le snapshot

État actuel :

```cpp
struct SimState {
    CMState        cm;
    CharacterState character;
    double         sim_time = 0.0;
};
```

`SimState` est l'unité d'observation pour le rendu, la télémétrie et les outils
de debug. C'est aussi l'unité de sérialisation pour l'historique de step-back et
les snapshots de scénarios.

Le plan prévoit d'y ajouter `PoseState` lorsque `PoseController` sera introduit.

## AppConfig — paramètres

`AppConfig` est un agrégat plat de structs de paramètres, un par sous-système :

`SimLoopConfig`, `CameraConfig`, `CharacterConfig`,
`CharacterReconstructionConfig`, `CMConfig`, `PhysicsConfig`,
`TerrainConfig`, `StandingConfig`, `BalanceConfig`, `StepConfig`, `WalkConfig`

`AppConfig` est chargé depuis `data/config.ini` au démarrage et sauvegardé sur
demande. `SimulationCore` conserve une référence non propriétaire vers
`AppConfig` : l'application SDL peut donc muter les paramètres en direct via
ImGui sans redémarrer la simulation.

Tous les paramètres réglables, y compris ceux de debug, doivent vivre dans
`AppConfig`, pas sous forme de constantes magiques dispersées dans le code.

## Sémantique de reset

`SimulationCore::reset(ScenarioInit)` remet :

- `cm.position     = init.cm_pos`
- `cm.velocity     = init.cm_vel`
- `cm.acceleration = {0, 0}`
- `character       = CharacterState{}` (`feet_initialized = false` par défaut)
- `character.facing     = sign(init.cm_vel.x)`
- `character.facing_vel = init.cm_vel.x`
- `character.last_heel_strike_t = -1.0` (`-1 = never`)
- `sim_time        = 0.0`
- `terrain         = régénéré si init.terrain_seed != 0`

Les pieds sont bootstrapés au premier `step()` après le reset.

## Modes de ScenarioInit

### Initialisation partielle (actuelle, par défaut)

On spécifie `cm_pos`, `cm_vel`, `terrain_seed`. Les pieds sont bootstrapés
automatiquement au premier step.

### Initialisation fully explicit (prévue, Bloque 3)

On spécifie tous les champs, y compris positions et phases des pieds, facing,
mode locomoteur et step plan actif. Les invariants suivants doivent tenir ; un
helper de validation doit signaler les violations avant `runScenario()` :

- un pied avec `phase == Planted` doit satisfaire
  `foot.pos.y == terrain.height_at(foot.pos.x)` à la tolérance près ;
- si `step_plan.active`, le pied swing et le pied d'appui doivent être cohérents
  avec `step_plan.move_right` ;
- `SupportState` est toujours recalculé depuis `FootState` et n'est jamais défini
  directement ;
- le `LocomotionMode` doit être compatible avec la vitesse du CM et l'état de
  contact.

## Télémétrie

`TelemetryRecorder` enregistre une `TelemetryRow` par tick de simulation. Le jeu
de colonnes CSV est figé et versionné :

```text
t, cm_x, cm_vx, cm_y, cm_vy, pelvis_x, xcom, mos,
support_left, support_right, support_width,
foot_L_x, foot_R_x, step_active, swing_right, trigger, loco_state
```

Ces colonnes ne doivent pas changer sans mise à jour explicite du marqueur de
version dans `TelemetryRow.h`. Les assertions de scénarios s'exécutent après la
fin d'un run, sur l'ensemble des lignes enregistrées.

## Sérialisation de snapshot

Le plan prévoit un helper texte qui sérialise `SimState` dans un bloc lisible
par humain. Son but est de capturer des états intéressants depuis le debugger
ImGui et de les reproduire comme conditions initiales de scénarios.

Ce n'est pas un format de persistance officiel. Aucune compatibilité inter-version
n'est promise.

## Flags événementiels par tick dans CharacterState

Ces champs sont réinitialisés au début de chaque `step()` puis définis pendant
le tick courant :

```cpp
StepTriggerType last_trigger          = StepTriggerType::None;
bool            heel_strike_this_tick = false;
```

Ils ne sont valables que pour le tick où ils sont émis. Les renderers et la
télémétrie les lisent après le retour de `step()` ; ils ne doivent pas être
mis en cache d'un tick à l'autre.
