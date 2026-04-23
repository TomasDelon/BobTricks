# Fichiers les plus importants pour la présentation

Cette sélection retient uniquement les cinq fichiers qui racontent le mieux
l'histoire de la locomotion : modèle physique du centre de masse, déclenchement
des pas, adaptation au terrain, visualisation du résultat et validation
automatique en mode headless. Les estimations de création indiquent le temps
nécessaire pour produire une première version fonctionnelle et correcte du
fichier, pas le temps historique réel. Le temps d'explication correspond à la
place réaliste à lui donner pendant une soutenance courte.

| # | Fichier | Lignes | Rôle | Pourquoi il est important ou difficile | Estimation de création | Temps d'explication à l'oral |
|---|---------|-------:|------|----------------------------------------|------------------------:|-----------------------------:|
| 1 | `src/core/simulation/SimulationCore.cpp` | 1321 | Contient le noyau principal de la simulation : intégration du centre de masse, physique horizontale et verticale, contacts, reconstruction de la pose et orchestration des sous-étapes de `step()`. | C'est le point central du modèle physique. Il relie le pendule inversé, le contrôle du CM, le XCoM, la stabilité, le saut, la récupération et les données de debug. Une erreur ici modifie tout le comportement de marche. | 45-70 h | 1 min 30 - 2 min |
| 2 | `src/core/simulation/SimulationCoreLocomotion.cpp` | 215 | Regroupe la partie la plus directement liée à la locomotion : déclencheurs de pas, marche, course, récupération après déséquilibre, swing du pied et logique d'atterrissage. | C'est le fichier le plus pertinent pour expliquer ce qui se passe quand le personnage marche. Il sépare les triggers normaux, liés à la cadence, des triggers d'urgence, liés au XCoM et à la stabilité. | 18-30 h | 1 min - 1 min 30 |
| 3 | `src/core/simulation/GroundReference.cpp` | 150 | Calcule une référence de sol stable sous le personnage à partir du terrain, de la position du CM et de l'état des pieds. | Il permet à la marche de rester cohérente sur un terrain non plat. Sa difficulté vient du compromis entre précision locale, lissage temporel et robustesse quand le personnage saute, atterrit ou change d'appui. | 10-18 h | 45 s - 1 min |
| 4 | `src/render/CharacterRenderer.cpp` | 541 | Transforme l'état physique du personnage en rendu lisible : squelette legacy, rendu spline, torse, tête, bras, jambes, mains, pieds et marqueurs visuels. | Il donne une preuve visuelle que la locomotion fonctionne. Le défi est de convertir des données physiques en une pose claire et esthétique sans masquer les informations utiles pour comprendre l'équilibre et les pas. | 25-40 h | 1 min - 1 min 30 |
| 5 | `src/headless/ScenarioLibrary.cpp` | 414 | Définit les scénarios de validation automatique : marche, course, saut, arrêt, récupération et départ immédiat à vitesse maximale. | Il démontre que la marche fonctionne sans dépendre de l'interface graphique. Sa difficulté est de transformer un comportement visuel en critères mesurables, reproductibles et utiles pour détecter les régressions. | 12-20 h | 1 min |

## Fil conducteur pour l'oral

L'ordre le plus clair consiste à partir du modèle physique dans
`SimulationCore.cpp`, puis à expliquer les déclencheurs de pas dans
`SimulationCoreLocomotion.cpp`, l'adaptation au terrain avec
`GroundReference.cpp`, la visualisation dans `CharacterRenderer.cpp`, et enfin
la validation headless avec `ScenarioLibrary.cpp`.
