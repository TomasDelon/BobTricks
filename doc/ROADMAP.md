# BobTricks — Feuille de route générale

Ce document recense les blocs de travail jusqu'à la fin avril et leur état
d'avancement. Il donne une vue d'ensemble du projet entier — pas seulement de la
locomotion.

Pour le détail de l'avancement des phases locomotrices, voir
`doc/ROADMAP_locomotion.md`.  
Pour la spécification technique des régimes, voir `doc/LOCOMOTION_SPEC.md`.

---

## Blocs de travail

| Bloc | Titre | État |
|---|---|---|
| 0 | Baseline et qualité | ✅ Terminé |
| 1 | Documentation vivante + structure de tests | 🔄 En cours |
| 2 | Consolidation structurelle bas risque | ⏳ Planifié |
| 3 | Infrastructure de validation avancée | ⏳ Planifié |
| 4 | Walking complet (phases 5 + 6) | ✅ Terminé |
| 5A | Généralisation structurelle | ⏳ Planifié |
| 5B | Jumping + Falling | ⏳ Planifié |
| 5C | Running | ⏳ Planifié |
| 6 | Crouching, Getting-up, pose secondaire, clôture | ⏳ Planifié |

---

## Bloque 0 — Baseline et qualité

Objectif : établir la ligne de base réelle avant tout autre travail.

- [x] `make test` passe proprement
- [x] `make build` sans warnings (`-Wall -Wextra`)
- [x] Cible `make build_asan` créée et verte sur le binaire headless
- [x] `make test_mem` (Valgrind) créé et vert sur le binaire headless
- [x] `src/input` retiré du Makefile (répertoire inexistant)
- [x] État exact de la phase 5 de locomotion documenté dans `ROADMAP_locomotion.md`
- [x] Décision sur `curves_lab` prise et documentée : incubateur temporaire, exclu du build principal

---

## Bloque 1 — Documentation vivante + structure de tests

Objectif : rendre explicite l'architecture existante et fixer la structure de
travail.

- [x] `README.md`
- [x] `doc/VISION.md`
- [x] `doc/ARCHITECTURE.md`
- [x] `doc/STATE_MODEL.md`
- [x] `doc/ROADMAP.md` (ce fichier)
- [x] `doc/COMMANDS_AND_MODES.md`
- [x] `doc/TESTING_AND_VALIDATION.md`
- [x] `doc/LOCOMOTION_SPEC.md` — spec walking complète, sections réservées pour
  les autres régimes
- [ ] `CLAUDE.md`
- [ ] Scaffold Doxygen minimal : `Doxyfile`, cible `make docs` fonctionnelle,
  page racine
- [ ] Structure `tests/unit/`, `tests/scenario/`, `tests/regression/` créée
- [ ] `doc/ARCHITECTURE_HEADLESS.md` archivé dans `doc/archive/`

---

## Bloque 2 — Consolidation structurelle bas risque

Objectif : corriger les classements incorrects sans changer le comportement.

- [x] `Camera2D` déplacé de `src/core/runtime/` vers `src/render/`
- [ ] `IPCompletionTest` extrait de `DebugUI` vers `src/core/analysis/`
- [x] `SaveRequests` renommé en `AppRequests` (migration partielle : separation config/runtime à compléter en Bloque 2)
- [ ] `LocomotionMode` et `PostureMode` introduits comme types explicites
- [ ] `SimulationLoop` documenté comme orchestration provisoire dans
  `ARCHITECTURE.md`
- [ ] Décision `curves_lab` exécutée

---

## Bloque 3 — Infrastructure de validation avancée

Objectif : rendre les tests expressifs pour tous les régimes à venir.

- [ ] `DetailedScenarioInit` et helpers de validation de cohérence
- [ ] Tests unitaires dans `tests/unit/`
- [ ] Au moins un test de régression walking dans `tests/regression/`
- [ ] Fonctions d'analyse de base dans `src/core/analysis/`
- [ ] Helper de sérialisation de snapshot `SimState`
- [ ] `VALIDATION_MATRIX` dans `TESTING_AND_VALIDATION.md`

---

## Bloque 4 — Walking complet

Objectif : phases 5 et 6 du roadmap locomoteur validées.

Voir `ROADMAP_locomotion.md` pour le détail.

---

## Bloque 5A — Généralisation structurelle

- [ ] `LocomotionCoordinator` comme module dédié
- [ ] `src/core/pose/` : `PoseController` + `PoseState`
- [ ] Tête et bras comme secondary motion dans `PoseController`
- [ ] Spec jumping, falling, running écrite dans `LOCOMOTION_SPEC.md`
- [ ] Fonctions d'analyse étendues pour nouveaux régimes

---

## Bloque 5B — Jumping + Falling

- [ ] Jumping implémenté et validé (tests de transition, assertions `cm_y`)
- [ ] Falling implémenté avec sous-phases explicites
- [ ] Tests de régression walking non régressés

---

## Bloque 5C — Running

- [ ] `RunController` implémenté
- [ ] Transitions Walking ↔ Running définies et testées
- [ ] Tests de régression walking et jumping non régressés

---

## Bloque 6 — Clôture

- [ ] Crouching comme modificateur orthogonal
- [ ] Getting-up comme transition procédurale explicite
- [ ] Enrichissement pose secondaire (bras/tête par régime)
- [ ] Doxygen complété sur les points d'entrée de sous-systèmes
- [ ] `make test_mem` vert (sans nouvelles fuites depuis Bloque 0)
- [ ] `make build_asan && make test` vert
- [ ] Performance smoke : `make test` complet dans un temps raisonnable
- [ ] Tous les documents vivants reflètent l'état final du système
