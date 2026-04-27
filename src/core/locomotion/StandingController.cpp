#include "core/locomotion/StandingController.h"

#include <cmath>
#include <algorithm>

static double distance2D(const Vec2& a, const Vec2& b)
{
    const double dx = b.x - a.x;
    const double dy = b.y - a.y;
    return std::sqrt(dx * dx + dy * dy);
}

bool StandingDiag::valid() const
{
    return c1 && c2 && c3 && c4 && c5;
}

std::optional<double> computeStandingCMTarget(const SupportState&   support,
                                              const CharacterConfig& char_cfg)
{
    const double L      = char_cfg.body_height_m / 5.0;
    const double r      = 2.0 * L;   // portée totale de la jambe — cohérente avec computeNominalY
    const double half_d = support.width() * 0.5;

    // La géométrie debout est impossible lorsque l'écart entre les pieds dépasse la portée totale de la jambe.
    // Retourner nullopt force l'appelant à changer de régime plutôt que d'utiliser
    // une cible dégradée silencieusement qui plaquerait le CM au sol.
    if (half_d >= r) return std::nullopt;

    const double h_pelvis = std::sqrt(r * r - half_d * half_d);
    return support.ground_center() + h_pelvis + char_cfg.cm_pelvis_ratio * L;
}

StandingDiag diagStanding(const CMState&        cm,
                           double                xcom,
                           const Vec2&           pelvis,
                           const SupportState&   support,
                           const FootState&      foot_L,
                           const FootState&      foot_R,
                           const CharacterConfig& char_cfg,
                           const StandingConfig&  stand_cfg)
{
    StandingDiag diag;

    const double L         = char_cfg.body_height_m / 5.0;
    const double max_reach = 2.0 * L;   // portée totale de la jambe — cohérente avec computeStandingCMTarget

    diag.d = support.width();

    // Critère 1 : les deux pieds sont en appui
    diag.c1 = support.both_planted();

    // Critère 2 : séparation des pieds dans [d_min, d_max] (×L).
    // Une tolérance de 0.03L sur la borne supérieure correspond à la zone morte
    // du déclencheur géométrique (shouldStep utilise ±0.03L), donc il n'existe
    // pas de trou où c2 échoue sans qu'aucun pas ne se déclenche.
    static constexpr double EPS_C2 = 0.03;
    diag.c2 = (diag.d >= stand_cfg.d_min * L)
           && (diag.d <= stand_cfg.d_max * L + EPS_C2 * L);

    // Critère 3 : le CoM extrapolé (xcom = cm.x + vx/omega0) est dans le support.
    // Utiliser xcom plutôt que cm.position.x évite des faux négatifs lorsque le CM
    // est momentanément déplacé mais que le CoM extrapolé reste dans le support.
    diag.c3 = (xcom >= support.x_left) && (xcom <= support.x_right);

    // Critère 4 : portée de chaque jambe = |pelvis - foot| <= 2L.
    // Avec la hauteur nominale corrigée géométriquement (bassin à h_pelvis_max, et non 2L),
    // les jambes sont complètement tendues sans être sur-étendues à l'écart d_pref.
    diag.reach_L = distance2D(pelvis, foot_L.pos);
    diag.reach_R = distance2D(pelvis, foot_R.pos);
    diag.c4 = (diag.reach_L <= max_reach) && (diag.reach_R <= max_reach);

    // Critère 5 : vitesse horizontale inférieure à eps_v
    diag.c5 = (std::abs(cm.velocity.x) <= stand_cfg.eps_v);

    return diag;
}
