# BobTricks — Tests et validation

## Principe

La validation repose sur deux piliers complémentaires :

1. **Headless déterministe** — scénarios reproductibles avec assertions
   quantitatives sur la télémétrie. Automatisable, sans display, exit code.
2. **Validation visuelle manuelle** — comportement SDL, overlays, feel du
   personnage. Non automatisable ; documentée ici comme procédure.

Avant un push, la porte de qualité minimale devient :

```sh
make test
make check_architecture
```

`make test` couvre le pilier 1. Le pilier 2 est une étape obligatoire pour tout
changement qui affecte le rendu, la pose ou le comportement visible.

---

## Matrice de validation

| Type | Ce que ça couvre | Commande | Automatique |
|---|---|---|---|
| Build propre | Compilation sans warnings | `make build` | ✅ |
| Build headless | Core + config sans SDL | `make build_headless` | ✅ |
| Scénarios comportementaux | Comportement locomoteur quantitatif | `make test_headless` | ✅ |
| Invariants d'architecture | `src/core/` sans SDL/ImGui, hygiène dépôt | `make check_architecture` | ✅ |
| Tests unitaires | Fonctions mathématiques et modules isolés | `make test_unit` | ✅ |
| Tests de régression | Non-régression sur sorties de télémétrie | `make test_regression` | ✅ |
| Mémoire (Valgrind) | Fuites sur le binaire headless | `make test_mem` | ✅ (prévu Bloque 0) |
| Sanitizers (ASan/UBSan) | UB et accès mémoire invalides | `make build_asan` + `make test` | ✅ (prévu Bloque 0) |
| API docs | Génération Doxygen sans erreurs | `make docs` | ✅ (prévu Bloque 1) |
| Validation visuelle SDL | Comportement visible, pose, transitions | Procédure manuelle | ❌ |

---

## Scénarios comportementaux

### Structure d'un `ScenarioDef`

```cpp
struct ScenarioDef {
    std::string  name;
    double       duration_s;
    ScenarioInit init;

    std::function<InputFrame(double /*sim_time*/)> input_fn;
    std::function<void(TelemetryRecorder&)>        setup_asserts;
};
```

`input_fn` est appelée à chaque tick ; elle peut retourner un `InputFrame`
constant, une séquence temporisée, ou `{}` pour un scénario sans entrée.

`setup_asserts` est appelée une fois avant le run. Elle enregistre des
assertions sur le `TelemetryRecorder`.

### Ajouter un scénario

1. Ajouter une entrée dans `src/headless/ScenarioLibrary.cpp`.
2. Définir le `ScenarioDef` : nom, durée, `ScenarioInit`, `input_fn`,
   `setup_asserts`.
3. Vérifier que `make test` passe avec le nouveau scénario.

### Assertions disponibles (`TelemetryRecorder`)

Les assertions s'appliquent sur l'ensemble des lignes CSV après la fin du run.
Exemples de patterns actuels :

```cpp
// Nombre de heel-strikes dans une plage
rec.addAssertion("heel_strikes >= 4", [](const auto& rows) { … });

// Valeur d'une colonne sur toutes les lignes
rec.addAssertion("cm_y > -0.1", [](const auto& rows) { … });
```

L'assertion retourne `true` si elle passe. Le `ScenarioRunner` collecte les
résultats et retourne `false` si au moins une échoue.

## Format CSV de télémétrie

Colonnes fixes (ne pas modifier sans versionner dans `TelemetryRow.h`) :

```text
t, cm_x, cm_vx, cm_y, cm_vy, pelvis_x, xcom, mos,
support_left, support_right, support_width,
foot_L_x, foot_R_x, step_active, swing_right, trigger, loco_state
```

- `t` : temps simulation en secondes
- `xcom` : extrapolated centre of mass (XCoM, Hof 2008)
- `mos` : margin of support (positif = CM projeté dans le polygone de support)
- `trigger` : `None`, `Normal`, ou `Emergency`
- `loco_state` : valeur entière de `LocomotionState`

Pour inspecter un run :

```sh
./build/bobtricks_headless --scenario walk_3s > out.csv
# extraire les heel-strikes :
awk -F, '$16 != "None" {print NR, $1, $16}' out.csv
# vérifier que cm_y reste positif :
awk -F, 'NR>1 && $4 < 0 {print "FAIL ligne", NR, "cm_y=", $4}' out.csv
```

## Tests unitaires

Les tests unitaires couvrent des fonctions isolées avec des entrées précises et
des résultats attendus. Ils n'instancient pas `SimulationCore` et ne nécessitent
pas de scénario.

Candidats prioritaires :

- `LegIK` : configurations de jambe connues → positions de genou attendues
- `BalanceComputer` : CM et support donnés → XCoM et MoS attendus
- `StepPlanner::shouldStep` : configurations de pieds et CM connues → trigger ou non
- `StepPlanner::planStep` : configurations données → target dans les plages attendues
- Fonctions de `src/core/analysis/`

## Tests de régression

Un test de régression lance un scénario, capture les sorties de télémétrie et
les compare à des plages de référence. Les plages sont suffisamment larges pour
ne pas être fragiles aux micro-variations numériques.

Exemple de contrat :

```text
scénario walk_3s, seed 42 :
  heel_strikes ∈ [5, 9]
  cm_x_final  ∈ [2.0, 5.0]
  cm_y_min    > -0.05
```

Si un changement de la logique de locomotion fait sortir un résultat des plages,
le test de régression doit détecter l'écart.

## État initial détaillé des scénarios (prévu Bloque 3)

Structure cible : `DetailedScenarioInit`

Permet de spécifier au-delà de `cm_pos / cm_vel / terrain_seed` :

- position et phase (`Planted / Swing`) de chaque pied
- `facing`
- `LocomotionMode` initial
- `step_plan` actif si besoin

Un helper de validation vérifie la cohérence avant `runScenario()`. Voir
`doc/STATE_MODEL.md` — Modes de `ScenarioInit` pour les invariants.

## Validation visuelle : procédure manuelle

Pour tout changement affectant le rendu, la pose ou le comportement perceptible :

1. `make run`
2. Vérifier le comportement pour chaque régime affecté (`standing`, `walking`, etc.)
3. Vérifier les overlays de debug concernés (`XCoM`, support, step plan)
4. Vérifier l'absence d'artefacts visuels (pieds qui sautent, pose qui flip,
   trail incohérent)
5. Si le comportement observé contredit l'attendu, investiguer avant de clôturer

La validation visuelle ne remplace pas `make test`. Les deux sont obligatoires
pour les changements qui touchent au comportement.

## Performance

La télémétrie ne doit pas déformer les timings du core en mode normal. Le
check minimal :

- `make test` complet doit se terminer dans un temps raisonnable
  (seuil à fixer dans le Bloque 0 une fois la suite de tests stabilisée)
- en mode SDL, l'enregistrement de télémétrie n'est actif que dans les scénarios
  headless, pas dans le runtime interactif
