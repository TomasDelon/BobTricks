#pragma once

#include <cmath>

/** @brief État du polygone de support sagittal réduit à deux appuis ponctuels. */
struct SupportState {
    // Intervalo de soporte en 1D (sagital).
    // Derivado de pos.x de los pies plantados — nunca de targets.
    double x_left  = 0.0;
    double x_right = 0.0;

    // Altura del terreno bajo cada pie plantado.
    // Necesario para IK y setpoint de CM en terreno no plano.
    double y_left  = 0.0;
    double y_right = 0.0;

    bool left_planted  = false;
    bool right_planted = false;

    // Derivados
    double center()        const { return 0.5 * (x_left + x_right); }
    double width()         const { return std::abs(x_right - x_left); }
    double ground_center() const { return 0.5 * (y_left + y_right); }
    bool   both_planted()  const { return left_planted && right_planted; }
};

// NOTA (V4 simplificación): soporte puntual en X.
// Cada pie contribuye con un único punto (pos.x), no con un intervalo heel/toe.
// MoS se calcula respecto a los dos puntos de contacto, no a un rectángulo real.
// Cuando se añadan segmentos de pie, reemplazar x_left/x_right por
// x_heel_left/x_toe_right (extremos del polígono de soporte).
