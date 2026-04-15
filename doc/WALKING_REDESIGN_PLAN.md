# BobTricks — Plan de refonte du walking

## Statut du document

Ce document est un **plan de redesign** du walking.

Il ne décrit **pas** l'état réel du code actuel. Son but est de définir :

- le comportement cible ;
- les invariants physiques et géométriques ;
- l'ordre d'implémentation ;
- les critères de validation ;
- l'appui théorique minimal derrière les choix.

`doc/LOCOMOTION_SPEC.md` reste la référence de l'état effectivement implémenté.
Ce document sert à préparer une révision plus correcte du walking, en
particulier pour les pieds, le support et les pentes.

---

## 1. Problème à résoudre

Le walking actuel présente trois défauts structurels :

1. la logique de contact du pied n'est pas alignée avec le contact réel au sol ;
2. la logique de support / recentrage du CM est mal posée sur terrain incliné ;
3. le CM présente des discontinuités, surtout en descente.

Conclusion :

- le problème principal n'est pas la couleur des overlays ;
- le problème principal n'est pas non plus le tuning fin ;
- le problème est un problème de **modèle**.

Le redesign doit donc commencer par les définitions autoritatives :

- qu'est-ce qu'un pied en contact ?
- qu'est-ce qu'un pied de support ?
- qu'est-ce qu'une transition de support ?
- quelles quantités du CM doivent être continues ?

---

## 2. Principes directeurs

### 2.0 Le redesign reste CM-first

Le redesign proposé reste **CM-first**.

Cela signifie :

- le CM demeure la variable physique maîtresse ;
- la locomotion n'est pas pilotée par une animation de pieds indépendante ;
- les pieds et le support n'introduisent pas une seconde "vérité physique" ;
- ils définissent les **contraintes de contact et de support** auxquelles la
  dynamique du CM doit obéir.

En résumé :

```text
CM-first
+ contact réel piloté par la géométrie
+ support dérivé des pieds actifs
+ foot lock strict en stance
```

La hiérarchie cible est donc :

1. le CM reste la quantité physique primaire ;
2. le support borne ce que le CM peut faire de façon cohérente ;
3. l'IK et la pose reconstruisent le corps sous ces contraintes.

### 2.1 Vérité physique unique

Le planner ne décide pas, à lui seul, qu'un pied est "au sol".

Le contact doit être dérivé de :

- la géométrie du pied ;
- la géométrie locale du terrain ;
- la vitesse relative pied-sol ;
- l'état de phase de la jambe.

### 2.2 Foot lock strict en support

Si un pied est en support, sa position d'ancrage monde reste fixe
(à une tolérance numérique près).

Le corps se déplace par rapport au pied.
Le pied ne glisse pas par rapport au sol.

### 2.3 Le support gouverne le CM

Le CM ne doit pas être recentré par une règle plane implicite du type
"reviens au milieu entre les deux pieds".

Le CM doit évoluer à partir :

- des contacts actifs ;
- de la transition entre ancien et nouveau support ;
- de la géométrie locale du terrain.

### 2.4 Terrain incliné = cas nominal

La montée et la descente ne sont pas des cas spéciaux ajoutés après coup.

Le modèle cible doit être correct dès le départ pour :

- terrain plat ;
- pente montante ;
- pente descendante ;
- variations modérées de pente.

---

## 3. Références théoriques

Le plan s'appuie sur les documents déjà présents dans le workspace :

- `others/stickman2_copy/doc/reports/2007-SIMBICON-Simple-Biped-Locomotion-Control.md`
- `others/stickman2_copy/doc/reports/Kuo2007-Six-Determinants-Gait-Inverted-Pendulum.md`
- `others/stickman2_copy/doc/reports/Normal Gait.md`
- `others/stickman2_copy/doc/reports/Footskate cleanup for motion capture editing.md`
- `others/stickman2_copy/doc/reports/The Motion of Body Center of Mass During Walking: A Review Oriented to Clinical Applications.md`
- `others/stickman2_copy/doc/reports/Inverse Kinematics and Geometric Constraints for Articulated Figure Manipulation.md`

### 3.1 Ce qu'on retient de SIMBICON

- la marche doit être structurée par événements et sous-phases ;
- `stance` et `swing` doivent être explicites ;
- la position du prochain pied dépend du CM et de sa vitesse ;
- les événements de contact doivent pouvoir déclencher les transitions.

### 3.2 Ce qu'on retient de Kuo

- le CM ne doit pas être artificiellement aplati ;
- le coût mécanique critique est la transition pas-à-pas ;
- le double support est une phase mécanique importante ;
- le CM doit être gouverné par les appuis actifs, pas par une oscillation décorative.

### 3.3 Ce qu'on retient du gait normal

- le cycle a une structure temporelle claire ;
- l'appui total occupe environ 60 % du cycle ;
- le swing occupe environ 40 % ;
- la marche naturelle alterne double support, appui simple, double support, swing ;
- la jambe doit absorber / propulser, pas seulement "poser un pied plus loin".

### 3.4 Ce qu'on retient du footskate cleanup

- un pied en `footplant` doit rester fixe en monde ;
- sans foot lock, la crédibilité visuelle du walking s'effondre immédiatement.

### 3.5 Ce qu'on retient des papiers CM / IK

- le CM reste la variable mécanique maîtresse ;
- mais l'IK doit s'adapter au CM sans casser les contraintes de chaîne ;
- les contraintes géométriques persistantes sont plus importantes que
  l'élégance mathématique pure.

---

## 4. Modèle géométrique du terrain

Le terrain 2D est modélisé comme :

```text
y = h(x)
```

Pour tout `x`, on définit :

```text
q(x) = (x, h(x))                            point du terrain
t(x) = normalize( (1, h'(x)) )             tangente locale
n(x) = normalize( (-h'(x), 1) )            normale locale
```

Remarque :

- si `h'(x)` n'est pas analytiquement disponible, on utilise une estimation
  numérique locale ;
- toutes les décisions de contact et de clearance doivent être exprimées dans
  ce repère local, pas uniquement en hauteur verticale globale.

---

## 5. Variables autoritatives à introduire / clarifier

Pour chaque pied `i ∈ {L, R}` :

```text
phase_i            ∈ {Swing, Landing, Loading, Stance, PushOff}
p_i                position monde du pied
v_i                vitesse monde du pied
anchor_i           ancre monde utilisée pendant le support
contact_geom_i     bool
support_active_i   bool
load_i             ∈ [0,1]
```

Pour le corps :

```text
CM                 position du centre de masse
Vcm                vitesse du CM
Acm                accélération du CM
support_state      dérivé des pieds actifs
```

Règle dure :

- `support_state` n'est jamais saisi à la main ;
- il est toujours reconstruit à partir des pieds et du terrain.

---

## 6. Définition du contact réel du pied

Pour un pied placé en `p_i = (x_i, y_i)` :

```text
q_i   = (x_i, h(x_i))
d_n_i = (p_i - q_i) · n(x_i)      distance normale au terrain
v_n_i = v_i · n(x_i)              vitesse normale relative
```

Définition cible :

```text
contact_geom_i = (|d_n_i| <= eps_contact)
                 AND
                 (v_n_i <= v_contact_max OR phase_i ∈ {Loading, Stance, PushOff})
```

Interprétation :

- le pied doit être géométriquement au niveau du terrain ;
- son mouvement doit être compatible avec une phase d'appui.

Règle dure :

- un pied ne peut jamais être considéré comme "planté" si `contact_geom_i == false`.

Conséquence directe :

- l'overlay orange doit lire le contact réel, pas seulement la phase interne.

---

## 7. Sémantique des phases du pied

### 7.1 Swing

- pas de contact actif ;
- le pied suit une trajectoire continue vers sa cible ;
- aucune contribution au support.

### 7.2 Landing

- phase terminale du swing ;
- le pied approche la cible et recherche une pose compatible avec le terrain ;
- le pied n'est pas encore verrouillé.

### 7.3 Loading

- le contact réel vient d'être établi ;
- le pied commence à accepter de la charge ;
- le support devient partagé avec l'autre pied.

### 7.4 Stance

- le pied est verrouillé sur une ancre monde ;
- le contact est réel ;
- le pied constitue le support principal ou partagé.

### 7.5 PushOff

- le pied est encore en contact ;
- il continue à contribuer au support ;
- mais il se décharge et prépare sa sortie vers le swing.

Remarque :

- le schéma actuel `Planted / Swing` est insuffisant pour gérer proprement
  la transition de support, surtout en pente.

---

## 8. Règle de foot lock

À l'entrée en `Stance` :

```text
anchor_i = p_i
```

Pendant `Stance` :

```text
p_i = anchor_i
```

à une tolérance numérique très faible près.

Règle dure :

- le pied de stance ne glisse pas.

Conséquences :

- le support devient une réalité géométrique stable ;
- la crédibilité visuelle de la marche s'améliore immédiatement ;
- le CM et la pelvis doivent s'adapter au pied, pas l'inverse.

---

## 9. Définition du support

Un pied contribue au support si :

```text
support_active_i = contact_geom_i
                   AND phase_i ∈ {Loading, Stance, PushOff}
```

Cas possibles :

- `0` pied actif : non compatible avec un walking stable ;
- `1` pied actif : appui simple ;
- `2` pieds actifs : double support.

Quand les deux pieds sont actifs :

```text
load_L + load_R = 1
load_L, load_R ∈ [0,1]
```

Interprétation :

- pendant `Loading`, le pied qui vient d'arriver gagne progressivement de la charge ;
- pendant `PushOff`, l'ancien pied de support perd progressivement de la charge.

Ce modèle permet une transition continue de support, au lieu d'un switch brutal.

### 9.1 Loi cible de transfert de charge

Le transfert de charge doit être une fonction continue du temps de phase
pendant le double support.

Notation :

```text
tau_ds ∈ [0,1]     phase normalisée du double support
```

Avec :

```text
load_old = 1 - w(tau_ds)
load_new =     w(tau_ds)
```

où `w` doit satisfaire :

- `w(0) = 0`
- `w(1) = 1`
- `w` monotone croissante
- `w'(0) = 0` et `w'(1) = 0` souhaitables pour éviter un transfert brutal

Exemples acceptables :

- `w(t) = t`
- `w(t) = 3t² - 2t³`

Préférence conceptuelle actuelle :

- utiliser une loi lissée de type cubic smoothstep, car elle évite une
  cassure nette dans la dérivée du support effectif.

Conséquence physique attendue :

- le rôle du pied ancien décroît continûment ;
- le rôle du pied nouveau croît continûment ;
- la référence mécanique du CM évolue sans switch instantané.

### 9.2 Support effectif utilisé par le CM

Le support effectif n'est pas un simple "milieu des pieds".

On définit une référence effective de support :

```text
S_eff = load_L * S_L + load_R * S_R
```

où `S_L` et `S_R` désignent les références de support associées à chaque pied
actif.

À ce stade, `S_i` peut être compris comme :

- la position d'ancrage du pied ;
- la géométrie locale du terrain sous ce pied ;
- les quantités utiles à la définition du setpoint du CM.

Le détail exact de `S_i` sera précisé plus bas.

---

## 10. Sous-phases du walking

Le cycle cible de marche est :

1. `Initial Double Support`
2. `Single Support`
3. `Terminal Double Support`
4. `Swing`

Correspondance fonctionnelle :

### 10.1 Initial Double Support

- le pied avant est en `Loading` ;
- le pied arrière est en `PushOff` ;
- le CM est redirigé du support ancien vers le support nouveau.

### 10.2 Single Support

- un seul pied reste support principal (`Stance`) ;
- l'autre jambe est en `Swing` ;
- le corps "vault" au-dessus du support actif.

### 10.3 Terminal Double Support

- le nouveau contact est établi ;
- la charge se transfère ;
- l'ancien support termine sa contribution.

### 10.4 Swing

- la jambe libre se reconfigure ;
- la trajectoire vise la prochaine pose de contact valide ;
- le pied ne participe pas au support.

Référence biomécanique initiale :

- appui total ≈ 60 % ;
- swing ≈ 40 %.

Ce ratio n'est pas un verrou absolu, mais une base raisonnable.

---

## 11. Positionnement de la cible du swing

La cible du swing doit être construite en deux étapes :

### 11.1 Cible horizontale

```text
x_target = f( vitesse, direction, état de balance, marge de récupération )
```

Le détail exact de `f` reste à fixer.

Règles dures :

- la cible doit rester dans un domaine atteignable ;
- la cible ne doit pas provoquer de croisement absurde ;
- la cible doit être cohérente avec la direction de marche ;
- la cible doit permettre un support futur plausible.

### 11.2 Projection sur le terrain

```text
y_target = h(x_target)
```

En pente :

- la cible n'est jamais évaluée contre un sol plat fictif ;
- elle vit directement sur le terrain réel.

---

## 12. Trajectoire du swing sur terrain incliné

Une simple arche "au-dessus de la ligne droite takeoff → landing" n'est pas
suffisante sur terrain vallonné.

Condition cible :

```text
d_n( pied(t), terrain ) >= clearance_min(t)
```

pour toute la durée du swing.

Exigences :

- la trajectoire doit être continue ;
- elle doit conserver une clearance positive contre le profil du terrain ;
- elle doit pouvoir monter et descendre une pente sans pénétration ;
- elle doit revenir progressivement vers le terrain avant le landing.

Remarque :

- la forme exacte de la trajectoire peut rester une heuristique ;
- l'invariant "pas de pénétration pendant swing" est non négociable.

---

## 13. Landing valide

Un landing ne doit pas être déclenché uniquement parce que la durée de swing est terminée.

Un landing est valide si :

1. la position du pied est compatible avec le terrain ;
2. le contact géométrique devient vrai ;
3. la vitesse d'impact est acceptable ;
4. la configuration IK reste valide.

Alors seulement :

- le pied passe en `Loading` ;
- le nouveau support est créé ;
- le double support commence.

### 13.1 Conditions minimales d'acceptation du landing

Un landing est accepté si, et seulement si, les conditions suivantes sont
réunies simultanément :

1. **proximité géométrique**

```text
|d_n_i| <= eps_contact
```

2. **compatibilité cinématique**

la vitesse normale d'impact ne doit pas être incompatible avec une prise de
contact stable :

```text
v_n_i <= v_n_land_max
```

3. **cohérence directionnelle**

le pied ne doit pas arriver avec un mouvement tangent inversé absurde par
rapport à la direction de marche et au terrain local.

4. **validité géométrique de la chaîne**

la pose de réception doit rester compatible avec les contraintes IK :

- reach valide ;
- pas de jambe géométriquement impossible ;
- pas de croisement non désiré.

### 13.2 Rejet du landing

Si la durée nominale du swing est atteinte mais que les conditions de landing
valide ne sont pas satisfaites, alors on ne doit pas immédiatement considérer
le pied comme support.

Le système doit choisir l'une des stratégies suivantes :

1. prolonger brièvement la phase `Landing` ;
2. projeter localement le pied vers une pose compatible ;
3. déclarer un incident de locomotion si aucune pose valide proche n'existe.

Règle dure :

- un timer de swing ne crée jamais, à lui seul, un support réel.

Conséquence :

- `u >= 1` ne doit plus signifier automatiquement "pied déjà support".

---

## 14. Comportement cible du CM

### 14.1 Principe général

Le CM ne doit pas être "recentré" par une règle plane explicite ou implicite
du style :

```text
ramener x_cm vers le milieu entre les deux pieds
```

Cette règle casse dès que le terrain est incliné ou que les transitions
d'appui deviennent non triviales.

### 14.2 Appui simple

En appui simple :

- le support actif définit la référence principale ;
- le CM suit une dynamique de type pendule inversé simplifié ;
- la géométrie du support actif contraint l'évolution du corps.

Propriété attendue :

- en appui simple, le CM reste gouverné par un unique pied de référence ;
- les grandeurs dérivées du support doivent être cohérentes avec ce pied et
  avec la pente locale sous ce pied.

### 14.3 Double support

En double support :

- le CM doit être redirigé de l'ancien support vers le nouveau ;
- cette redirection doit être continue ;
- l'effet des deux pieds doit être mélangé par les coefficients de charge.

Schéma abstrait :

```text
R_support = load_L * R_L + load_R * R_R
```

où `R_L` et `R_R` représentent des références de support dérivées de chaque
pied actif.

Le point important n'est pas encore la formule précise, mais l'invariant :

- **la transition est continue**.

### 14.4 Pentes et variations de pente

Le setpoint vertical du CM ne doit pas être obtenu par une fonction brusque
de `terrain.height_at(cm.x)`.

Il doit être dérivé du ou des supports actifs :

- positions réelles des pieds actifs ;
- géométrie locale du terrain sous ces pieds ;
- poids relatifs pendant la transition.

Sinon :

- le CM fait des pops en descente ;
- la logique de support devient incohérente dès que le sol n'est plus plat.

### 14.5 Référence de support en pente

Pour chaque pied actif `i`, on définit une référence locale de support :

```text
S_i = (anchor_i, n_i, t_i, h_i)
```

avec :

- `anchor_i` : point d'ancrage du pied ;
- `n_i` : normale locale du terrain sous le pied ;
- `t_i` : tangente locale ;
- `h_i` : hauteur locale du terrain au point d'ancrage.

La référence mécanique effective du support est alors dérivée de :

```text
S_eff = load_L * S_L + load_R * S_R
```

Interprétation pratique :

- en appui simple, `S_eff = S_stance` ;
- en double support, `S_eff` évolue continûment d'un pied vers l'autre ;
- sur pente, la direction verticale utile du support est reconstruite à partir
  de la géométrie locale active, pas d'un sol plat fictif.

### 14.6 Setpoint vertical du CM

Le setpoint vertical ne doit plus être pensé comme :

```text
y_target = h(cm.x) + constante
```

car cela introduit un risque fort de cassure lorsque :

- le terrain change vite ;
- le pied dominant change ;
- le CM se trouve entre deux pieds sur des hauteurs différentes.

Le setpoint vertical cible doit être formulé comme une fonction du support
effectif :

```text
y_target = F(S_eff, configuration_du_corps, phase_de_marche)
```

Contraintes sur `F` :

- continuité temporelle ;
- dépendance aux pieds réellement actifs ;
- invariance vis-à-vis du changement discret de pied dominant ;
- compatibilité avec le terrain incliné.

À ce stade, le détail exact de `F` reste ouvert.

En revanche, l'invariant cible est clair :

- aucun changement de support ne doit produire de saut instantané du setpoint.

### 14.7 Cas critique : descente

La descente doit être traitée comme le test prioritaire du redesign.

Pourquoi :

- c'est le cas où les erreurs de support vertical apparaissent le plus vite ;
- les deux pieds peuvent être à des hauteurs différentes ;
- un changement brutal de support se voit immédiatement sur le CM.

Critère de réussite spécifique en descente :

- le CM suit une évolution continue ;
- le changement de pied dominant ne provoque ni snap vertical ni cassure
  visible de vitesse.

---

## 15. Invariants de continuité du CM

Pendant le walking, on exige :

1. `x_cm(t)` continu ;
2. `y_cm(t)` continu ;
3. `v_cm(t)` continue, sauf impulsion explicitement modélisée ;
4. pas de changement discontinu du setpoint vertical à un changement de support ;
5. pas de "snap" au passage :
   `single support -> double support -> single support`.

Règle dure :

- si le support change et que le CM saute, c'est que la logique de support est mal définie.

---

## 16. Règles dures vs heuristiques

### 16.1 Règles dures

- le contact réel est distinct de la phase du pied ;
- un pied de stance est verrouillé en monde ;
- le support est dérivé des contacts actifs réels ;
- un landing requiert une géométrie valide ;
- le CM est continu ;
- le swing ne traverse pas le terrain.

### 16.2 Heuristiques acceptables

- la formule exacte de `x_target` ;
- la durée exacte du swing ;
- le profil exact de `clearance_min(t)` ;
- la loi exacte de transfert de charge `load_L / load_R` ;
- le détail du push-off tant qu'il reste continu et cohérent.

Autrement dit :

- la structure ne se négocie pas ;
- le tuning se négocie.

---

## 17. Ordre d'implémentation recommandé

### Bloc A — vérité du contact

Objectif :

- distinguer proprement phase, contact réel et support.

Doit fonctionner :

- un pied orange touche réellement le sol ;
- un pied en l'air n'est jamais présenté comme support.

### Bloc B — foot lock

Objectif :

- verrouiller correctement le pied de support.

Doit fonctionner :

- zéro footskate visible en stance.

### Bloc C — swing et landing sur terrain réel

Objectif :

- rendre le swing compatible avec montée et descente.

Doit fonctionner :

- pas de traversée du sol ;
- pas de landing flottant.

### Bloc D — double support explicite

Objectif :

- modéliser la transition de support.

Doit fonctionner :

- les deux pieds peuvent être actifs sans incohérence logique ;
- la transition de charge devient continue.

### Bloc E — refonte du comportement du CM

Objectif :

- supprimer le recentrage plane-based ;
- dériver le CM du support réel.

Doit fonctionner :

- plus de pops du CM en descente ;
- plus de logique absurde quand les deux pieds sont au sol sur une pente.

### Bloc F — réglage fin du walking

Objectif :

- retoucher trigger, placement, timing, push-off, aspect naturel.

Remarque :

- ce bloc n'a de sens que quand A–E sont déjà sains.

---

## 18. Critères de validation

### 18.1 Validation géométrique

- stance = pied verrouillé ;
- pas de swing qui traverse le terrain ;
- pas de pied "planté" sans contact réel ;
- IK toujours valide dans les limites définies.

### 18.2 Validation dynamique

- pas de saut brutal du CM ;
- transition continue en double support ;
- cohérence du support en terrain incliné.

### 18.3 Validation perceptive

- plus de faux contact visible ;
- plus de flottement manifeste du pied de support ;
- descente de pente sans discontinuité du corps ;
- transfert de poids lisible.

---

## 19. Décisions explicites à prendre plus tard

Les points suivants sont volontairement laissés ouverts :

1. la formule exacte du placement horizontal du pied ;
2. la loi précise de transfert de charge ;
3. la forme précise de la trajectoire swing ;
4. l'existence ou non d'un push-off explicite sur le CM ;
5. le niveau de détail futur du pied (point, segment, heel/toe).

Ils ne bloquent pas le redesign conceptuel.

En revanche, ils ne doivent être décidés qu'après stabilisation de :

- contact ;
- support ;
- continuité du CM.

---

## 21. Questions ouvertes à trancher après stabilisation du modèle

Les questions suivantes sont importantes, mais ne doivent être décidées qu'après
validation des blocs structurels :

### 21.1 Le push-off agit-il explicitement sur le CM ?

Deux options :

1. push-off implicite via la dynamique globale et le placement du pied ;
2. push-off explicite via une contribution dédiée sur `Vcm` / `Acm`.

Le document de Kuo suggère qu'une redirection active entre jambes existe
réellement. Il est donc probable qu'un simple changement géométrique de support
ne suffise pas à produire un walking convaincant.

Mais cette décision doit être prise après correction :

- du contact ;
- du support ;
- de la transition de charge.

### 21.2 Faut-il garder un pied ponctuel ou introduire heel / toe ?

Le modèle actuel assimile chaque pied à un point.

Ce choix est acceptable pour une première correction du walking, mais il limite :

- la fidélité du support ;
- la lecture du push-off ;
- la précision du landing sur pente.

Décision provisoire :

- rester d'abord en modèle ponctuel ;
- réévaluer seulement après stabilisation du walking structurel.

### 21.3 Quelle variable pilote le début du swing ?

Le trigger futur devra probablement combiner :

- retard du pied arrière ;
- état du support courant ;
- dynamique du CM / vitesse récupérable ;
- possibilité réelle de créer le support suivant.

Mais tant que le support lui-même est mal défini, il est prématuré de figer
ce trigger.

---

## 22. Plan d'implémentation concret

Cette section traduit le redesign en blocs de travail exécutables.

Principe de méthode :

- chaque bloc doit produire un gain structurel visible ;
- chaque bloc doit pouvoir être validé isolément ;
- on évite les refactors transverses massifs tant que les invariants de base
  ne sont pas stabilisés ;
- tant qu'un bloc n'est pas validé, le bloc suivant ne doit pas réinterpréter
  son comportement.

### Bloc 0 — instrumentation minimale avant refonte

Objectif :

- rendre observables les erreurs actuelles avant de modifier la logique.

À faire :

- exposer explicitement, dans la télémétrie et/ou le debug, la différence entre :
  - phase du pied ;
  - contact géométrique ;
  - support actif ;
- afficher / logger la distance normale au terrain ;
- afficher / logger la hauteur du terrain au point de contact ;
- afficher / logger l'état de double support et les poids de charge futurs
  quand ils existeront.

Doit fonctionner :

- on peut voir noir sur blanc quand un pied est "planté" sans contact réel ;
- on peut localiser les frames où le CM saute.

Ce qu'on ne fait pas encore :

- aucune correction lourde de logique ;
- aucun nouveau trigger de pas ;
- aucune refonte du swing.

Validation :

- inspection SDL ;
- captures headless ciblées ;
- outils `analysis/` si nécessaire.

### Bloc 1 — séparation explicite des vérités de pied

Objectif :

- casser la confusion entre phase, contact et support.

À faire :

- faire évoluer le modèle de pied pour distinguer clairement :
  - phase interne ;
  - contact géométrique ;
  - participation au support ;
- faire dériver l'état visuel "pied au sol" de cette vérité ;
- empêcher que `phase == planted` serve encore implicitement de vérité physique.

Doit fonctionner :

- un pied orange touche réellement le terrain ;
- un pied en swing n'est jamais support ;
- un pied peut être en `Landing` sans être encore support.

Ce qu'on ne fait pas encore :

- pas de double support sophistiqué ;
- pas de nouvelle loi de CM ;
- pas de réécriture complète du planner.

Validation :

- test manuel sur terrain plat et pente faible ;
- assertions ciblées sur contact réel vs phase ;
- revue de télémétrie sur quelques scénarios walking.

### Bloc 2 — foot lock strict

Objectif :

- rendre le pied de support géométriquement stable.

À faire :

- introduire une vraie ancre monde pour le pied en `Stance` ;
- faire respecter cette ancre pendant toute la phase de support ;
- empêcher le glissement du pied de support ;
- clarifier le moment exact où l'ancre est créée et détruite.

Doit fonctionner :

- plus de footskate visible en appui ;
- le corps bouge par rapport au pied ;
- la jambe se reconfigure autour de cette contrainte.

Ce qu'on ne fait pas encore :

- pas encore de correction du setpoint CM ;
- pas encore de gestion complète de pente en swing.

Validation :

- inspection visuelle obligatoire ;
- scénario walking simple ;
- mesure de déplacement résiduel du pied de stance.

### Bloc 3 — landing géométriquement valide

Objectif :

- empêcher qu'un timer crée artificiellement un support.

À faire :

- transformer le landing en vraie phase ;
- n'autoriser l'entrée en support qu'après validation géométrique ;
- gérer l'écart entre "fin de durée de swing" et "contact réellement acceptable" ;
- définir la politique de rattrapage minimale si le target est proche mais pas
  encore valide.

Doit fonctionner :

- un pied n'entre pas en support alors qu'il flotte ;
- un pied n'entre pas en support alors qu'il pénètre le terrain ;
- le début du double support correspond à un vrai événement de contact.

Ce qu'on ne fait pas encore :

- pas encore de loi complète de transfert de charge ;
- pas encore de recentrage refondu du CM.

Validation :

- cas de marche sur terrain plat ;
- cas de marche sur pente modérée ;
- lecture frame-by-frame des phases `Swing -> Landing -> Loading`.

### Bloc 4 — swing compatible avec le terrain réel

Objectif :

- faire passer le pied au-dessus du profil réel, pas au-dessus d'une ligne
  abstraite.

À faire :

- faire dépendre la clearance du profil du terrain entre takeoff et target ;
- garantir l'absence de traversée du terrain en montée et descente ;
- conserver une trajectoire temporellement lisse ;
- garder l'IK dans un domaine valable.

Doit fonctionner :

- le pied monte une colline sans la traverser ;
- le pied descend vers une pente sans rester manifestement flottant ;
- les transitions takeoff / landing restent lisibles.

Ce qu'on ne fait pas encore :

- pas encore de refonte complète du comportement du CM ;
- pas encore de tuning biomécanique fin.

Validation :

- scénarios dédiés montée / descente ;
- outil d'analyse de clearance ;
- vérification visuelle de plusieurs pentes et vitesses.

### Bloc 5 — double support explicite

Objectif :

- faire exister la transition de support comme objet logique et mécanique.

À faire :

- introduire la fenêtre de double support comme état/phase réelle ;
- introduire `load_old / load_new` ;
- définir la loi temporelle de transfert de charge ;
- faire dériver le support effectif de ces poids.

Doit fonctionner :

- pas de switch brutal d'un pied à l'autre ;
- le nouveau pied accepte la charge progressivement ;
- l'ancien pied se décharge progressivement.

Ce qu'on ne fait pas encore :

- pas encore la loi définitive du CM sur pente ;
- pas encore le meilleur push-off.

Validation :

- visualisation des poids de charge ;
- cohérence des supports actifs ;
- comportement stable sur terrain plat avant pente.

### Bloc 6 — refonte du comportement du CM à partir du support effectif

Objectif :

- remplacer la logique plane implicite actuelle par une logique dérivée du
  support réel.

À faire :

- définir une référence de support par pied actif ;
- définir `S_eff` ;
- reformuler le setpoint du CM à partir de `S_eff` ;
- garantir la continuité temporelle à chaque transition.

Doit fonctionner :

- plus de recentrage absurde quand les deux pieds sont au sol en pente ;
- plus de pop du CM lors du changement de support ;
- comportement cohérent en montée et en descente.

Ce qu'on ne fait pas encore :

- pas encore l'optimisation esthétique fine ;
- pas encore les raffinements avancés de push-off ou de cadence.

Validation :

- scénarios "flat", "uphill", "downhill" ;
- inspection des courbes `cm_y`, `cm_vy`, `support_*` ;
- absence de discontinuité visible.

### Bloc 7 — trigger de pas révisé sur base saine

Objectif :

- recalibrer le déclenchement des pas une fois contact et support corrigés.

À faire :

- réévaluer la règle "pied arrière trop loin" ;
- réévaluer la condition de récupération ;
- intégrer l'existence du support futur possible ;
- recalibrer longueur et timing du pas avec les nouvelles contraintes.

Doit fonctionner :

- alternance stable ;
- pas de re-trigger pathologique ;
- walking cohérent sur pente modérée.

Ce qu'on ne fait pas encore :

- pas encore running / jumping / falling ;
- pas encore enrichissement complet de la pose secondaire.

Validation :

- scénarios walking existants ;
- nouveaux scénarios de pente ;
- contrôle visuel prolongé.

### Bloc 8 — raffinement biomécanique

Objectif :

- améliorer le naturel une fois la structure correcte.

À faire éventuellement :

- push-off explicite ;
- meilleure loi de clearance ;
- meilleure flexion de genou en réception ;
- raffinement du timing du double support ;
- réglage de la relation vitesse / longueur de pas / cadence.

Règle :

- ce bloc n'est autorisé qu'après stabilisation complète des blocs 1 à 7.

---

## 23. Fichiers logiquement concernés par la refonte

Cette liste n'est pas un engagement absolu, mais une carte probable.

### Noyau locomotion / simulation

- `src/core/character/FootState.h`
- `src/core/character/SupportState.h`
- `src/core/character/CharacterState.h`
- `src/core/character/CharacterState.cpp`
- `src/core/locomotion/StepPlanner.h`
- `src/core/locomotion/StepPlanner.cpp`
- `src/core/simulation/SimulationCore.h`
- `src/core/simulation/SimulationCore.cpp`

### Télémétrie / debug

- `src/core/telemetry/TelemetryRow.h`
- `src/core/telemetry/TelemetryRecorder.cpp`
- `src/debug/DebugUI.cpp`
- `src/debug/DebugUI.h`
- `src/render/DebugOverlayRenderer.cpp`

### Validation

- `src/headless/ScenarioLibrary.cpp`
- `tests/regression/test_headless_scenarios.cpp`
- nouveaux tests unitaires ciblés
- nouveaux outils dans `analysis/` si nécessaire

### Documents

- `doc/LOCOMOTION_SPEC.md`
- `doc/STATE_MODEL.md`
- `doc/TESTING_AND_VALIDATION.md`
- `doc/ROADMAP_locomotion.md`

---

## 24. Stratégie de validation par bloc

Le redesign ne doit pas être validé uniquement "à l'œil".

Pour chaque bloc, on veut trois niveaux de validation :

### 24.1 Validation structurelle

Questions :

- l'état est-il correctement séparé ?
- les invariants sont-ils représentables explicitement ?
- une variable ne joue-t-elle plus deux rôles contradictoires ?

### 24.2 Validation quantitative

Questions :

- peut-on mesurer l'erreur de contact ?
- peut-on mesurer le glissement d'un pied de support ?
- peut-on mesurer les discontinuités du CM ?

Exemples de métriques :

- distance normale pied-sol ;
- dérive d'ancre pendant `Stance` ;
- saut de `cm_y` à la transition de support ;
- nombre de frames incohérentes `contact_geom == false && support_active == true`.

### 24.3 Validation perceptive

Questions :

- le support est-il lisible visuellement ?
- le pied de support glisse-t-il ?
- la descente de pente semble-t-elle continue ?
- le transfert de poids est-il compréhensible ?

Règle :

- aucune étape ne doit être considérée terminée sans validation quantitative
  minimale **et** validation visuelle.

---

## 25. Non-objectifs explicites du redesign immédiat

Pour éviter l'élargissement incontrôlé du scope, les éléments suivants ne font
pas partie du redesign immédiat :

- modélisation détaillée du pied avec heel et toe séparés ;
- reconstruction biomécanique complète de la cheville ;
- running ;
- jumping ;
- falling ;
- correction par IA automatisée ;
- matching à des courbes cliniques exactes de force de réaction au sol.

Le but actuel est plus simple :

- obtenir un walking structurellement correct ;
- particulièrement sur pentes ;
- avec des pieds et un support logiquement sains.

---

## 26. Décision de séquencement recommandée

Séquence recommandée :

1. Bloc 0
2. Bloc 1
3. Bloc 2
4. Bloc 3
5. Bloc 4
6. Bloc 5
7. Bloc 6
8. Bloc 7
9. Bloc 8

Justification :

- il faut d'abord rendre le bug observable ;
- puis corriger la vérité du pied ;
- puis stabiliser le support ;
- puis seulement réinterpréter le CM ;
- et seulement à la fin ajuster le pas lui-même.

En particulier :

- commencer par "un meilleur trigger" avant d'avoir corrigé le contact serait
  une erreur ;
- commencer par "rendre le CM plus joli" avant d'avoir corrigé le support
  serait également une erreur.

---

## 27. Tableau d'exécution par bloc

Cette section reformule les blocs précédents sous une forme plus directement
actionnable.

Format :

- objectif fonctionnel ;
- changements probables ;
- tests / validations à prévoir ;
- critère de sortie.

### Bloc 0 — instrumentation minimale

#### Objectif fonctionnel

Rendre visibles les contradictions du système actuel avant toute correction de
logique.

#### Changements probables

- enrichir l'état observé en debug ;
- enrichir la télémétrie de walking ;
- ajouter des overlays ou logs dédiés :
  - `contact_geom`
  - `support_active`
  - distance normale au terrain
  - hauteur terrain au point du pied

#### Tests / validations à prévoir

- inspection manuelle de `walk_3s`, `walk_left`, `walk_then_stop`
- au moins un scénario de montée
- au moins un scénario de descente

#### Critère de sortie

On peut identifier sans ambiguïté :

- les frames où un pied est visuellement "planté" sans contact réel ;
- les frames où le CM devient discontinu ;
- les phases de support réellement actives.

### Bloc 1 — séparation phase / contact / support

#### Objectif fonctionnel

Distinguer proprement :

- l'intention de la jambe ;
- le contact réel ;
- la participation effective au support.

#### Changements probables

- révision de `FootState`
- révision de `CharacterState`
- mise à jour du calcul de `SupportState`
- mise à jour des overlays et de la télémétrie

#### Tests / validations à prévoir

- assertions de cohérence du type :
  - pas de `support_active == true` si `contact_geom == false`
  - pas de `Swing` considéré comme support
- vérification visuelle des couleurs / états de pied

#### Critère de sortie

Le système n'utilise plus implicitement `phase == planted` comme vérité
physique.

### Bloc 2 — foot lock strict

#### Objectif fonctionnel

Supprimer le glissement du pied pendant le support.

#### Changements probables

- ajout / clarification de l'ancre monde du pied
- logique d'entrée / sortie de `Stance`
- adaptation de l'IK et du rendu si nécessaire

#### Tests / validations à prévoir

- mesure de dérive de l'ancre du pied de stance
- inspection frame-by-frame sur plusieurs cycles de marche
- validation en terrain plat puis pente faible

#### Critère de sortie

La dérive du pied de stance reste bornée à une tolérance très faible sur tout
un cycle d'appui.

### Bloc 3 — landing valide

#### Objectif fonctionnel

Empêcher la création d'un support artificiel à la fin d'un swing.

#### Changements probables

- introduction réelle de la phase `Landing`
- validation géométrique avant entrée en `Loading`
- politique minimale de rattrapage si le landing est proche mais invalide

#### Tests / validations à prévoir

- cas où le pied arrive trop haut
- cas où le pied arrive trop bas
- cas où le timer de swing expire avant contact valide

#### Critère de sortie

Un pied n'entre jamais en support sans satisfaire les conditions minimales de
landing.

### Bloc 4 — swing compatible terrain

#### Objectif fonctionnel

Faire passer le pied libre au-dessus du profil réel du terrain.

#### Changements probables

- révision de la trajectoire de swing
- évaluation locale du profil du terrain entre origine et destination
- révision éventuelle du clearance planning

#### Tests / validations à prévoir

- montée continue
- descente continue
- rupture modérée de pente
- comparaison visuelle des trajectoires avant / après

#### Critère de sortie

Aucune traversée visible du terrain pendant le swing sur les scénarios de base
en plat, montée et descente.

### Bloc 5 — double support explicite

#### Objectif fonctionnel

Rendre la transition d'appui continue et pilotable.

#### Changements probables

- ajout de poids de charge
- ajout d'une phase explicite de double support
- adaptation du calcul du support effectif

#### Tests / validations à prévoir

- visualisation des poids `load_old / load_new`
- cohérence temporelle du passage `Loading -> Stance -> PushOff`
- inspection sur terrain plat

#### Critère de sortie

Le système ne fait plus de switch brutal entre "ancien pied support" et
"nouveau pied support".

### Bloc 6 — refonte du CM depuis le support effectif

#### Objectif fonctionnel

Dériver le comportement du CM du support réel, y compris en pente.

#### Changements probables

- révision du setpoint du CM
- introduction explicite d'une référence de support effective
- abandon des raccourcis implicites fondés sur un sol plat

#### Tests / validations à prévoir

- tracés de `cm_y`, `cm_vy`, `support_*`
- scénarios ciblés de descente
- comparaison visuelle avant / après

#### Critère de sortie

Les cas de descente et de double support sur pente ne produisent plus de pop
visible du CM.

### Bloc 7 — trigger et placement révisés

#### Objectif fonctionnel

Réajuster le pas une fois la base physique redevenue saine.

#### Changements probables

- révision de `shouldStep`
- révision de `planStep`
- éventuelle intégration plus forte de la récupérabilité du support

#### Tests / validations à prévoir

- scénarios walking existants
- nouveaux scénarios de pente
- vérification de l'absence de re-trigger pathologique

#### Critère de sortie

Le système génère des pas stables sans régression majeure sur plat ni en pente
modérée.

### Bloc 8 — raffinement biomécanique

#### Objectif fonctionnel

Améliorer le naturel une fois la structure validée.

#### Changements probables

- push-off explicite
- meilleure loi de clearance
- meilleure réception
- ajustement cadence / longueur de pas / vitesse

#### Tests / validations à prévoir

- comparaison visuelle prolongée
- nouvelles métriques de naturalité
- incidents ciblés si une capture automatique est introduite plus tard

#### Critère de sortie

Le walking paraît plus naturel sans réintroduire de violation structurelle
des blocs 1 à 7.

---

## 28. Plan minimal de tests à créer ou renforcer

Le redesign a besoin d'un noyau de validation dédié. Le minimum recommandé est :

### 28.1 Tests unitaires

À ajouter ou renforcer :

- cohérence `contact_geom` sur terrain plat
- cohérence `contact_geom` sur terrain incliné
- invariants du foot lock
- validation minimale des conditions de landing
- non-pénétration du swing sur un profil de terrain synthétique

### 28.2 Tests de régression

Scénarios cibles :

- `walk_flat`
- `walk_uphill`
- `walk_downhill`
- `walk_then_stop_on_slope`

Métriques minimales :

- pas de `NaN`
- pas de pied support sans contact réel
- pas de dérive excessive du pied de stance
- pas de saut anormal de `cm_y`

### 28.3 Validation visuelle obligatoire

Cas obligatoires :

- terrain plat
- pente montante
- pente descendante
- changement de pente modéré

Ce redesign ne doit pas être validé uniquement sur les tests headless.

---

## 29. Définition pratique du "done" pour la refonte walking

Le redesign du walking sera considéré comme suffisamment réussi quand les
conditions suivantes seront toutes vraies :

1. un pied affiché comme en support touche réellement le terrain ;
2. le pied de stance ne glisse pas visiblement ;
3. le swing ne traverse pas le terrain en montée ni en descente ;
4. le landing ne crée pas artificiellement un support ;
5. le double support existe comme transition réelle ;
6. le CM ne présente plus de pop visible en descente ;
7. les scénarios de walking en pente passent sans incohérence structurelle ;
8. le système reste conceptuellement CM-first.

Tant que ces huit conditions ne sont pas réunies, le walking ne doit pas être
considéré comme "correctement posé".

---

## 30. Bloc 0 — design d'exécution détaillé

Cette section rend le Bloc 0 immédiatement actionnable sans encore modifier la
logique de locomotion elle-même.

But :

- observer les erreurs ;
- localiser les contradictions ;
- préparer les blocs suivants avec des données fiables.

### 30.1 Questions auxquelles le Bloc 0 doit répondre

Après implémentation du Bloc 0, on doit pouvoir répondre précisément à :

1. à quel moment un pied est-il considéré "planté" par le code actuel ?
2. à quel moment ce même pied touche-t-il réellement le terrain ?
3. à quel moment ce pied participe-t-il réellement au support ?
4. à quel moment le CM change-t-il brutalement de référence ?
5. le problème se produit-il :
   - au heel-strike ?
   - au toe-off ?
   - lors du passage en double support ?
   - lors d'une variation locale de pente ?

Tant que ces questions restent floues, corriger le walking est prématuré.

### 30.2 Signaux à rendre visibles

Le Bloc 0 doit rendre visibles, pour chaque frame utile :

#### Par pied

- `phase`
- `foot.pos.x`
- `foot.pos.y`
- `terrain_y_at_foot = h(foot.pos.x)`
- `d_n` : distance normale pied-terrain
- `contact_geom`
- `support_active`
- `is_locked` / présence d'une ancre de stance

#### Au niveau support

- nombre de pieds actifs dans le support
- support simple / double support
- référence de support courante utilisée par le core
- pied dominant actuel

#### Au niveau CM

- `cm_x`, `cm_y`
- `cm_vx`, `cm_vy`
- setpoint vertical courant si disponible
- référence de support utilisée pour le calcul du setpoint

#### Au niveau événements

- début de swing
- fin de swing nominale
- landing accepté
- landing rejeté
- heel-strike
- toe-off
- changement de pied dominant

### 30.3 Colonnes de télémétrie recommandées

Le jeu de télémétrie actuel est utile, mais insuffisant pour cette refonte.

Colonnes candidates à ajouter :

```text
foot_L_phase
foot_R_phase
foot_L_contact_geom
foot_R_contact_geom
foot_L_support_active
foot_R_support_active
foot_L_dn
foot_R_dn
foot_L_ground_y
foot_R_ground_y
support_mode
support_dominant
support_ref_x
support_ref_y
cm_target_y
landing_event
toeoff_event
```

Remarque :

- il n'est pas nécessaire d'ajouter toutes ces colonnes d'un coup ;
- mais il faut au minimum instrumenter ce qui permet de repérer les trois
  problèmes connus :
  - faux contact ;
  - support absurde en pente ;
  - discontinuité du CM.

### 30.4 Overlays / panneaux debug recommandés

Le mode SDL doit montrer explicitement :

#### Overlay pieds

- couleur de phase
- couleur de contact réel
- marqueur d'ancre de stance
- affichage de `d_n`

#### Overlay support

- quel(s) pied(s) participent réellement au support
- support simple vs double support
- pied dominant
- référence de support courante

#### Overlay CM

- position du CM
- setpoint vertical
- delta entre `cm_y` et `cm_target_y`

Objectif :

- voir immédiatement si le système ment sur le contact ;
- voir immédiatement où le CM change de logique.

### 30.5 Scénarios minimum à préparer

Le Bloc 0 a besoin d'un petit noyau de scénarios dédiés, même si leur logique
de validation reste simple au début.

#### S0 — flat_walk_baseline

Objectif :

- observer le comportement nominal sur terrain plat.

À vérifier :

- faux contact éventuel ;
- footskate ;
- continuité du CM en terrain simple.

#### S1 — uphill_walk

Objectif :

- observer le comportement en montée continue.

À vérifier :

- landing sur terrain plus haut ;
- clearance du swing ;
- support pendant transfert de charge.

#### S2 — downhill_walk

Objectif :

- observer le cas prioritaire de la descente.

À vérifier :

- discontinuités du CM ;
- comportement du double support avec pieds à hauteurs différentes ;
- faux support du pied avant / arrière.

#### S3 — slope_transition

Objectif :

- observer le passage terrain plat -> pente ou pente -> plat.

À vérifier :

- changement de référence de support ;
- stabilité du CM ;
- validité du landing au changement de géométrie locale.

### 30.6 Critères de sortie détaillés du Bloc 0

Le Bloc 0 est considéré terminé si :

1. on peut pointer, frame par frame, les faux contacts actuels ;
2. on peut identifier les transitions où le CM saute ;
3. on peut distinguer clairement :
   - phase du pied,
   - contact réel,
   - support actif ;
4. on dispose de scénarios permettant de reproduire :
   - terrain plat,
   - montée,
   - descente,
   - transition de pente.

Le Bloc 0 n'est pas terminé si l'on reste obligé de décrire les bugs par des
phrases vagues du type :

- "ça a l'air faux"
- "le pied semble bizarre"
- "le corps saute à un moment"

À la sortie du Bloc 0, ces bugs doivent être localisables en temps et en état.

### 30.7 Non-objectif explicite du Bloc 0

Le Bloc 0 ne doit pas :

- corriger le walking ;
- changer le trigger principal ;
- réécrire le CM ;
- introduire un nouveau modèle complet de swing.

Le Bloc 0 sert uniquement à rendre le problème mesurable et reproductible.

### 30.8 État d'avancement réel du Bloc 0 dans le dépôt

Le dépôt contient désormais une **première implémentation partielle** du Bloc 0.

Ce qui est déjà en place :

- enrichissement de l'état debug des pieds avec :
  - hauteur locale du terrain ;
  - pente locale du segment de terrain ;
  - écart vertical pied-sol ;
  - écart signé selon la normale locale du terrain ;
  - estimation simple de contact ;
  - drapeau de support actif dérivé de l'état actuel du core ;
- enrichissement de la télémétrie avec :
  - `foot_L_y`, `foot_R_y`
  - `foot_L_terrain_y`, `foot_R_terrain_y`
  - `foot_L_terrain_slope`, `foot_R_terrain_slope`
  - `foot_L_gap_y`, `foot_R_gap_y`
  - `foot_L_gap_n`, `foot_R_gap_n`
  - `foot_L_contact_est`, `foot_R_contact_est`
  - `foot_L_support`, `foot_R_support`
  - `support_mode`
  - `support_ref_x`, `support_ref_y`
  - `cm_target_y`
- panneau debug SDL enrichi pour visualiser, par pied :
  - phase ;
  - support logique ;
  - estimation de contact ;
  - hauteur du pied ;
  - hauteur du terrain ;
  - pente locale ;
  - gap vertical ;
  - gap normal ;
- panneau debug SDL enrichi pour visualiser aussi :
  - le `support_mode` courant (`none/left/right/double`) ;
  - la référence de support actuellement utilisée ;
  - le setpoint vertical courant du CM ;
- overlay de rendu enrichi :
  - marqueur de terrain sous chaque pied ;
  - segment vertical rouge quand le pied est visiblement séparé du terrain ;
- adaptation des tests de régression pour accepter la télémétrie enrichie.

Validation obtenue à ce stade :

- `make test` passe ;
- `make check_architecture` passe ;
- aucune logique lourde de walking n'a encore été modifiée.

Limites connues de cette première itération :

- le "contact" instrumenté n'est pas encore le vrai `contact_geom` cible ;
- il s'agit pour l'instant d'une **estimation géométrique légère** à partir
  d'un pied ponctuel et de la normale locale du segment de terrain ;
- la mesure exposée `gap_n` est déjà plus utile qu'un simple gap vertical,
  mais elle ne remplace pas encore une vraie logique de contact ;
- l'ancre de stance n'est pas encore instrumentée ;
- les scénarios dédiés `uphill/downhill/slope_transition` du Bloc 0 ne sont pas encore ajoutés.

Conclusion pratique :

- le Bloc 0 a commencé ;
- il est déjà utile pour rendre visibles les faux contacts les plus évidents ;
- mais il n'est pas encore terminé au sens strict de cette spécification ;
- la prochaine sous-étape logique est de compléter l'observation avec la
  géométrie locale du terrain et des scénarios de pente dédiés.

### 30.9 Sous-étapes restantes pour clôturer proprement le Bloc 0

Pour terminer le Bloc 0 sans dériver vers une refonte prématurée, l'ordre
recommandé est :

1. exposer le pied dominant si une notion stable de dominance est disponible ;
2. exposer l'ancre de stance dès qu'elle existe réellement dans le modèle ;
3. ajouter au minimum des scénarios de montée et de descente dédiés à
   l'observation ;
4. seulement après cela, déclarer le Bloc 0 terminé et passer au Bloc 1.

---

## 20. Résumé exécutif

Le walking doit être reconstruit dans cet ordre :

1. définir le contact réel du pied ;
2. verrouiller le pied en support ;
3. faire vivre le swing sur le terrain réel ;
4. introduire un double support explicite ;
5. dériver le CM du support actif au lieu de le recentrer sur une hypothèse plane ;
6. seulement ensuite retoucher le trigger et le naturel du pas.

La priorité n'est donc pas "faire un meilleur pas".

La priorité est :

**rendre vraies les notions de contact, de support et de transition de support**.

Sans cela, le walking restera structurellement fragile, surtout en montée et en descente.
