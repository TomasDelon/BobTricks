// Logs land_target.y vs terrain.height_at(land_target.x) at heel-strike moment.
// Also logs foot.pos.y vs terrain at foot.pos.x throughout swing.

#include <cstdio>
#include "config/AppConfig.h"
#include "config/ConfigIO.h"
#include "core/character/CharacterState.h"
#include "core/character/FootState.h"
#include "core/simulation/InputFrame.h"
#include "core/simulation/SimulationCore.h"

int main()
{
    AppConfig cfg;
    ConfigIO::load("data/config.ini", cfg);

    cfg.terrain.enabled     = true;
    cfg.terrain.seed        = 7;
    cfg.terrain.angle_large = 25.0;
    cfg.terrain.large_prob  = 0.8;

    SimulationCore core(cfg);

    ScenarioInit init;
    init.cm_pos       = { 0.0, 0.9 };
    init.cm_vel       = { 1.5, 0.0 };
    init.terrain_seed = 7;
    core.reset(init);

    const double dt       = cfg.sim_loop.fixed_dt_s;
    const double duration = 5.0;
    const int    n_steps  = static_cast<int>(duration / dt);

    // Print heel-strike events only
    printf("t,foot,land_target_x,land_target_y,terrain_at_target_x,gap_at_landing\n");

    for (int i = 0; i < n_steps; ++i) {
        const double t = i * dt;

        const SimState&       s_before = core.state();
        const StepPlan        plan_before = s_before.character.step_plan;

        InputFrame input;
        input.key_right = (t < 3.0);
        core.step(dt, input);

        const SimState&       s    = core.state();
        const CharacterState& ch   = s.character;

        // Detect heel-strike: step was active before, not active now
        if (plan_before.active && !ch.step_plan.active) {
            const bool        swing_right  = plan_before.move_right;
            const FootState&  foot         = swing_right ? ch.foot_right : ch.foot_left;
            const double      terrain_here = core.terrain().height_at(foot.pos.x);
            const double      gap          = foot.pos.y - terrain_here;

            printf("%.4f,%s,%.4f,%.4f,%.4f,%.6f\n",
                   t,
                   swing_right ? "R" : "L",
                   plan_before.land_target.x,
                   plan_before.land_target.y,
                   terrain_here,
                   gap);
        }
    }

    return 0;
}
