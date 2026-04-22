# Décisions d'architecture — BobTricks

Ce document explique **pourquoi** le projet est structuré tel qu'il est.
Chaque décision est justifiée par le problème qu'elle résout.

---

## 1. Le CM est l'état autoritatif — tout le reste est dérivé

**Décision :** `CMState` (position, vitesse, accélération) est la seule vérité
physique. `CharacterState` (bassin, torse, membres, pieds) est recalculé à
chaque frame à partir du CM.

**Pourquoi :**
- En locomotion humaine, c'est le CM qui est contrôlé par le système nerveux
  central ; les membres suivent en réaction.
- Séparer physique autoritaire et pose dérivée permet de modifier le rendu
  (lean, hunch, splines) sans jamais toucher la physique.
- Les snapshots de step-back (`StepSnapshot`) ne stockent que `SimState` — ils
  sont petits et déterministes car tout le reste se recalcule.

**Alternative rejetée :** Simuler chaque articulation par contraintes. Trop
coûteux, instable sans solveur dédié, inutile pour un personnage 2D stylisé.

---

## 2. Noyau sans SDL — binaire headless séparé

**Décision :** Tout ce qui touche à la physique et à la locomotion vit dans
`src/core/` et `src/config/`. Zéro include SDL dans ces dossiers.

**Pourquoi :**
- Le binaire `bobtricks_headless` tourne en CI/CD sans affichage.
- Les tests de scénarios (`ScenarioRunner`) sont déterministes et reproductibles.
- Le noyau peut être réutilisé dans un contexte serveur ou WebAssembly.
- La règle est vérifiée par `scripts/check_architecture.sh` à chaque build.

**Conséquence directe :** `Camera2D` est dans `src/render/`, pas dans `src/core/`.
`DebugUI` est dans `src/debug/`. `Application` est dans `src/app/`. Seuls les
headers de `src/core/` et `src/config/` peuvent être inclus par le headless.

---

## 3. Boucle à pas fixe avec accumulateur (`SimulationLoop`)

**Décision :** La simulation avance par pas de `fixed_dt = 1/60 s` indépendamment
du framerate de rendu.

**Pourquoi :**
- Le modèle d'intégration Euler semi-implicite est stable seulement si `dt` est
  constant et petit. Avec un `dt` variable, l'énergie dérive.
- Les tests headless sont déterministes : le même scénario produit exactement
  les mêmes états quel que soit le matériel.
- Le step-back fonctionne par snapshots numérotés — facile avec pas fixe.
- Le `max_accumulator` évite la « spirale de la mort » (lag → gros dt → plus
  de lag) en plafonnant le retard accumulé.

**Alternative rejetée :** Intégration à pas variable. Rapide à implémenter,
mais non déterministe et instable sur les branches à forte courbure (swing arc).

---

## 4. Configuration INI par sections (`AppConfig` + `ConfigIO`)

**Décision :** Tous les paramètres numériques sont dans `data/config.ini`,
organisés en sections (`[physics]`, `[walk]`, `[arms]`…). Aucune constante
magique dans le code.

**Pourquoi :**
- **Itération rapide :** modifier un paramètre ne nécessite pas de recompilation.
- **Persistance automatique :** `DebugUI` modifie les structs en mémoire ;
  `Application` déclenche `ConfigIO::save()` à chaque changement.
- **Lisibilité :** un étudiant peut ouvrir `config.ini` et comprendre les
  paramètres sans lire le code.
- **Robustesse :** les clés inconnues sont ignorées lors du chargement —
  un ancien fichier de config reste compatible avec une version plus récente.

**Structure de la hiérarchie :**
```
AppConfig
 ├── SimLoopConfig       [sim_loop]
 ├── PhysicsConfig       [physics]
 ├── WalkConfig          [walk]
 ├── RunConfig           [run]
 ├── JumpConfig          [jump]
 ├── CharacterConfig     [character]
 ├── ArmConfig           [arms]
 ├── HeadConfig          [head]
 ├── TerrainConfig       [terrain]
 └── …
```

---

## 5. Deux mécanismes de déclenchement de pas coexistent

**Décision :** un déclencheur normal (cadence) et un déclencheur d'urgence
(XCoM dehors) sont deux mécanismes distincts, jamais fusionnés.

**Pourquoi :**
- Le déclencheur normal gère la **cadence** : il s'assure que le pied arrière
  ne traîne pas. Il correspond au pattern « tapis roulant » de la marche normale.
- Le déclencheur emergency gère la **stabilité** : quand ξ dépasse le pied avant,
  un pas must fire immédiatement, quelle que soit la cadence.
- Fusionner les deux en un seul seuil conduit à une marche soit trop rigide
  (seuil trop court) soit à des chutes lors des accélérations soudaines
  (seuil trop long).
- Le panneau **Balance** du debug UI affiche le type du dernier déclencheur en
  temps réel pour l'analyse.

---

## 6. `CharacterState` contient des champs de debug

**Décision :** `CharacterState` inclut `debug_cm_target_y`, `debug_ref_slope`,
etc. — des valeurs intermédiaires normalement internes à `SimulationCore`.

**Pourquoi :**
- Les renderers et l'UI de debug ont besoin de ces valeurs pour afficher des
  overlays utiles (hauteur cible, référence de sol, markers de pied).
- Les sortir de `SimulationCore` via `SimState` évite de recalculer ces
  quantités côté rendu avec des configs potentiellement obsolètes.
- En production, ces champs existent mais sont ignorés — pas de coût à l'exécution.

**Convention :** tout champ préfixé `debug_` est calculé dans le noyau pour
l'affichage et ne doit jamais influencer la physique.

---

## 7. `StrokePath` est séparé de `StrokeRenderer`

**Décision :** `StrokePath` accumule des commandes vectorielles (moveTo, cubicTo…),
`StrokeRenderer` les convertit en pixels. Ce sont deux classes distinctes.

**Pourquoi :**
- Séparation de responsabilités : la géométrie des membres (domaine du noyau)
  est séparée du rastérisation (domaine SDL).
- `StrokePath` peut être testé sans SDL (`flatten()` retourne un `std::vector<Vec2>`).
- Si SDL est remplacé par OpenGL ou une autre API, seul `StrokeRenderer` change.

---

## 8. `SimulationCore` référence `AppConfig` sans en être propriétaire

**Décision :** `SimulationCore` reçoit une **référence non possédée** à
`AppConfig` dans son constructeur.

**Pourquoi :**
- Les paramètres modifiés via l'UI de debug sont reflétés immédiatement au
  prochain `step()` sans aucun mécanisme de notification.
- `Application` possède `AppConfig` (durée de vie garantie) ; `SimulationCore`
  n'en est qu'un consommateur.
- Alternative rejetée : copier la config dans le noyau à chaque frame →
  surcoût inutile et risque de désynchronisation entre UI et physique.

---

## 9. Le renderer spline est une surcouche optionnelle

**Décision :** Le rendu legacy (segments SDL) et le rendu spline (Bézier épais)
coexistent. `SplineRenderConfig::enabled` bascule entre les deux, et
`draw_under_legacy` permet de superposer les deux.

**Pourquoi :**
- Le rendu spline est encore expérimental. Le legacy est toujours fonctionnel
  et utile pour le debug.
- La superposition permet de valider visuellement que la pose spline correspond
  bien à la pose legacy.
- En mode présentation (`PresentationConfig`), tous les overlays de debug sont
  masqués et seul le rendu spline est visible.

---

## 10. Gestion de la caméra avec deadzone

**Décision :** La caméra ne suit le CM que lorsqu'il sort d'une zone centrale
(`deadzone_x`, `deadzone_y` en mètres monde). À l'intérieur, la caméra est fixe.

**Pourquoi :**
- Sans deadzone, la caméra oscille en permanence avec le bob vertical du CM,
  ce qui est visuellement fatigant.
- La deadzone donne l'impression que le personnage « joue dans l'espace » plutôt
  que d'être toujours collé au centre de l'écran.
- Les valeurs par défaut (0,35 m horizontal, 0,15 m vertical) correspondent
  approximativement à l'amplitude du bob à vitesse de marche normale.

---

## 11. Tests séparés en unitaires et régressions

**Décision :** Deux binaires de test distincts :
- `build/tests/unit/core_unit_tests` — tests bas niveau sur les fonctions
  mathématiques (`BalanceComputer`, `Terrain`, `LegIK`…).
- `build/tests/regression/headless_regression_tests` — tests sur des scénarios
  complets (marche plate, pente, saut).

**Pourquoi :**
- Les tests unitaires sont rapides (< 1 s) et localisent précisément la régression.
- Les tests de régression valident le comportement émergent du système complet,
  qui ne peut pas être capturé par des tests unitaires isolés.
- `TelemetryRecorder` est réutilisé dans les deux : les assertions de régression
  portent sur des statistiques de la trajectoire (vitesse moyenne, hauteur
  minimale atteinte lors d'un saut, etc.).
