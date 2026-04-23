# Plan détaillé de soutenance — BobTricks

## Objectif de la présentation

La soutenance doit durer **20 minutes** : environ **9 minutes de présentation
technique**, **6 minutes de démonstration**, puis **5 minutes de questions**.
Le but n'est pas de présenter toutes les fonctionnalités, mais de montrer que
BobTricks est un projet techniquement maîtrisé, bien conçu, testable et abouti.

Le fil conducteur choisi est la **locomotion procédurale 2D** :

1. un modèle physique centré sur le centre de masse ;
2. des déclencheurs de pas normaux et d'urgence ;
3. une adaptation au terrain ;
4. une visualisation claire du résultat ;
5. une validation automatique en mode headless.

Ce plan répond directement aux critères d'évaluation : difficulté technique,
conception modulaire, qualité du code, tests, démonstration, documentation et
organisation du travail.

## Message principal à faire passer

BobTricks n'est pas seulement un personnage qui bouge à l'écran. C'est un
système de locomotion procédurale où le mouvement visible est dérivé d'un état
physique simple : le **centre de masse**. Les pieds, les jambes, le torse, les
bras et le rendu sont reconstruits à partir de cet état, puis vérifiés par des
scénarios automatiques sans interface graphique.

Phrase possible pour introduire le projet :

> BobTricks est une simulation 2D de locomotion procédurale : le personnage ne
> joue pas une animation préenregistrée, il marche, court, saute et récupère son
> équilibre à partir d'un modèle physique simplifié du centre de masse.

## Structure temporelle recommandée

| Partie | Durée | Objectif |
|--------|------:|----------|
| Présentation technique | 8-10 min | Expliquer la conception, les classes importantes et les choix techniques. |
| Démonstration | 5-7 min | Montrer ce qui fonctionne dans l'exécutable, dans le mode headless et réserver 20 s à l'exécution Windows pour le bonus multiplateforme. |
| Questions | 5 min | Être prêt à justifier les choix, les limites et les tests. |

## Alignement avec la grille technique

Cette partie ne doit pas forcément devenir une slide complète. Elle sert à
préparer les phrases et les preuves à placer pendant la soutenance pour répondre
explicitement au barème technique.

| Critère évalué | Ce qu'il faut mettre en avant | Moment conseillé |
|----------------|-------------------------------|------------------|
| Impression d'ensemble, difficulté et pertinence des choix techniques | Dire que le sujet est difficile parce qu'il faut obtenir une marche stable sans animation préenregistrée. Quantifier le projet : environ 80 fichiers C++/headers dans `src/` et `tests/`, environ 11 000 lignes de code, noyau physique, rendu SDL, debug UI, audio, effets et headless. | Introduction, slides 1 à 4. |
| Structures de données et algorithmes évolués | Présenter `CMState`, `CharacterState`, `FootState`, `SupportState`, `BalanceState`, `ScenarioDef`, `TelemetryRow`, les `std::vector` de trajectoire/télémétrie, la bibliothèque de scénarios sous forme de `std::map`, l'IK analytique, le XCoM, la marge de stabilité, les splines Bézier et les triggers de pas. | Slides 4 à 9. |
| Maîtrise des bibliothèques externes | Expliquer l'utilisation de SDL2 pour la fenêtre et le rendu, SDL_mixer pour l'audio, Dear ImGui pour le debug UI, OpenGL côté linkage graphique, et le fait que le Makefile compile aussi un binaire headless sans SDL ni ImGui. | Slide 10 et démo graphique. |
| Robustesse du programme | Montrer que la démo ne dépend pas d'un seul chemin manuel : il existe des tests unitaires, des tests de régression et des scénarios headless. Insister sur les assertions anti-NaN, les scénarios de marche, course, saut, arrêt et récupération. | Slide 9, puis démo headless. |
| Finition | Montrer le rendu spline, les overlays lisibles, la caméra, les effets de poussière, l'audio de pas, la configuration persistante et le mode présentation sans debug visuel excessif. | Slide 2, slide 8 et démo graphique. |
| Mémoire propre | Dire que le projet évite la gestion manuelle de mémoire quand ce n'est pas nécessaire : usage de valeurs, références, `std::vector`, structures simples. Prévoir de mentionner `make test_mem` si Valgrind est disponible et testé avant la soutenance. | Slide 10 ou questions. |
| Fichiers de données | Expliquer `data/config.ini` : paramètres regroupés par sections, format texte lisible, utilisé par le debug UI et documenté dans le README. Montrer que les assets sont dans `data/`. | Slide 10. |
| Bonus Linux/Windows | Réserver 20 secondes pour lancer l'exécutable Windows et faire bouger le personnage. Dire explicitement : le projet compile et s'exécute sous Linux et Windows. | Fin de la démo. |

Phrase à placer au début de la partie technique :

> Pour répondre aux critères techniques, on va expliquer nos choix, quantifier
> le code, montrer les modules les plus intéressants, puis démontrer que le même
> noyau fonctionne en graphique, en headless et sous Windows.

## Alignement avec la grille conception et code

Cette grille est évaluée en lisant le code. Il faut donc utiliser la présentation
pour orienter les enseignants vers les bons fichiers et justifier l'organisation
du projet.

| Critère évalué | Ce qu'il faut défendre | Preuve concrète à citer |
|----------------|------------------------|--------------------------|
| Qualité de la conception, diagramme des modules, modularité, développement en parallèle | Le projet est découpé par responsabilités : `core` pour la simulation, `render` pour l'affichage, `headless` pour les scénarios, `debug` pour l'interface de réglage, `config` pour les paramètres, `app` pour l'orchestration. | Montrer le diagramme UML de `doc/class_diagram.md` et expliquer les cinq fichiers principaux de `doc/presentation/archivos_importantes.md`. |
| Modularité de l'interface : passage mode texte / mode graphique | Le même noyau fonctionne dans deux exécutables : application SDL graphique et binaire headless sans SDL. Le headless remplace l'interface par des scénarios et de la télémétrie. | `make run`, `make build_headless`, `./build/bobtricks_headless --all --quiet`, `src/headless/ScenarioRunner.cpp`. |
| Qualité de programmation : pas de variables globales, paramètres propres | Les états sont passés par structures explicites (`InputFrame`, `SimState`, `CMState`, `CharacterState`, `AppConfig`). Les modules reçoivent les données nécessaires par référence ou par valeur, au lieu de dépendre d'un état global caché. | `SimulationCore` reçoit `AppConfig&`, `ScenarioDef` décrit les entrées et assertions, `CharacterRenderer::render()` reçoit explicitement renderer, caméra, état et config. |
| Qualité d'écriture : indentation, nommage homogène, pré/post-conditions | Les fichiers suivent une convention C++ cohérente : classes en PascalCase, fonctions en camelCase, configs structurées, commentaires Doxygen dans les headers, noms orientés domaine (`stepLaunchSwing`, `computeGroundReferenceSample`, `renderSplineLeg`). | Headers dans `src/core/simulation/`, `src/render/CharacterRenderer.h`, `src/headless/ScenarioDef.h`. |
| Tests et capacité à déboguer | Le projet combine tests unitaires, tests de régression et scénarios headless. Les tests vérifient les maths, l'IK, le terrain, la stabilité, la marche, la course, le saut et l'absence de NaN. | `make test`, `make test_unit`, `make test_regression`, `make test_headless`, `tests/unit/test_core_math.cpp`, `tests/regression/test_headless_scenarios.cpp`. |
| Code géré par Git | Dire que l'historique a été utilisé pour reconstruire les phases de développement et que les commits montrent une progression du prototype vers l'architecture finale. | `doc/historique_taches.md` et historique Forge/Git. |
| Compilation par Makefile | Le Makefile central compile l'application SDL, le headless, les tests, ASan/UBSan, Valgrind et la documentation. | `make build`, `make run`, `make test`, `make build_headless`, `make docs`. |
| Organisation des fichiers | Le dépôt suit les dossiers attendus : `src`, `data`, `doc`, `tests`, `vendor`, `build`. Les fichiers générés restent dans `build/`. | Montrer l'arborescence rapidement ou la citer dans le README. |

Phrase à placer sur la slide architecture :

> Notre conception est volontairement séparée en deux chemins : un chemin
> graphique SDL pour l'utilisateur et un chemin headless pour tester exactement
> le même noyau sans interface. Cette séparation répond au critère de modularité
> et rend le projet vérifiable automatiquement.

Phrase à placer sur la slide organisation :

> Si l'on lit le code, les responsabilités sont localisables : la physique est
> dans `core`, l'affichage dans `render`, les tests de comportement dans
> `headless`, et les paramètres dans `config`/`data/config.ini`.

## Alignement avec la grille organisation, documents et présentation

Cette partie sert à préparer les preuves de travail régulier et de livrables à
jour. Elle doit aussi guider la conclusion et la démo.

| Critère évalué | Ce qu'il faut montrer ou dire | Preuve concrète à préparer |
|----------------|-------------------------------|----------------------------|
| Documents de projet final à jour | Présentation, tâches réalisées, diagramme des modules et Gantt doivent être cohérents avec l'état final du code. Dire que les documents ont été mis à jour après les refactorings. | `doc/class_diagram.md`, `doc/historique_taches.md`, `doc/gantt_bobtricks.xlsx`, `doc/presentation/plan_presentation_orale.md`, slides finales. |
| Manuel d'utilisation | Le README doit expliquer le but de l'application, l'organisation des dossiers, la compilation, l'exécution graphique, le mode headless, les tests et les fonctionnalités. | `README.md` avec `make build`, `make run`, `make build_headless`, `make test`, `make test_headless`. |
| Documentation des headers et Doxygen | Les headers contiennent des commentaires Doxygen et la documentation peut être générée par `make docs`. Ne pas forcément lancer Doxygen pendant la démo, mais être prêt à montrer la cible Makefile. | `Doxyfile`, `make docs`, sortie `doc/doxygen/html/index.html`. |
| Présentation et démo : clarté et organisation | La soutenance suit un fil clair : physique, rendu, validation. La démo est scriptée, répétée et répartie entre les trois membres. | Ce document, section "Script de démonstration" et section "Répartition de parole". |
| Capacité à tenir les délais finaux | Expliquer l'évolution du projet par phases : prototype, architecture propre, locomotion, rendu, course/saut, tests, refactoring. Montrer ce qui a été atteint par rapport à l'objectif initial. | `doc/historique_taches.md` et conclusion de la présentation. |
| Motivation et participation individuelle | Chaque membre doit avoir une zone technique identifiable et parler pendant la soutenance. La répartition doit montrer une implication réelle, pas seulement une division artificielle. | Tomas : physique/locomotion ; Meleik : rendu/visuel ; Gedik : headless/tests/organisation/Windows. |

Phrase à placer en conclusion :

> Le projet a évolué régulièrement : on est partis d'un prototype SDL simple,
> puis on a séparé le noyau, le rendu et le headless, ajouté la locomotion
> complète, puis consolidé avec des tests et de la documentation.

Phrase à placer si on parle de l'organisation du groupe :

> La répartition finale correspond aux responsabilités techniques : Tomas a
> porté l'explication physique de la locomotion, Meleik la partie visible et le
> rendu, Gedik la validation headless, l'organisation et la portabilité.

## Plan des slides

### Slide 1 — Titre et idée générale

**Durée : 45 s**

Contenu :

- Nom du projet : **BobTricks**.
- Auteurs du groupe.
- Capture d'écran du personnage en mouvement.
- Résumé en une phrase : locomotion procédurale 2D en C++20 avec SDL2.

À dire :

- Le projet vise à produire une marche lisible sans utiliser d'animation fixe.
- Le personnage est piloté par un modèle physique simplifié.
- La présentation va surtout expliquer la conception et le code, pas seulement
  l'interface.

Critères couverts :

- Présentation claire.
- Impression d'ensemble.
- Difficulté apparente du sujet.

### Slide 2 — Fonctionnalités visibles

**Durée : 45 s**

Contenu :

- Marche et arrêt.
- Course avec transition marche-course.
- Saut et réception.
- Adaptation au terrain.
- Rendu spline, effets visuels et debug overlay.
- Mode headless pour tester les scénarios.

À dire :

- Les fonctionnalités visibles servent à démontrer le système de locomotion.
- Le debug overlay n'est pas seulement décoratif : il rend visibles le CM, le
  XCoM, les pieds, les triggers et les références de sol.
- La partie la plus importante est la cohérence entre simulation, rendu et tests.

Critères couverts :

- Qualité de la réalisation.
- Finition.
- Utilisation correcte de SDL2.

### Slide 3 — Vue d'ensemble de l'architecture

**Durée : 1 min**

Contenu :

- Diagramme UML ou extrait simplifié de `doc/class_diagram.md`.
- Séparation en blocs :
  `core`, `config`, `headless`, `render`, `debug`, `app`.
- Deux chemins d'exécution :
  application graphique SDL et binaire headless sans SDL.

À dire :

- Le noyau de simulation est indépendant de SDL.
- `src/core/` contient la physique, la locomotion, le terrain et les états.
- `src/render/` transforme l'état en image.
- `src/headless/` exécute les mêmes scénarios sans fenêtre graphique.
- Cette séparation permet de tester la locomotion sans dépendre de l'interface.
- Cette organisation permet aussi de répartir le travail : physique, rendu,
  tests et configuration peuvent évoluer sans se mélanger.

Point à insister :

- La règle importante est : **pas de SDL dans le noyau**.
- C'est un choix de conception, pas seulement une organisation de dossiers.
- Le mode headless joue le rôle de version texte/testable de l'application
  graphique.

Critères couverts :

- Qualité de conception.
- Modularité.
- Capacité à développer et tester les modules séparément.
- Modularité interface graphique / mode headless.

### Slide 4 — Fichier 1 : `SimulationCore.cpp`

**Durée : 1 min 15 s**

Contenu :

- Rôle : noyau de simulation.
- État autoritatif : `CMState`.
- Pipeline de `step()` :
  entrée utilisateur, intégration du CM, contrainte de sol, mise à jour des
  pieds, reconstruction du corps, cache des données de debug.

À dire :

- Le centre de masse est la vérité physique du système.
- Les autres données du personnage sont dérivées à chaque frame.
- Ce choix évite de simuler chaque articulation avec un solveur complexe.
- Le modèle est plus simple, plus stable et plus facile à tester.

Éléments techniques à montrer :

- Intégration à pas fixe.
- Modèle de pendule inversé simplifié.
- Reconstruction de la pose depuis le CM.
- Données de debug calculées dans le noyau mais utilisées par le rendu.

Critères couverts :

- Difficulté technique.
- Choix techniques pertinents.
- Structures et algorithmes dignes d'intérêt.

### Slide 5 — Modèle mathématique : CM, XCoM et stabilité

**Durée : 1 min**

Contenu :

Formules à afficher :

```text
omega0 = sqrt(g / h_ref)
xi = x_cm + vx_cm / omega0
xi_step = x_cm + xcom_scale * vx_cm / omega0
```

```text
MoS = min(xi - x_gauche, x_droite - xi)
```

À dire :

- Le XCoM prédit où le centre de masse va se retrouver si rien ne change.
- Dans `SimulationCore`, les triggers utilisent `xi_step`, une version calibrée
  par `xcom_scale` pour régler l'anticipation de la marche/course.
- Si le XCoM sort du polygone de support, un pas devient nécessaire.
- Ce modèle permet de décider quand poser un pied sans écrire une animation à la
  main.

Ne pas trop détailler :

- Ne pas transformer la slide en cours de biomécanique.
- L'objectif est de montrer que les décisions de marche sont calculées.

Critères couverts :

- Algorithmes évolués.
- Originalité de la solution.
- Maîtrise technique.

### Slide 6 — Fichier 2 : `SimulationCoreLocomotion.cpp`

**Durée : 1 min 30 s**

Contenu :

- Fonctions clés :
  `stepFireTriggers`, `stepFireWalkTrigger`, `stepFireRunTrigger`,
  `stepLaunchSwing`, `stepRetargetLateSwings`.
- Deux triggers :
  trigger normal et trigger d'urgence.

Table à afficher :

| Trigger | Condition | Rôle |
|---------|-----------|------|
| Normal | Le pied arrière est trop loin du bassin. | Maintenir la cadence de marche. |
| Urgence | Le XCoM dépasse le pied avant. | Récupérer l'équilibre. |

À dire :

- Le trigger normal donne une marche régulière.
- Le trigger d'urgence empêche le personnage de tomber lors d'une accélération,
  d'une réception ou d'un déséquilibre.
- Les deux mécanismes ne doivent pas être fusionnés : ils ne résolvent pas le
  même problème.

Point fort à défendre :

- Cette séparation rend le comportement plus robuste et plus réglable.

Critères couverts :

- Pertinence des choix techniques.
- Difficulté de l'application.
- Capacité à expliquer une classe ou un module important.

### Slide 7 — Fichier 3 : `GroundReference.cpp`

**Durée : 1 min**

Contenu :

- Rôle : calculer une référence de sol stable sous le personnage.
- Données utilisées :
  terrain, position du CM, pieds, état aérien ou au sol.
- Résultat :
  hauteur de référence, normale moyenne, lissage temporel.

À dire :

- Sur terrain plat, la référence de sol est simple.
- Sur terrain irrégulier, il faut éviter que le CM suive brutalement chaque
  variation locale.
- Le module lisse la référence pour que la marche reste lisible et stable.
- Il intervient aussi lors des sauts et des atterrissages.

Critères couverts :

- Robustesse.
- Algorithme lié au terrain.
- Séparation propre des responsabilités.

### Slide 8 — Fichier 4 : `CharacterRenderer.cpp`

**Durée : 1 min**

Contenu :

- Rôle : convertir l'état physique en rendu lisible.
- Deux niveaux :
  rendu legacy pour debug, rendu spline pour présentation.
- Parties rendues :
  jambes, bras, torse, tête, mains, pieds, marqueurs.

À dire :

- Le renderer ne décide pas de la physique.
- Il lit l'état calculé par le noyau et le transforme en pose visuelle.
- La difficulté est de rendre le résultat compréhensible : une simulation peut
  être correcte mathématiquement mais illisible visuellement.
- Les overlays permettent d'expliquer pendant la démo ce que fait le noyau.

Critères couverts :

- Finition.
- Interface graphique.
- Utilisation de SDL2.
- Capacité à relier code et résultat visuel.

### Slide 9 — Fichier 5 : `ScenarioLibrary.cpp`

**Durée : 1 min**

Contenu :

- Rôle : définir des scénarios headless déterministes.
- Exemples :
  marche 3 secondes, course, arrêt, saut depuis la marche, pente, récupération.
- Commandes utiles :

```bash
make test
make test_headless
./build/bobtricks_headless --all --quiet
```

À dire :

- Le mode headless utilise le même noyau que l'application graphique.
- Les scénarios vérifient le comportement sans dépendre du rendu.
- C'est important parce qu'une locomotion peut sembler correcte à l'œil mais
  régresser sur une mesure : vitesse moyenne, absence de NaN, progression,
  hauteur de saut, capacité à s'arrêter.

Critères couverts :

- Qualité des tests.
- Robustesse.
- Capacité à déboguer.
- Modularité interface graphique / mode sans affichage.

### Slide 10 — Organisation, qualité du code et documentation

**Durée : 1 min**

Contenu :

- Makefile avec cibles séparées.
- Exécution vérifiée sous Linux et sous Windows.
- Bibliothèques externes : SDL2, SDL_mixer, Dear ImGui.
- Git utilisé pour suivre l'évolution.
- Documentation :
  `README.md`, `doc/mathematiques.md`, `doc/decisions_architecture.md`,
  `doc/class_diagram.md`, `doc/historique_taches.md`, `doc/guide_tuning.md`,
  `doc/gantt_bobtricks.xlsx`.
- Documentation Doxygen générable avec `make docs`.
- Paramètres dans `data/config.ini`.
- Tests de robustesse : `make test`, `make test_headless`, `make test_mem` si
  Valgrind est disponible sur la machine de démonstration.
- Organisation des dossiers : `src`, `data`, `doc`, `tests`, `vendor`, `build`.

À dire :

- Le projet est organisé pour être compilé, lancé, testé et documenté.
- Les paramètres numériques ne sont pas dispersés dans le code.
- Le format de données principal est `data/config.ini`, un fichier texte par
  sections qui rend les réglages de locomotion, rendu, audio et terrain lisibles.
- Les décisions d'architecture sont documentées pour expliquer les compromis.
- L'historique des tâches montre une progression : prototype, architecture,
  locomotion, rendu, headless, refactoring.
- Le README sert de manuel d'utilisation : compilation, exécution, scénarios
  headless, tests et organisation des dossiers.
- Le diagramme UML, le Gantt et les tâches réalisées sont à jour avec la version
  finale du projet.
- La gestion mémoire repose surtout sur des objets par valeur, des références et
  des conteneurs standard, pour éviter les allocations manuelles fragiles.
- Les tests et le mode headless servent aussi de support de debug : ils
  reproduisent des situations précises sans dépendre de l'œil humain.
- La démonstration inclut volontairement un lancement sous Windows, car la
  compatibilité Linux/Windows est un bonus explicite de la grille d'évaluation.

Critères couverts :

- Documents à jour.
- Manuel d'utilisation.
- Doxygen.
- Organisation des fichiers.
- Compilation par Makefile.
- Qualité de conception et de programmation.
- Utilisation maîtrisée des bibliothèques externes.
- Fichiers de données documentés.
- Gestion mémoire défendable.
- Tests de régression et capacité à déboguer.
- Bonus multiplateforme Linux/Windows.

### Slide 11 — Transition vers la démonstration

**Durée : 15 s**

Contenu :

- application graphique ;
- overlays de debug ;
- terrain, course, saut ;
- headless ;
- Windows en 20 secondes.

À dire :

- La présentation technique vient d'expliquer les modules ; la démonstration va
  montrer le même fil en exécution.

### Slide 12 — Limites et améliorations possibles

**Durée : 45 s**

Contenu :

- Limites actuelles :
  modèle physique simplifié, personnage 2D, pas de solveur dynamique complet,
  rendu stylisé.
- Améliorations possibles :
  comparaison quantitative avec des données de marche réelle, meilleur contrôle
  des bras, terrains plus complexes, export vidéo, intégration d'un vrai solveur.

À dire :

- Le choix assumé est de privilégier un modèle robuste et compréhensible plutôt
  qu'une simulation biomécanique complète.
- Avec plus de temps, l'amélioration prioritaire serait d'ajouter des métriques
  de validation plus riches dans le headless.

Critères couverts :

- Conclusion non banale.
- Capacité à prendre du recul.
- Adéquation avec le cahier des charges.

## Démonstration recommandée

### Préparation avant la soutenance

À faire avant d'entrer dans la salle :

- Compiler l'application avec `make build`.
- Compiler le headless avec `make build_headless`.
- Vérifier `make test_headless`.
- Vérifier `make test` si le temps de préparation le permet.
- Vérifier `make test_mem` avant la soutenance uniquement si Valgrind est
  installé et suffisamment rapide ; sinon ne pas le lancer en direct.
- Préparer l'exécutable Windows déjà compilé pour le bonus multiplateforme.
- Lancer une fois l'application pour vérifier le plein écran ou la taille de la
  fenêtre.
- Préparer le terminal dans le dossier du projet.
- Préparer la fenêtre graphique déjà prête à être lancée avec `make run`.

### Script de démonstration

**Durée cible : 6 minutes**, dont **20 secondes réservées au lancement sous
Windows** pour montrer la compatibilité multiplateforme.

#### Étape 1 — Lancement de l'application graphique

Commande :

```bash
make run
```

À montrer :

- Le personnage debout.
- La marche vers la droite.
- L'arrêt.

À dire :

- Le mouvement n'est pas une animation fixe.
- La pose est recalculée depuis le centre de masse et les pieds.

#### Étape 2 — Marche et triggers

À montrer :

- Activer l'overlay de debug si disponible.
- Marcher lentement.
- Montrer les pieds, le CM et le XCoM.

À dire :

- Quand le pied arrière devient trop loin, le trigger normal lance un nouveau
  pas.
- Si le XCoM sort de la zone de support, le trigger d'urgence corrige
  l'équilibre.

#### Étape 3 — Terrain

À montrer :

- Avancer sur une zone avec pente ou terrain irrégulier.
- Observer l'adaptation des pieds et du corps.

À dire :

- `GroundReference.cpp` stabilise la référence verticale.
- Les pieds utilisent le terrain, mais le CM ne doit pas suivre toutes les
  irrégularités de manière brutale.

#### Étape 4 — Course et saut

À montrer :

- Maintenir la touche de course.
- Faire un saut puis réceptionner.

À dire :

- La course utilise une logique différente de la marche mais reste intégrée dans
  le même noyau.
- Après réception, le système peut déclencher des pas de récupération.

#### Étape 5 — Mode headless

**Durée cible : 40 s**

Commande :

```bash
./build/bobtricks_headless --all --quiet
```

ou :

```bash
make test_headless
```

À dire :

- Ce test ne rend rien à l'écran.
- Il vérifie automatiquement que les scénarios de locomotion restent valides.
- C'est la preuve que la logique n'est pas uniquement visuelle.

#### Étape 6 — Lancement sous Windows

**Durée cible : 20 s**

À préparer :

- Avoir une version Windows déjà compilée ou un environnement Windows prêt.
- Placer le terminal ou l'explorateur Windows sur l'exécutable avant la démo.
- Ne pas compiler pendant la soutenance : il faut seulement lancer le programme.

À montrer :

- Lancer l'exécutable Windows.
- Faire avancer le personnage quelques secondes.
- Fermer proprement l'application.

À dire :

- Le même projet fonctionne aussi sous Windows, pas seulement sous Linux.
- Cette vérification répond au bonus de compatibilité multiplateforme de la
  grille d'évaluation.

## Répartition de parole

| Personne | Parties recommandées |
|----------|----------------------|
| Tomas | Explications physiques et mathématiques : centre de masse, pendule inversé, XCoM, marge de stabilité, triggers de pas, adaptation de la marche au terrain. |
| Meleik | Partie visible : rendu du personnage, rendu spline, overlays de debug, interface graphique, démonstration visuelle de la marche, de la course et du saut. |
| Gedik | Partie non visible : architecture headless, scénarios automatiques, tests, organisation du projet, Makefile, documentation, lancement Windows pour le bonus multiplateforme. |

Chaque personne doit parler sur une partie technique réelle. Tomas porte le
raisonnement physique, Meleik montre comment ce raisonnement devient visible à
l'écran, et Gedik démontre que le projet est testable, organisé et portable.

## Répartition slide par slide

| Slide | Intervenant principal | Raison |
|-------|----------------------|--------|
| Slide 1 — Titre et idée générale | Tomas | Introduire le problème de locomotion procédurale et le principe du centre de masse. |
| Slide 2 — Fonctionnalités visibles | Meleik | Présenter ce que l'utilisateur voit : marche, course, saut, terrain et overlays. |
| Slide 3 — Vue d'ensemble de l'architecture | Gedik | Expliquer la séparation entre noyau, rendu, debug, headless et application. |
| Slide 4 — `SimulationCore.cpp` | Tomas | Expliquer le noyau physique et le rôle autoritatif du CM. |
| Slide 5 — CM, XCoM et stabilité | Tomas | Présenter les formules et la logique de stabilité. |
| Slide 6 — `SimulationCoreLocomotion.cpp` | Tomas | Expliquer les triggers normaux et d'urgence, puis le lancement du swing. |
| Slide 7 — `GroundReference.cpp` | Tomas | Expliquer l'adaptation physique de la marche au terrain. |
| Slide 8 — `CharacterRenderer.cpp` | Meleik | Expliquer techniquement la transformation de l'état simulé en rendu lisible : monde-écran, legacy/spline, profondeur et Bézier. |
| Slide 9 — `ScenarioLibrary.cpp` | Gedik | Présenter les scénarios headless, les tests et les critères mesurables. |
| Slide 10 — Organisation, qualité du code et documentation | Gedik | Relier Makefile, Git, documentation, tests et portabilité Windows/Linux. |
| Slide 11 — Transition démonstration | Meleik | Lancer la partie visible de la démo. |
| Slide 12 — Limites et améliorations | Tomas, puis Gedik | Tomas conclut sur les limites physiques ; Gedik conclut sur tests et évolutions. |

## Répartition de la démonstration

| Moment de démo | Responsable | Ce qu'il faut montrer |
|----------------|-------------|-----------------------|
| Lancement graphique et marche simple | Meleik | Le personnage visible, la marche, l'arrêt et la lisibilité du rendu. |
| Explication des overlays pendant la marche | Tomas | CM, XCoM, pieds, support et lien avec les triggers de pas. |
| Terrain, course et saut | Meleik, avec commentaire de Tomas | Meleik manipule l'application ; Tomas explique ce que fait la physique. |
| Mode headless | Gedik | Exécution de `make test_headless` ou du binaire headless, sans rendu graphique. |
| Lancement Windows en 20 s | Gedik | Lancer l'exécutable Windows, faire bouger le personnage, fermer proprement. |

## Questions probables et réponses préparées

### Pourquoi ne pas utiliser des animations préenregistrées ?

Parce que le but du projet est de produire une locomotion procédurale. Avec des
animations fixes, il serait plus difficile de réagir correctement à la vitesse,
au terrain, au saut ou aux déséquilibres. Ici, la pose est reconstruite depuis
un état physique.

### Pourquoi le centre de masse est-il l'état autoritatif ?

Parce qu'il donne un état physique compact, stable et testable. Les pieds, le
bassin, le torse et les bras peuvent être recalculés depuis cet état. Cela évite
de simuler toutes les articulations avec un solveur plus complexe.

### Pourquoi deux triggers de pas ?

Le trigger normal gère la cadence de marche. Le trigger d'urgence gère la
stabilité quand le XCoM sort du support. Les fusionner produirait soit une marche
trop rigide, soit des pertes d'équilibre lors des accélérations ou réceptions.

### Comment testez-vous la locomotion ?

Avec trois niveaux : tests unitaires pour les fonctions mathématiques, tests de
régression pour les scénarios, et exécution headless complète. Le mode headless
utilise le même noyau que l'application SDL mais sans rendu.

### Quelle est la partie la plus difficile ?

La partie la plus difficile est la cohérence entre les triggers de pas, le
placement du pied, l'adaptation au terrain et la stabilité du CM. Un petit
changement de seuil peut rendre la marche trop lente, trop nerveuse ou instable.

### Quelle est la principale limite du projet ?

Le modèle reste simplifié : c'est une locomotion 2D stylisée, pas une simulation
biomécanique complète. Ce choix permet d'avoir un résultat stable, compréhensible
et testable dans le temps du projet.

## Checklist finale avant rendu

- La présentation contient le titre, les auteurs et une capture.
- Le diagramme UML est à jour et lisible.
- Le Gantt final est à jour et cohérent avec les tâches réalisées.
- Les cinq fichiers principaux sont expliqués.
- La démonstration est répétée au moins une fois en conditions réelles.
- Le projet compile avec `make build`.
- Le lancement Windows est préparé et prend moins de 20 secondes.
- Les tests headless passent avec `make test_headless`.
- Le README explique compilation, exécution et fonctionnalités.
- La documentation Doxygen est générable avec `make docs`.
- Les responsabilités individuelles sont explicites dans la présentation.
- Les documents du dossier `doc/` sont présents dans l'archive finale.
- Les fichiers inutiles de build ne sont pas inclus dans l'archive.

## Sources utilisées pour construire ce plan

- Consigne officielle LIFAPCD : soutenance de 20 minutes, présentation technique,
  démo, UML, explication des classes importantes et conclusion.
- Grille d'évaluation : technique, conception/code, organisation/documents,
  présentation/démo.
- Règles de conception du projet : modularité, Makefile, Git, Doxygen, README,
  qualité du passage de paramètres, mémoire propre.
- Documentation interne du projet : `README.md`, `doc/class_diagram.md`,
  `doc/decisions_architecture.md`, `doc/mathematiques.md`,
  `doc/historique_taches.md`, `doc/presentation/archivos_importantes.md`.
