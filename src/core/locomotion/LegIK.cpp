#include "core/locomotion/LegIK.h"

#include <cmath>

LegIKResult computeKnee(const Vec2& P, const Vec2& F_target, double L, double facing)
{
    Vec2 F = F_target;
    // Distance bassin → pied.
    const double dx    = F.x - P.x;
    const double dy    = F.y - P.y;
    double       d_leg = std::sqrt(dx * dx + dy * dy);

    // Saturation : éviter la singularité en extension complète.
    static constexpr double EPS = 1e-4;
    const double max_reach = 2.0 * L - EPS;
    if (d_leg > max_reach) {
        // Ramener la position du pied sur la sphère atteignable.
        const double scale = max_reach / d_leg;
        F.x   = P.x + dx * scale;
        F.y   = P.y + dy * scale;
        d_leg = max_reach;
    }
    if (d_leg < EPS) {
        // Cas dégénéré : bassin au-dessus du pied — placer le genou directement vers l'avant.
        return { { P.x + facing * L, P.y }, F };
    }

    // Hauteur du genou au-dessus du milieu de PF (triangle isocèle).
    // À partir de : L² = (d/2)² + h²  →  h = sqrt(L² - (d/2)²)
    const double h_k = std::sqrt(std::max(0.0, L * L - (d_leg * 0.5) * (d_leg * 0.5)));

    // Vecteur unitaire le long de PF et sa perpendiculaire orientée anti-horaire.
    const double ex = (F.x - P.x) / d_leg;
    const double ey = (F.y - P.y) / d_leg;

    // Perpendiculaire (rotation anti-horaire de e_PF de 90°) : n = (-ey, ex)
    double nx = -ey;
    double ny =  ex;

    // Le genou se plie vers l'avant : n doit avoir une composante x positive dans la direction du personnage.
    if (nx * facing < 0.0) {
        nx = -nx;
        ny = -ny;
    }

    // Milieu de PF + décalage le long de la perpendiculaire.
    const Vec2 M    = { (P.x + F.x) * 0.5, (P.y + F.y) * 0.5 };
    const Vec2 knee = { M.x + h_k * nx, M.y + h_k * ny };
    return { knee, F };
}
