# BobTricks — Vision

## Nature du projet

BobTricks est un système de locomotion bipède procédurale 2D écrit en C++20.

Le personnage est un stickman piloté par un noyau physique centré sur le centre
de masse (CM). La pose, le placement des pieds et l'allure émergent
procéduralement à partir de l'état du CM et des règles locomotrices : il n'y a
pas d'animation par keyframes.

Le projet est à la fois :

- une simulation interactive temps réel (frontend SDL2 + ImGui) ;
- un validateur déterministe headless (scénarios, CSV, assertions) ;
- un banc d'essai pour des algorithmes de locomotion (tests unitaires,
  régressions, fonctions d'analyse).

## Régimes locomoteurs dans le scope

Tous les régimes suivants font partie du projet complet :

| Régime | État actuel |
|---|---|
| Standing | Implémenté |
| Walking | Phase 5 en cours (voir `ROADMAP_locomotion.md`) |
| Jumping | Planifié |
| Falling | Planifié |
| Running | Planifié |
| Crouching | Planifié — modificateur, pas mode exclusif |
| Getting-up | Planifié — dépend de Falling |

**Ordre d'implémentation par dépendance technique :**  
walking → jumping + falling → running → crouching → getting-up →
enrichissement de la pose secondaire (bras, tête)

## Ce que veut dire « complet »

Le projet est considéré comme complet quand :

1. Tous les régimes du tableau ci-dessus sont implémentés et couverts par des
   scénarios de validation.
2. Les transitions entre régimes sont gouvernées par des préconditions
   physiques explicites, définies dans `LOCOMOTION_SPEC.md`.
3. `make test` passe proprement, y compris les tests de régression nécessaires
   au périmètre réellement implémenté.
4. Le binaire headless peut être vérifié sans fuites mémoire quand la cible
   `make test_mem` existera.
5. L'application SDL exécute les régimes interactifs sans crash ni artefacts
   visuels bloquants.
6. La documentation vivante décrit fidèlement le système implémenté.

## Hors scope

- IK du haut du corps. Les bras et la tête sont de la secondary motion
  procédurale, pas un solveur d'IK séparé.
- Multijoueur, réseau, chargement d'assets externes.
- Simulation 3D.
- Animation pilotée par motion capture.

## Pose secondaire

Les bras et la tête sont reconstruits à chaque tick par `PoseController` en
fonction du régime locomoteur actif et de l'état du CM. Ils n'ont ni physique
indépendante ni cibles d'IK propres. Un état interne de lissage peut exister à
l'intérieur de `PoseController`, mais il ne fait pas partie de `SimState` ni de
`ScenarioInit` dans le contrat normal.

## Engagement architectural

Un seul noyau de simulation. Plusieurs frontends. Plusieurs modes d'exécution.
Une seule vérité physique. C'est l'invariant central que l'architecture doit
préserver.
