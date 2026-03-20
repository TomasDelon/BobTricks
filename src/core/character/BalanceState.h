#pragma once

struct BalanceState {
    double omega0 = 0.0;  // sqrt(g / h_ref)              — frecuencia del péndulo
    double xcom   = 0.0;  // xi = x_CM + xdot_CM / omega0  — XCoM (Hof 2008)
    double mos    = 0.0;  // min(xi - x_left, x_right - xi) — Margin of Support
};
