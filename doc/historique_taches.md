# Historique des tâches — BobTricks

Liste chronologique de toutes les tâches concrètes réalisées au cours du développement,
reconstituée à partir de l'historique git.
Les tâches d'un même commit sont listées dans leur ordre logique d'implémentation.

---

## Phase 0 — Prototype SDL initial

1. Créer la structure de dossiers du projet (`src/`, `data/`, `doc/`)
2. Configurer le projet SDL2 avec fenêtre et boucle d'événements de base
3. Définir `AppState` avec un flag de running et les données de simulation couplées au runtime
4. Normaliser le nommage d'`AppState` pour séparer l'état SDL de l'état de simulation
5. Traduire les pages d'index de documentation en anglais
6. Migrer le système de build de CMake vers un Makefile direct
7. Implémenter `Vec2` initial avec les opérations arithmétiques de base
8. Définir `CMState` (centre de masse : position, vitesse)
9. Définir `CharacterState` avec position du bassin, torse, tête et pieds sous forme de `Vec2`
10. Définir `FootTarget` avec la position cible et un flag de contact
11. Définir `TuningParams` avec les paramètres de géométrie corporelle (hauteurs, longueurs de segments)
12. Implémenter `LocomotionController` avec une logique de pas basée sur un seuil de distance
13. Dériver la géométrie de la jambe (longueur cuisse/tibia) depuis `TuningParams`
14. Corriger le positionnement du `FootTarget` par rapport à l'origine du corps
15. Implémenter `ProceduralAnimator` : IK analytique à 2 segments pour la jambe
16. Implémenter l'oscillation procédurale des bras synchronisée avec le pas
17. Connecter `SimulationCore`, `SDLRenderer` et la boucle applicative : premier personnage visible à l'écran

---

## Phase 1 — Architecture propre (reconstruction)

18. Redéfinir le système de build avec trois cibles : `app`, `headless` et `analysis`
19. Configurer les flags de compilation : C++20, `-Wall -Wextra`, ASan + UBSan en debug
20. Implémenter `Vec2` définitif avec `length()`, `normalize()`, `dot()`, `normalizeOr()`
21. Définir `WorldConstants` avec les constantes physiques (`g`, `kTau`, `kDegToRad`)
22. Définir `MathConstants` avec les epsilon numériques (`kEpsLength`, `kEpsAngle`)
23. Implémenter le système de configuration hiérarchique avec sections INI
24. Implémenter `ConfigIO` avec lecture et écriture de `AppConfig` vers fichier `.ini`
25. Structurer `AppConfig` en sous-configs : `CharacterConfig`, `PhysicsConfig`, `WalkConfig`, etc.
26. Définir `CMState` définitif : position, vitesse, accélération, flags de contact
27. Définir `FootState` avec position courante, position ancrée, normale du terrain, flags (`pinned`, `swinging`, `airborne`, `on_ground`)
28. Définir `SupportState` : type de support (bilatéral, unilatéral gauche/droit, aérien)
29. Définir `BalanceState` : XCoM, marge de stabilité, projection sur le polygone de support
30. Définir `StepPlan` avec position cible du pied, durée de vol et phase de swing
    Type supprimé ensuite lors de la recentralisation de la logique de pas dans `SimulationCore`.
31. Implémenter le générateur de terrain procédural par méthode angle-walk (somme d'angles aléatoires)
32. Paramétrer le terrain : fréquence, amplitude, lissage et graine aléatoire
33. Implémenter l'IK analytique à 2 segments (`LegIK`) : calcul de l'angle du genou par la loi des cosinus
34. Implémenter `BalanceComputer` : calcul du XCoM (extrapolated center of mass) avec `ω = sqrt(g/h)`
35. Calculer la marge de stabilité comme distance signée du XCoM au bord du polygone de support
36. Implémenter `StepPlanner` : condition de déclenchement par seuil du XCoM sur le pied d'appui
    Module supprimé ensuite ; la logique active vit désormais dans `SimulationCore`.
37. Calculer la position cible du pied (placement) relative au CM avec facteur `k_step = 0.90`
38. Implémenter la trajectoire de swing avec arc parabolique (hauteur configurable)
39. Implémenter `TelemetryRecorder` : buffer de `TelemetryRow` par frame avec export CSV
40. Définir `TelemetryRow` avec tous les champs d'état pertinents pour la validation scientifique
41. Implémenter `SimulationCore::step()` : intégration Euler du CM avec modèle de pendule inversé
42. Connecter dans `SimulationCore` : terrain → IK → step planner → balance → état du personnage
43. Implémenter `HeadlessRunner` : binaire déterministe exécutant des scénarios avec assertions
44. Définir `ScenarioDef` avec conditions initiales, durée et prédicats de validation
45. Implémenter `ScenarioLibrary` avec scénarios standards (marche plate, pente, arrêt)
46. Implémenter `Camera2D` avec transformation monde→écran, zoom et suivi du CM
47. Implémenter `SimulationLoop` avec contrôle de timestep fixe et interpolation pour le rendu
48. Implémenter `SceneRenderer` : fond dégradé, grille de terrain, ligne de sol
49. Implémenter `CharacterRenderer` (héritage) : segments de squelette et cercles pour les articulations
50. Implémenter `DebugOverlayRenderer` : marqueurs des pieds, XCoM, polygone de support
51. Intégrer Dear ImGui (embarqué) : initialisation SDL2+OpenGL, nouveau frame et rendu
52. Implémenter le panneau de debug **Physique** : sliders gravité, vitesse maximale, masse
53. Implémenter le panneau de debug **Locomotion** : sliders step speed, k_step, hauteur de swing
54. Implémenter `Application` : boucle d'événements SDL2 avec capture clavier et souris
55. Implémenter l'historique step-back (annulation de pas) : ring buffer de snapshots de `SimState`
56. Implémenter le module `curves_lab` : Bézier cubique, Catmull-Rom, longueur d'arc paramétrée

---

## Phase 2 — Documentation et refactoring mineur

57. Rédiger `ROADMAP_locomotion.md` avec les blocs d'implémentation (Blocs 1–6)
58. Rédiger la documentation de l'architecture headless et les contrats de scénarios
59. Rédiger la documentation vivante (Bloc 1 : état actuel et décisions de conception)
60. Déplacer `Camera2D` de `core/runtime/` vers `render/` (responsabilité correcte)
61. Renommer `SaveRequests` → `AppRequests` pour refléter un usage plus large
62. Supprimer `curves_lab` du build et remplacer les splines par des lignes de squelette (simplification temporaire)
63. Ajouter `CLAUDE.md` avec le contexte de session pour Claude Code
64. Synchroniser `ROADMAP_locomotion.md` avec les signatures réelles du code (`k_step=0.90`, targets relatifs au CM)
65. Afficher `last_trigger` (Normal / Emergency / None) dans le panneau de balance du debug UI
66. Vendoriser les dépendances externes et ajouter une structure de tests avec Google Test

---

## Phase 3 — Upper body, contrôleur de bras et refonte de la locomotion

67. Ajouter `ArmController` : modèle de pendule de bras contralatéral couplé au cycle de pas
68. Implémenter `solveTwoBoneArm()` : IK à deux os avec sélection du coude par préférence de pliure
69. Implémenter `updateArmState()` : mélange walk/run, suivi de phase, couplage avec le pied actif
70. Ajouter `HeadController` : inclinaison de tête selon la vitesse et la direction de déplacement
71. Ajouter `UpperBodyKinematics` : orchestre `ArmController` et `HeadController` avec décalage du torse
72. Ajouter `UpperBodyTypes` : structs `ArmPose`, `HeadPose` pour découpler données et logique
73. Étendre `CharacterState` avec les champs upper body : épaules, coudes, mains, tête, cou
74. Étendre `CharacterState` avec `run_blend`, `arm_phase`, `arm_phase_velocity`
75. Ajouter `FootState.pinned_normal` pour résoudre l'IK sur terrain incliné
76. Étendre `StepPlan` avec des champs pour la trajectoire de swing sur terrain non plat
    Héritage retiré depuis : la trajectoire de swing est maintenant portée par `FootState` et le contexte interne de `SimulationCore`.
77. Affiner `StandingController` : séparer la logique de positionnement du bassin de celle du torse
78. Affiner `BalanceComputer` : exposer `SupportState` comme sortie explicite du calcul
79. Étendre `TelemetryRow` avec les champs upper body (angles de bras, phase de swing)
80. Mettre à jour `TelemetryRecorder` pour sérialiser tous les nouveaux champs dans le CSV
81. Étendre `AppConfig` avec des sous-configs : `ArmConfig`, `HeadConfig`, `SplineRenderConfig`
82. Mettre à jour `ConfigIO` pour lire/écrire toutes les nouvelles sections du `.ini`
83. Affiner `SimulationCore` : intégrer les appels à `UpperBodyKinematics` dans la boucle de step
84. Ajouter `Bezier` (classe définitive) : cubique avec `evaluate(t)` et `flatten(n)`
85. Ajouter `StrokePath` : séquence de courbes Bézier avec `moveTo`, `cubicTo`, `closePath`, `flatten`
86. Ajouter `StrokeRenderer` : rendu de polylignes avec épaisseur de trait variable sous SDL
87. Ajouter interaction souris : glisser-déposer des mains (`left_hand_target`, `right_hand_target`)
88. Ajouter interaction souris : glisser-déposer des pieds (pin/unpin du `FootState`)
89. Ajouter interaction souris : contrôle du gaze (direction du regard de la tête)
90. Implémenter `CharacterRenderer` avec rendu spline : `renderSplineLeg`, `renderSplineArm`, `renderSplineTorso`, `renderSplineHead`
91. Implémenter `DebugOverlayRenderer` avec splines : polygones de contrôle, points d'échantillonnage
92. Ajouter un panneau de debug pour les contrôles de splines (épaisseur, samples par courbe, flags de debug)

---

## Phase 4 — Refonte de la marche (intégration depuis la branche GitHub)

93. Redéfinir `SimulationCore::step()` avec 14 sous-phases nommées explicitement
94. Implémenter `applyGroundConstraint()` : snap du pied sur le terrain avec normale interpolée
95. Implémenter `bootstrapFeetOnLanding()` : initialisation des pieds au premier contact avec le sol
96. Implémenter `stepHandleGroundRecontact()` : récupération après atterrissage d'un saut
97. Implémenter `refreshSwingArcProfile()` : recalcul de l'arc de swing selon le terrain de destination
98. Extraire `GroundReference` comme module indépendant : calcul de `ground_y` et normale moyenne sous le personnage
99. Ajouter `SimulationEvents` dans `SimState` : flags de step, land, jump pour l'audio et les effets
100. Implémenter le panneau de debug **Rendu spline** avec contrôles par section (tête, torse, bras, jambes)
101. Ajouter le panneau de debug **Reconstruction** : contrôles de posture du torse, lean, inclinaison latérale

---

## Phase 5 — Mode course

102. Implémenter le mode course activé par la touche Shift : champ `is_running` dans `InputFrame`
103. Implémenter la phase de vol approximée SLIP : le CM suit un arc parabolique pendant `airborne`
104. Corriger la hauteur du CM en mode course : remplacer l'arc IP par le modèle SLIP avec conservation de hauteur
105. Réimplémenter la course avec oscillation de phase style stickman3 : `run_phase` oscille sinusoïdalement
106. Élargir la foulée et augmenter l'inclinaison vers l'avant du torse en courant
107. Affiner la démarche en course : synchroniser l'oscillation du CM avec la phase d'appui/vol
108. Étendre `ArmConfig` avec des paramètres spécifiques à la course (`run_front_hand_start_deg`, etc.)
109. Exposer les contrôles de bras en course dans le panneau de debug
110. Corriger la démarche asymétrique en descente : appliquer un dégagement par bombement du terrain sous le pied en swing

---

## Phase 6 — Saut et atterrissage

111. Implémenter le saut : la touche Espace génère une impulsion verticale dans `InputFrame`
112. Implémenter la planification de l'atterrissage : prédire la position des pieds au landing
113. Implémenter la récupération de pas après atterrissage : déclenchement de emergency steps
114. Réajuster les cibles de récupération : pas plus courts et centrés sous le CM
115. Affiner le mouvement en montée en courant : ajustement du lean et du dégagement du genou

---

## Phase 7 — Refactoring et modularisation

116. Consolider les constantes mathématiques dupliquées dans `MathConstants.h` (supprimer les redéfinitions dans les `.cpp`)
117. Consolider les helpers vectoriels dupliqués (`normalizeOr`, `dot`) dans `Vec2.h`
118. Extraire les paramètres de saut vers `JumpConfig` dans `AppConfig`
119. Compléter `RunConfig` avec tous les paramètres de course extraits de `PhysicsConfig`
120. Mettre à jour `ConfigIO` pour persister `JumpConfig` et `RunConfig` dans le `.ini`
121. Décomposer `SimulationCore::step()` en 14 fonctions membres nommées (une par sous-phase)
122. Extraire `blendWalkRunConfig()` hors de `stepBlendParams()` comme fonction indépendante
123. Ajouter des tests unitaires pour `BalanceComputer` : vérifier XCoM et marge en posture statique
124. Ajouter des tests unitaires pour `Terrain` : vérifier `height_at()` et l'interpolation aux bords
125. Ajouter des tests unitaires pour les scénarios de saut : vérifier que le CM atteint une hauteur minimale
126. Implémenter la reconstruction configurable du hunch du torse (`torso_hunch_angle`)
127. Améliorer les contrôles de rendu spline : séparer l'épaisseur de trait pour chaque partie du corps
128. Ajouter l'audio de pas : `AudioSystem` joue un sample lors de la détection de `SimulationEvents::step`
129. Ajouter les effets de poussière réactifs : `EffectsSystem` émet un `DustParticle` à chaque pas et atterrissage
130. Ajouter une aide aux interactions dans le panneau de debug (liste des raccourcis clavier et souris)
131. Déplacer `SimulationEvents` des sorties locales de `SimulationCore` vers `SimState` (accessible depuis n'importe quel système)
132. Extraire `InputController` d'`Application` : gère les événements SDL et produit un `InputFrame`
133. Extraire `AudioSystem` d'`Application` : encapsule SDL_mixer et la file de samples
134. Extraire `EffectsSystem` d'`Application` : gère le pool de `DustParticle`, son update et son rendu
135. Éliminer les warnings de compilation restants (casts implicites, variables inutilisées)
136. Extraire les helpers de référence de sol de `SimulationCore` vers le module `GroundReference` indépendant
137. Ajouter le support de deadzone dans `Camera2D` : le CM doit sortir d'une zone centrale avant de déplacer la caméra
138. Exposer les contrôles de bras en course dans le panneau de debug (sliders d'angles front/back hand)
139. Modulariser le flux de simulation : séparer update du CM, update des pieds, update du upper body en fonctions publiques de `SimulationCore`
140. Ajouter un retour visuel dans le debug UI pour l'état de simulation (course vs marche, aérien)
141. Refactoriser `SimulationCore` : éliminer les blocs plant-foot dupliqués en extrayant un lambda `plantFoot`
142. Refactoriser `SimulationCore` : supprimer la double déclaration de `dx`/`dy`/`hdist` dans `refreshSwingArcProfile`
143. Refactoriser `DebugUI` : extraire le helper `sliderDouble` en éliminant ~270 lignes de boilerplate float-cast
144. Refactoriser `ArmController` : supprimer la branche `if/else` morte dans la sélection de la cible de main
145. Refactoriser `ArmController` : fusionner les branches de damping walking/idle en une seule expression avec facteur d'échelle
146. Déplacer `drawFilledCircle` et `drawCircleOutline` de méthodes statiques de classe vers des fonctions libres dans un anonymous namespace
147. Extraire `renderFlattenedPath()` comme fonction libre, éliminant 4 blocs identiques dans `renderSplineHead/Torso/Arm/Leg`
148. Ajouter le lambda `toScreen` dans `computeScreenSpacePose` pour éliminer la répétition des arguments `ground_y/viewport_w/viewport_h`
149. Ajouter le lambda `drawPinnableHand` dans `renderLegacyBody` pour éliminer le motif couleur-pinned dupliqué
150. Répartir l'implémentation de `SimulationCore` entre `SimulationCore.cpp`, `SimulationCoreLifecycle.cpp` et `SimulationCoreLocomotion.cpp` en conservant une API publique unique
151. Répartir l'implémentation de `DebugUI` en séparant les panneaux liés au personnage dans `DebugUICharacterPanels.cpp`
