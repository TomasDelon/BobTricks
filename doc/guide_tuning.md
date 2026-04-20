# Guide de réglage des paramètres — BobTricks

Ce guide explique **comment chaque paramètre affecte le comportement visible**
et donne des conseils de réglage pratiques. Les paramètres sont regroupés par
problème à résoudre, pas par section INI.

Tous les paramètres sont modifiables en temps réel dans le debug UI sans
recompilation.

---

## Physique de base (`[physics]`)

| Paramètre | Effet | Plage typique |
|-----------|-------|---------------|
| `gravity` | Plus élevé = mouvement plus « lourd », bob plus prononcé | 5–15 m/s² |
| `accel` | Réactivité aux touches ← → | 3–10 m/s² |
| `walk_max_speed` | Vitesse de marche maximale | 1–2,5 m/s |
| `floor_friction` | Glissement à l'arrêt des touches | 2–8 s⁻¹ |
| `stop_speed` | En-dessous de cette vitesse, le personnage s'arrête net | 0,02–0,10 m/s |

**Recette glissement réaliste :** `friction ≈ 4`, `stop_speed ≈ 0,05`.
**Recette physique lune :** `gravity ≈ 1,6`, `accel ≈ 3`, `friction ≈ 1,5`.

---

## Hauteur et bob vertical (`[walk]`)

Le bob vertical suit le modèle de pendule inversé. Il est contrôlé par :

| Paramètre | Effet | Conseil |
|-----------|-------|---------|
| `leg_flex_coeff` | Fléchissement du genou en appui — augmenter = plus de bob | 0,0–0,15 |
| `bob_scale` | Multiplicateur de l'amplitude du bob IP | 1–5 |
| `bob_amp` | Plafond de la descente du CM (évite les genoux au sol) | 0,05–0,25·L |
| `cm_height_offset` | Décalage fixe de la hauteur cible | −0,1 à +0,1 m |
| `max_speed_drop` | Accroupissement à haute vitesse | 0,05–0,25·L |

**Trop de bob :** réduire `bob_scale` et `leg_flex_coeff`.
**Marche trop rigide :** augmenter `bob_scale` à 3–4, `leg_flex_coeff` à 0,08.

---

## Cadence et longueur de pas (`[walk]`)

| Paramètre | Effet | Plage typique |
|-----------|-------|---------------|
| `step_speed` | Vitesse d'avancement de `swing_t` — plus élevé = pas plus rapide | 4–8 steps/s |
| `stability_margin` | Distance d'avance du pied devant ξ — augmenter = pas plus larges | 1,0–2,0·L |
| `d_rear_max` | Distance max du pied arrière avant déclenchement normal | 1,0–2,0·L |
| `xcom_scale` | Poids du lookahead dans ξ (0 = position pure, 1 = capture point) | 0,3–0,7 |
| `double_support_time` | Temps minimum entre deux pas | 0,03–0,10 s |

**Pas trop courts :** augmenter `stability_margin`.
**Personnage qui « court sur place » :** réduire `step_speed`, augmenter `d_rear_max`.
**Instabilité lors d'une accélération rapide :** augmenter `xcom_scale` vers 0,7.

---

## Comportement en descente (`[walk]`)

| Paramètre | Effet |
|-----------|-------|
| `downhill_crouch_max` | Accroupissement maximal en descente (`[×L]`) |
| `downhill_crouch_tau` | Rapidité d'engagement de l'accroupissement (s) |
| `downhill_relax_tau` | Rapidité de relâchement en revenant au plat (s) |
| `downhill_step_bonus` | Pas plus longs en descente pour garder l'équilibre (`[×L]`) |

**Personnage qui glisse en descente :** augmenter `downhill_step_bonus` et réduire
`downhill_crouch_tau`.

---

## Morphologie du personnage (`[character]`)

| Paramètre | Effet | Défaut |
|-----------|-------|--------|
| `body_height_m` | Taille totale → calibre tout le reste via `L = H/5` | 1,80 m |
| `cm_pelvis_ratio` | Hauteur relative du CM au-dessus du bassin | 0,75 |

Modifier `body_height_m` redimensionne **tout** (hauteur nominale, portées,
forces) de façon cohérente. C'est le seul paramètre de mise à l'échelle global.

---

## Reconstruction du torse (`[reconstruction]`)

| Paramètre | Effet |
|-----------|-------|
| `theta_max_deg` | Lean maximal à haute vitesse (en degrés) |
| `v_ref` | Vitesse de demi-saturation du lean (`tanh(vx/v_ref)`) |
| `tau_lean` | Constante de temps du filtre de lean (s) — plus grand = plus paresseux |
| `hunch_current_deg` | Courbure statique du dos vers l'avant |
| `slope_lean_factor` | Fraction de la pente du terrain transférée au torse |

**Lean trop violent :** augmenter `tau_lean` et réduire `theta_max_deg`.
**Personnage qui ne penche pas assez :** réduire `v_ref` (saturation plus tôt).

---

## Bras (`[arms]`)

Les bras oscillent en arc dans un repère local au torse. L'arc est défini par
ses angles de début et de fin en marche et en course.

| Paramètre | Effet |
|-----------|-------|
| `walk_front_hand_start/end_deg` | Arc du bras avant en marche (degrés dans le repère torse) |
| `walk_back_hand_start/end_deg` | Arc du bras arrière en marche |
| `walk_hand_phase_speed_scale` | Fraction de `step_speed` utilisée comme cadence des bras |
| `walk_hand_speed_arc_gain` | Réduction de l'amplitude à basse vitesse |
| `walk_hand_phase_response` | Rapidité de démarrage de l'oscillation (s⁻¹) |
| `walk_hand_phase_friction` | Amortissement à l'arrêt des touches (s⁻¹) |

**Bras qui s'agitent trop vite :** réduire `walk_hand_phase_speed_scale`.
**Bras qui restent raides à l'arrêt :** réduire `walk_hand_phase_friction`.

---

## Course (`[run]`)

| Paramètre | Effet |
|-----------|-------|
| `max_speed` | Vitesse maximale en course (m/s) |
| `accel_factor` | Multiplicateur de l'accélération par rapport à la marche |
| `step_speed` | Vitesse de swing en course (plus élevé = foulée plus rapide) |
| `cadence_spm_min/max` | Plage de cadence en pas par minute (ex. 162–176 spm) |
| `stride_len` | Longueur de foulée cible à vitesse max (`[×L]`) |
| `leg_flex_coeff` | Compression de jambe en course → amplitude verticale |
| `blend_tau` | Durée de transition marche→course (s) |

**Transition marche→course trop brusque :** augmenter `blend_tau`.
**Foulée trop petite :** augmenter `stride_len` et `stability_margin`.

---

## Saut (`[jump]`)

| Paramètre | Effet |
|-----------|-------|
| `jump_impulse` | Vitesse verticale initiale (m/s) — directement dans `[physics]` |
| `preload_dur_*` | Durée de l'accroupissement avant décollage selon le mode |
| `preload_depth_*` | Profondeur de l'accroupissement (`[×L]`) |
| `tuck_height_ratio` | Remontée maximale des pieds en vol (`[×L]`) |
| `landing_dur_*` | Durée de la récupération à l'atterrissage |

**Saut qui fait trembler :** réduire `jump_impulse` ou augmenter `landing_dur_jump`.
**Personnage qui ne ramasse pas ses pieds :** augmenter `tuck_height_ratio`.

---

## Terrain procédural (`[terrain]`)

| Paramètre | Effet |
|-----------|-------|
| `enabled` | Active/désactive le terrain (plat si `false`) |
| `seed` | Graine du générateur pseudo-aléatoire |
| `seg_min/max` | Longueur minimale/maximale des segments plats (m) |
| `angle_small/large` | Angles possibles aux jonctions (°) |
| `large_prob` | Probabilité d'un grand angle à chaque jonction |
| `slope_max` | Pente maximale autorisée (°) |
| `height_min/max` | Bornes de hauteur du terrain (m) |

**Terrain trop monotone :** augmenter `large_prob` et `angle_large`.
**Personnage qui bloque sur les pentes :** réduire `slope_max` ou augmenter
`h_clear_slope_factor` dans `[walk]`.

---

## Référence de sol (`[terrain_sampling]`)

| Paramètre | Effet |
|-----------|-------|
| `w_back` | Distance arrière de la fenêtre d'échantillonnage (`[×L]`) |
| `w_fwd` | Distance avant de base (`[×L]`) |
| `t_look` | Lookahead vitesse ajouté à la fenêtre avant (s) |
| `tau_slide` | Constante de temps du glissement des extrémités (s) |

**CM qui oscille sur terrain bosselé :** augmenter `tau_slide`.
**CM qui réagit trop lentement aux pentes :** réduire `tau_slide`.

---

## Caméra (`[camera]`)

| Paramètre | Effet |
|-----------|-------|
| `follow_x/y` | Active le suivi selon chaque axe |
| `smooth_x/y` | Constante de temps du lissage (0 = instantané) |
| `deadzone_x/y` | Zone morte en mètres monde — la caméra ne bouge pas à l'intérieur |
| `zoom` | Zoom initial |

**Caméra qui tremble avec le bob :** augmenter `deadzone_y` à 0,20–0,30 m.
**Caméra qui « lag » trop :** réduire `smooth_x`.

---

## Rendu spline (`[spline_render]`)

| Paramètre | Effet |
|-----------|-------|
| `enabled` | Active les courbes de Bézier épaisses |
| `draw_under_legacy` | Superpose le rendu legacy par-dessus les splines |
| `stroke_width_px` | Épaisseur des traits (px) |
| `samples_per_curve` | Lissage des courbes (24 = bon compromis) |
| `show_control_polygon` | Affiche les tangentes de contrôle (debug) |

---

## Workflow de réglage recommandé

1. **Stabiliser d'abord la marche plate** : terrain désactivé, régler `step_speed`,
   `stability_margin`, `bob_scale`.
2. **Tester en pente légère** (10°) : ajuster `h_clear_slope_factor` et
   `downhill_crouch_max`.
3. **Activer la course** : régler `run.max_speed`, `blend_tau`, vérifier la
   transition.
4. **Tester le saut** : régler `jump_impulse`, `preload_depth_walk/run`.
5. **Ajuster le visuel** : lean, hunch, bras, rendu spline.
6. **Stress test** : terrain maximal (`slope_max = 25°`, `large_prob = 0,5`),
   vérifier l'absence de chute après 60 secondes.
