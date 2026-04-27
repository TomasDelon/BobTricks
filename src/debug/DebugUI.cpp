#include "debug/DebugUI.h"
#include "app/AudioSystem.h"
#include "core/locomotion/StandingController.h"
#include "core/physics/Geometry.h"

#include <cmath>
#include <cstdio>
#include "imgui.h"

namespace {

static bool sliderDouble(const char* label, double& val, float lo, float hi, const char* fmt = "%.2f")
{
    float v = static_cast<float>(val);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat(label, &v, lo, hi, fmt)) {
        val = static_cast<double>(v);
        return true;
    }
    return false;
}


void renderPhysicsGravityControls(PhysicsConfig& config)
{
    ImGui::Checkbox("##grav_en", &config.gravity_enabled);
    ImGui::SameLine();
    float g = static_cast<float>(config.gravity);
    ImGui::BeginDisabled(!config.gravity_enabled);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("Gravite (m/s²)", &g, 0.f, 20.f, "%.2f"))
        config.gravity = static_cast<double>(g);
    ImGui::EndDisabled();
}

void renderPhysicsVerticalTrackingControls(PhysicsConfig& config)
{
    ImGui::Checkbox("##spring_en", &config.spring_enabled);
    ImGui::SameLine();
    ImGui::BeginDisabled(!config.spring_enabled);
    sliderDouble("vy_max (m/s)",  config.vy_max,  0.1f,  8.0f);
    sliderDouble("d_soft (m)",    config.d_soft,  0.01f, 1.0f, "%.3f");
    sliderDouble("vy_tau (s⁻¹)", config.vy_tau,  1.0f,  60.0f, "%.1f");
    ImGui::EndDisabled();
}

void renderPhysicsLocomotionControls(PhysicsConfig& config)
{
    sliderDouble("Accel (m/s²)",       config.accel,          0.f,  20.f, "%.1f");
    sliderDouble("Max speed (m/s)",    config.walk_max_speed, 0.1f,  6.f);
    sliderDouble("Hold speed (m/s)",   config.hold_speed,     0.0f,  2.f);
    sliderDouble("Impulsion de saut (m/s)", config.jump_impulse,   0.f,  15.f, "%.1f");
}

void renderTerrainGenerationControls(TerrainConfig& config)
{
    ImGui::Separator();
    ImGui::TextDisabled("Generation");
    ImGui::SetNextItemWidth(120.f);
    ImGui::InputInt("Seed", &config.seed);
    sliderDouble("Seg min (m)", config.seg_min, 0.5f, 10.f, "%.1f");
    sliderDouble("Seg max (m)", config.seg_max, 0.5f, 20.f, "%.1f");
}

void renderTerrainAngleControls(TerrainConfig& config)
{
    ImGui::Separator();
    ImGui::TextDisabled("Angles");
    sliderDouble("Small angle (°)", config.angle_small, 0.f, 45.f, "%.1f");
    sliderDouble("Large angle (°)", config.angle_large, 0.f, 60.f, "%.1f");
    sliderDouble("Large prob",      config.large_prob,  0.f,  1.f);
    sliderDouble("Slope max (°)",   config.slope_max,   5.f, 60.f, "%.1f");
}

void renderTerrainHeightControls(TerrainConfig& config)
{
    ImGui::Separator();
    ImGui::TextDisabled("Height bounds");
    sliderDouble("Height min (m)", config.height_min, -10.f, 0.f,  "%.1f");
    sliderDouble("Height max (m)", config.height_max,   0.f, 10.f, "%.1f");
}

void renderTerrainSamplingControls(TerrainSamplingConfig& config)
{
    sliderDouble("w_back (xL)",   config.w_back,    0.1f,  2.0f);
    sliderDouble("w_fwd (xL)",    config.w_fwd,     0.1f,  2.0f);
    sliderDouble("t_look (s)",    config.t_look,    0.0f,  1.0f);
    sliderDouble("tau_slide (s)", config.tau_slide, 0.01f, 1.0f, "%.3f");
    ImGui::SameLine(); ImGui::TextDisabled("endpoint slide lag");
}

void renderSplineControls(SplineRenderConfig& config)
{
    ImGui::Checkbox("Use spline renderer", &config.enabled);
    ImGui::Checkbox("Draw spline under legacy", &config.draw_under_legacy);
    ImGui::Checkbox("Rendre la tete", &config.show_head);
    ImGui::Checkbox("Rendre le torse", &config.show_torso);
    ImGui::Checkbox("Rendre les bras", &config.show_arms);
    ImGui::Checkbox("Rendre les jambes", &config.show_legs);
    ImGui::Checkbox("Afficher la courbe de test", &config.show_test_curve);
    ImGui::Checkbox("Afficher le polygone de controle", &config.show_control_polygon);
    ImGui::Checkbox("Afficher les points echantillonnes", &config.show_sample_points);

    ImGui::Separator();

    ImGui::SetNextItemWidth(180.f);
    ImGui::SliderFloat("stroke_width_px", &config.stroke_width_px, 1.0f, 32.0f, "%.1f");

    ImGui::SetNextItemWidth(180.f);
    ImGui::SliderInt("samples_per_curve", &config.samples_per_curve, 4, 96);
}

void sliderJumpDuration(const char* label, double& val, bool& saveRequested)
{
    if (sliderDouble(label, val, 0.02f, 0.30f, "%.3f s"))
        saveRequested = true;
}

void sliderJumpDepth(const char* label, double& val, bool& saveRequested)
{
    if (sliderDouble(label, val, 0.05f, 0.60f, "%.2f ×L"))
        saveRequested = true;
}

} // fin namespace

AppRequests DebugUI::render(const FrameStats&      stats,
                             SimulationLoop&        simLoop,   SimLoopConfig&   simConfig,
                             Camera2D&              camera,    CameraConfig&    camConfig,
                             CharacterConfig&       charConfig,
                             HeadConfig&            headConfig,
                             ArmConfig&             armConfig,
                             SplineRenderConfig&    splineConfig,
                             PresentationConfig&    presentationConfig,
                             CharacterReconstructionConfig& reconstructionConfig,
                             const CMState&         cmState,   CMConfig&        cmConfig,
                             const CharacterState&  charState,
                             const StandingConfig&  standConfig,
                             PhysicsConfig&         physConfig,
                             TerrainConfig&         terrainConfig,
                             ParticlesConfig&       particlesConfig,
                             AudioConfig&           audioConfig,
                             TerrainSamplingConfig& terrainSamplingConfig,
                             WalkConfig&            walkConfig,
                             JumpConfig&            jumpConfig,
                             const Terrain&         terrain)
{
    AppRequests req;

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const ImVec2 work_pos = viewport->WorkPos;
    const ImVec2 work_size = viewport->WorkSize;
    const ImVec2 margin = {8.0f, 8.0f};
    const float debug_width = std::min(401.0f, std::max(301.0f, work_size.x * 0.34f - 19.0f));
    const float debug_height = std::max(322.0f, work_size.y - 2.0f * margin.y - 38.0f);
    ImGui::SetNextWindowPos({work_pos.x + margin.x, work_pos.y + margin.y}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize({debug_width, debug_height}, ImGuiCond_FirstUseEver);

    ImGui::Begin("Debug");
    renderHelpPanel();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    renderSimLoopPanel(stats, simLoop, simConfig, req.sim_loop, req.step_back);
    renderCameraPanel(camera, camConfig, req.camera);
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    renderParticlesPanel(particlesConfig, req.particles);
    renderAudioPanel(audioConfig, req.audio);
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    renderTerrainPanel(terrainConfig, req.terrain, req.regenerate_terrain);
    renderTerrainSamplingPanel(terrainSamplingConfig, cmConfig, req.terrain);
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    renderPhysicsPanel(physConfig, req.physics);
    renderJumpPanel(jumpConfig, req.jump);
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    renderCMKinematicsPanel(cmState, cmConfig, req.cm, req.clear_trail);
    renderCharacterPanel(charConfig, standConfig, req.character);
    renderReconstructionPanel(reconstructionConfig, req.reconstruction);
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    renderHeadPanel(charState, headConfig, req.head);
    renderTorsoPanel(charState, charConfig, reconstructionConfig, terrain, req.reconstruction);
    renderLegsPanel(charState, charConfig, cmConfig, req.character);
    renderArmsPanel(armConfig, req.arms);
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    renderLocomotionPanel(cmState, charState, charConfig, reconstructionConfig,
                          terrain, walkConfig, req.walk);
    renderBalancePanel(cmState, charState, charConfig, standConfig, cmConfig, req.cm);
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    renderSplinePanel(splineConfig, req.spline);
    renderPresentationPanel(presentationConfig, req.presentation);
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Panel masqué pour l'instant : utile pour des vérifications ponctuelles
    // du modèle de pendule inverse, mais trop spécialisé pour l'UI principale.
    // renderIPTestPanel(cmState, charState, charConfig, standConfig, physConfig, simLoop, req);
    ImGui::End();

    return req;
}

// ─── Boucle de simulation ────────────────────────────────────────────────────

void DebugUI::renderSimLoopPanel(const FrameStats& stats, SimulationLoop& simLoop,
                                  SimLoopConfig& config, bool& saveRequested, bool& stepBack)
{
    if (!ImGui::CollapsingHeader("Boucle de simulation", ImGuiTreeNodeFlags_None))
        return;

    ImGui::Text("FPS courant : %.1f", stats.current_fps);

    ImGui::SetNextItemWidth(200.f);
    ImGui::SliderInt("FPS max", &config.max_fps, 0, 300);

    ImGui::Separator();

    ImGui::Text("dt frame : %.4f s  |  dt fixe : %.4f s",
                stats.frame_dt_s, static_cast<float>(simLoop.getFixedDt()));
    ImGui::Text("Pas sim  : %lu",  simLoop.getTotalStepCount());
    ImGui::Text("Temps sim: %.3f s", simLoop.getSimulationTime());

    ImGui::Separator();

    float timeScale = static_cast<float>(simLoop.getTimeScale());
    ImGui::SetNextItemWidth(200.f);
    if (ImGui::SliderFloat("Echelle du temps", &timeScale, 0.f, 3.f, "%.2fx"))
        simLoop.setTimeScale(static_cast<double>(timeScale));

    if (ImGui::Button("< Retour pas"))
        stepBack = true;

    ImGui::SameLine();
    if (ImGui::Button(simLoop.isPaused() ? "Reprendre" : "Pause"))
        simLoop.togglePaused();

    ImGui::SameLine();
    if (ImGui::Button("Pas suivant >")) {
        simLoop.setPaused(true);
        simLoop.requestSingleStep();
    }

    ImGui::Separator();
    if (ImGui::Button("Sauver la config de boucle"))
        saveRequested = true;
}

// ─── Caméra ──────────────────────────────────────────────────────────────────

void DebugUI::renderCameraPanel(Camera2D& camera, CameraConfig& config, bool& saveRequested)
{
    if (!ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_None))
        return;

    float zoom = static_cast<float>(config.zoom);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("Zoom", &zoom, 0.1f, 5.0f, "%.2fx")) {
        config.zoom = static_cast<double>(zoom);
        const double cur = camera.getZoom();
        if (cur > 1e-9) camera.zoomBy(config.zoom / cur);
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset##zoom")) {
        camera.resetZoom();
        config.zoom = 1.0;
    }

    ImGui::Text("Echelle : %.1f px/m", camera.getPixelsPerMeter());

    ImGui::Separator();

    const Vec2 center = camera.getCenter();
    ImGui::Text("Centre x, y : %.3f m , %.3f m", center.x, center.y);

    ImGui::Checkbox("Suivre CM.x", &config.follow_x);
    ImGui::SameLine(); ImGui::TextDisabled("(Faux = glisser pour deplacer X)");
    ImGui::Checkbox("Suivre CM.y", &config.follow_y);
    ImGui::SameLine(); ImGui::TextDisabled("(Faux = glisser pour deplacer Y)");

    float sx = static_cast<float>(config.smooth_x);
    float sy = static_cast<float>(config.smooth_y);
    float dzx = static_cast<float>(config.deadzone_x);
    float dzy = static_cast<float>(config.deadzone_y);

    ImGui::SetNextItemWidth(200.f);
    if (ImGui::SliderFloat("Lissage X", &sx, 0.f, 5.f, sx <= 0.f ? "instant" : "%.2f s"))
        config.smooth_x = static_cast<double>(sx);

    ImGui::SetNextItemWidth(200.f);
    if (ImGui::SliderFloat("Lissage Y", &sy, 0.f, 5.f, sy <= 0.f ? "instant" : "%.2f s"))
        config.smooth_y = static_cast<double>(sy);

    ImGui::SetNextItemWidth(200.f);
    if (ImGui::SliderFloat("Zone morte X", &dzx, 0.f, 2.f, "%.2f m"))
        config.deadzone_x = static_cast<double>(dzx);

    ImGui::SetNextItemWidth(200.f);
    if (ImGui::SliderFloat("Zone morte Y", &dzy, 0.f, 2.f, "%.2f m"))
        config.deadzone_y = static_cast<double>(dzy);

    ImGui::Separator();
    if (ImGui::Button("Sauver la config camera"))
        saveRequested = true;
}


void DebugUI::renderSplinePanel(SplineRenderConfig& config, bool& saveRequested)
{
    if (!ImGui::CollapsingHeader("Rendu spline", ImGuiTreeNodeFlags_None))
        return;

    renderSplineControls(config);

    ImGui::Separator();
    if (ImGui::Button("Sauver la config spline"))
        saveRequested = true;
}

void DebugUI::renderPresentationPanel(PresentationConfig& config, bool& saveRequested)
{
    if (!ImGui::CollapsingHeader("Presentation", ImGuiTreeNodeFlags_None))
        return;

    ImGui::Checkbox("Afficher le rendu spline", &config.show_spline_renderer);
    ImGui::Checkbox("Afficher la grille de fond", &config.show_background_grid);

    ImGui::Separator();
    ImGui::TextDisabled("Corps");
    ImGui::Checkbox("Afficher le squelette legacy", &config.show_legacy_skeleton);
    ImGui::Checkbox("Afficher les marqueurs legacy", &config.show_character_debug_markers);
    ImGui::Checkbox("Afficher le disque de portee du bassin", &config.show_pelvis_reach_disk);

    ImGui::Separator();
    ImGui::TextDisabled("Overlays CM / locomotion");
    ImGui::Checkbox("Afficher la trainee du CM", &config.show_trail_overlay);
    ImGui::Checkbox("Afficher la reference de sol", &config.show_ground_reference);
    ImGui::Checkbox("Afficher la ligne de support", &config.show_support_line);
    ImGui::Checkbox("Afficher projection / hauteur cible", &config.show_cm_projection);
    ImGui::TextUnformatted("Vecteur vitesse");
    ImGui::SameLine();
    ImGui::RadioButton("Off##pres_vel", &config.velocity_components, 0); ImGui::SameLine();
    ImGui::RadioButton("X##pres_vel",   &config.velocity_components, 1); ImGui::SameLine();
    ImGui::RadioButton("Y##pres_vel",   &config.velocity_components, 2); ImGui::SameLine();
    ImGui::RadioButton("XY##pres_vel",  &config.velocity_components, 3);
    ImGui::TextUnformatted("Vecteur acceleration");
    ImGui::SameLine();
    ImGui::RadioButton("Off##pres_acc", &config.accel_components, 0); ImGui::SameLine();
    ImGui::RadioButton("X##pres_acc",   &config.accel_components, 1); ImGui::SameLine();
    ImGui::RadioButton("Y##pres_acc",   &config.accel_components, 2); ImGui::SameLine();
    ImGui::RadioButton("XY##pres_acc",  &config.accel_components, 3);
    ImGui::SliderFloat("Epaisseur debug", &config.debug_thickness_scale, 1.0f, 6.0f, "x%.1f");
    ImGui::Checkbox("Afficher XCoM / cible de pas", &config.show_xcom_overlay);

    ImGui::Separator();
    ImGui::TextDisabled("Overlays tete / bras / spline");
    ImGui::Checkbox("Afficher debug bras", &config.show_arm_overlay);
    ImGui::Checkbox("Afficher debug spline", &config.show_spline_debug_overlay);

    ImGui::Separator();
    if (ImGui::Button("Sauver la config presentation"))
        saveRequested = true;
}


// ─── Saut ────────────────────────────────────────────────────────────────────

void DebugUI::renderJumpPanel(JumpConfig& config, bool& saveRequested)
{
    if (!ImGui::CollapsingHeader("Saut", ImGuiTreeNodeFlags_None))
        return;

    ImGui::TextDisabled("Preload (accroupissement avant decollage)");
    sliderJumpDuration("Duree course (s)", config.preload_dur_run, saveRequested);
    sliderJumpDuration("Duree marche (s)", config.preload_dur_walk, saveRequested);
    sliderJumpDuration("Duree debout (s)", config.preload_dur_stand, saveRequested);
    sliderJumpDepth("Profondeur course (×L)", config.preload_depth_run, saveRequested);
    sliderJumpDepth("Profondeur marche (×L)", config.preload_depth_walk, saveRequested);
    sliderJumpDepth("Profondeur debout (×L)", config.preload_depth_stand, saveRequested);

    ImGui::Separator();
    ImGui::TextDisabled("Vol");
    sliderJumpDepth("Hauteur de tuck (×L)", config.tuck_height_ratio, saveRequested);

    ImGui::Separator();
    ImGui::TextDisabled("Recuperation a la reception");
    sliderJumpDuration("Duree saut (s)", config.landing_dur_jump, saveRequested);
    sliderJumpDuration("Duree reception marche (s)", config.landing_dur_walk, saveRequested);
    if (sliderDouble("Boost base saut",  config.landing_boost_base_jump,  0.0f, 2.0f)) saveRequested = true;
    if (sliderDouble("Boost echelle saut", config.landing_boost_scale_jump, 0.0f, 3.0f)) saveRequested = true;
    if (sliderDouble("Boost base marche",  config.landing_boost_base_walk,  0.0f, 2.0f)) saveRequested = true;
    if (sliderDouble("Boost echelle marche", config.landing_boost_scale_walk, 0.0f, 3.0f)) saveRequested = true;

    ImGui::Separator();
    if (ImGui::Button("Sauver la config saut"))
        saveRequested = true;
}

// ─── Physique ────────────────────────────────────────────────────────────────

void DebugUI::renderPhysicsPanel(PhysicsConfig& config, bool& saveRequested)
{
    if (!ImGui::CollapsingHeader("Physique", ImGuiTreeNodeFlags_None))
        return;

    renderPhysicsGravityControls(config);

    // Frottement au sol
    float kf = static_cast<float>(config.floor_friction);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("Frottement sol (s⁻¹)", &kf, 0.f, 20.f, "%.2f"))
        config.floor_friction = static_cast<double>(kf);

    ImGui::Separator();

    // Suivi vertical (tanh non lineaire)
    renderPhysicsVerticalTrackingControls(config);

    ImGui::Separator();

    // Locomotion
    renderPhysicsLocomotionControls(config);

    ImGui::TextDisabled("Q / D = gauche / droite    ESPACE = saut");

    ImGui::Separator();
    if (ImGui::Button("Sauver la config physique"))
        saveRequested = true;
}

// ─── Terrain ─────────────────────────────────────────────────────────────────

void DebugUI::renderTerrainPanel(TerrainConfig& config, bool& saveRequested, bool& regenerateRequested)
{
    if (!ImGui::CollapsingHeader("Terrain", ImGuiTreeNodeFlags_None))
        return;

    if (ImGui::Checkbox("Activer le terrain", &config.enabled))
        regenerateRequested = true;

    ImGui::BeginDisabled(!config.enabled);
    renderTerrainGenerationControls(config);
    renderTerrainAngleControls(config);
    renderTerrainHeightControls(config);

    ImGui::EndDisabled();

    ImGui::Separator();
    if (ImGui::Button("Regenerer"))
        regenerateRequested = true;
    ImGui::SameLine();
    if (ImGui::Button("Sauver la config terrain"))
        saveRequested = true;
}

void DebugUI::renderParticlesPanel(ParticlesConfig& config, bool& saveRequested)
{
    if (!ImGui::CollapsingHeader("Particules", ImGuiTreeNodeFlags_None))
        return;

    ImGui::Checkbox("Activer les particules", &config.enabled);
    ImGui::BeginDisabled(!config.enabled);
    ImGui::Checkbox("Activer la poussiere", &config.dust_enabled);
    ImGui::Checkbox("Bouffees d'impact", &config.impact_enabled);
    ImGui::Checkbox("Poussiere de glissade", &config.slide_enabled);
    ImGui::Checkbox("Projection de reception", &config.landing_enabled);

    ImGui::SetNextItemWidth(180.f);
    ImGui::SliderInt("dust_burst_count", &config.dust_burst_count, 0, 32);

    ImGui::SetNextItemWidth(180.f);
    ImGui::SliderFloat("dust_radius_px", &config.dust_radius_px, 1.0f, 24.0f, "%.1f");

    ImGui::SetNextItemWidth(180.f);
    ImGui::SliderFloat("dust_alpha", &config.dust_alpha, 1.0f, 160.0f, "%.1f");

    float dust_lifetime = static_cast<float>(config.dust_lifetime_s);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("dust_lifetime_s", &dust_lifetime, 0.05f, 1.50f, "%.2f"))
        config.dust_lifetime_s = static_cast<double>(dust_lifetime);

    float dust_speed = static_cast<float>(config.dust_speed_mps);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("dust_speed_mps", &dust_speed, 0.0f, 3.0f, "%.2f"))
        config.dust_speed_mps = static_cast<double>(dust_speed);

    float slide_interval = static_cast<float>(config.slide_emit_interval_s);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("slide_emit_interval_s", &slide_interval, 0.02f, 0.25f, "%.2f"))
        config.slide_emit_interval_s = static_cast<double>(slide_interval);

    float landing_scale = static_cast<float>(config.landing_burst_scale);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("landing_burst_scale", &landing_scale, 1.0f, 5.0f, "%.2f"))
        config.landing_burst_scale = static_cast<double>(landing_scale);

    ImGui::EndDisabled();

    ImGui::Separator();
    if (ImGui::Button("Sauver la config particules"))
        saveRequested = true;
}

void DebugUI::renderAudioPanel(AudioConfig& config, bool& saveRequested)
{
    if (!ImGui::CollapsingHeader("Audio", ImGuiTreeNodeFlags_None))
        return;

    float footstep_volume = static_cast<float>(config.footstep_volume);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("footstep_volume", &footstep_volume, 0.0f, 4.0f, "%.2f")) {
        config.footstep_volume = static_cast<double>(footstep_volume);
        saveRequested = true;
    }

    if (ImGui::Checkbox("Activer la musique", &config.music_enabled))
        saveRequested = true;

    float music_volume = static_cast<float>(config.music_volume);
    ImGui::SetNextItemWidth(180.f);
    if (ImGui::SliderFloat("music_volume", &music_volume, 0.0f, 1.0f, "%.2f")) {
        config.music_volume = static_cast<double>(music_volume);
        saveRequested = true;
    }

    const int track_count = AudioSystem::musicTrackCount();
    if (track_count > 0) {
        int track = std::clamp(config.music_track, 0, track_count - 1);
        const char* preview = AudioSystem::musicTrackLabel(track);
        if (ImGui::BeginCombo("Piste musicale", preview)) {
            for (int i = 0; i < track_count; ++i) {
                const bool selected = (i == track);
                if (ImGui::Selectable(AudioSystem::musicTrackLabel(i), selected)) {
                    config.music_track = i;
                    saveRequested = true;
                }
                if (selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
    } else {
        ImGui::TextDisabled("Aucune piste trouvee dans data/audio/music/");
    }

    ImGui::Separator();
    if (ImGui::Button("Sauver la config audio"))
        saveRequested = true;
}

void DebugUI::renderHelpPanel()
{
    if (!ImGui::CollapsingHeader("Aide", ImGuiTreeNodeFlags_None))
        return;

    ImGui::Separator();
    ImGui::TextWrapped("Clavier");
    ImGui::BulletText("Q / D : aller a gauche / droite");
    ImGui::BulletText("Shift : courir");
    ImGui::BulletText("Espace : sauter");
    ImGui::BulletText("P : basculer le mode presentation");
    ImGui::BulletText("Echap : quitter");

    ImGui::Separator();
    ImGui::TextWrapped("Souris");
    ImGui::BulletText("Molette : zoom camera");
    ImGui::BulletText("Glisser gauche dans le vide : deplacer la camera");
    ImGui::BulletText("Glisser gauche sur un pied : deplacer ce pied");
    ImGui::BulletText("Glisser gauche sur une main : deplacer cette main");
    ImGui::BulletText("Clic droit sur un pied : epingler ou desepingler ce pied");
    ImGui::BulletText("Clic droit sur une main : epingler ou desepingler cette main");
    ImGui::BulletText("Glisser droit pres du CM : fixer la vitesse du centre de masse");

    ImGui::Separator();
    ImGui::TextWrapped("Notes");
    ImGui::BulletText("Le mode presentation a ses propres toggles d'affichage.");
    ImGui::BulletText("Les mains et les pieds ne peuvent etre saisis que si la cible est assez proche.");
}

// ─── Echantillonnage du terrain ──────────────────────────────────────────────

void DebugUI::renderTerrainSamplingPanel(TerrainSamplingConfig& config, CMConfig& cmConfig, bool& saveRequested)
{
    if (!ImGui::CollapsingHeader("Echantillonnage du terrain", ImGuiTreeNodeFlags_None))
        return;

    renderTerrainSamplingControls(config);

    ImGui::Separator();
    ImGui::Checkbox("Afficher la reference de sol", &cmConfig.show_ground_reference);

    ImGui::Separator();
    if (ImGui::Button("Sauver la config d'echantillonnage"))
        saveRequested = true;
}

// ─── Test de completion du pendule inverse ───────────────────────────────────

void DebugUI::renderIPTestPanel(const CMState&        cmState,
                                const CharacterState& charState,
                                const CharacterConfig& charConfig,
                                const StandingConfig& standConfig,
                                const PhysicsConfig&  physConfig,
                                SimulationLoop&       simLoop,
                                AppRequests&         req)
{
    if (!ImGui::CollapsingHeader("Test de completion du pendule inverse"))
        return;

    const double L         = charConfig.body_height_m / 5.0;
    const double nominal_y = computeNominalY(L, standConfig.d_pref, charConfig.cm_pelvis_ratio);
    const double omega0    = std::sqrt(physConfig.gravity / nominal_y);
    const double mu        = physConfig.floor_friction;

    ImGui::TextDisabled("Test : prediction du pendule inverse vs physique reelle.");
    ImGui::TextDisabled("Necessite des pieds initialises.");
    ImGui::Separator();

    // Bouton de lancement — necessite des pieds initialises
    if (!charState.feet_initialized)
        ImGui::BeginDisabled();

    if (ImGui::Button("Lancer  [CM = pied G + 0.3L,  vx = 0]") && charState.feet_initialized) {
        const FootState& stance_foot = charState.foot_left;
        const double stance_x  = stance_foot.pos.x;
        const double facing    = charState.facing;
        const double x0_rel    = 0.3 * L;
        const double target_x  = stance_x + facing * x0_rel;

        const double disc = std::sqrt(mu*mu + 4.0*omega0*omega0);
        const double r1   = (-mu + disc) * 0.5;
        const double r2   = (-mu - disc) * 0.5;
        const double v0   = 0.0;
        const double C1   = (v0 - r2*x0_rel) / (r1 - r2);
        const double C2   = (r1*x0_rel - v0) / (r1 - r2);

        m_ipTest = { true,
                     simLoop.getSimulationTime(),
                     stance_x, facing,
                     x0_rel, v0,
                     omega0, mu, r1, r2, C1, C2 };

        req.ip_test_launch = true;
        req.ip_test_cm_x   = target_x;
        req.ip_test_cm_vx  = 0.0;
    }

    if (!charState.feet_initialized) {
        ImGui::EndDisabled();
        ImGui::SameLine();
        ImGui::TextColored({1,0.4f,0,1}, "Pieds non initialises");
    }

    if (m_ipTest.active && ImGui::Button("Arreter le test"))
        m_ipTest.active = false;

    if (!m_ipTest.active) return;

    ImGui::Separator();

    const double t = simLoop.getSimulationTime() - m_ipTest.t_start;
    const double exp1   = std::exp(m_ipTest.r1 * t);
    const double exp2   = std::exp(m_ipTest.r2 * t);
    const double x_pred = m_ipTest.C1 * exp1 + m_ipTest.C2 * exp2;
    const double v_pred = m_ipTest.C1 * m_ipTest.r1 * exp1
                        + m_ipTest.C2 * m_ipTest.r2 * exp2;

    const double x_rel_act = (cmState.position.x - m_ipTest.stance_x) * m_ipTest.facing;
    const double v_act     = cmState.velocity.x * m_ipTest.facing;

    const double dx_pred = x_pred - m_ipTest.x0_rel;
    const double dx_act  = x_rel_act - m_ipTest.x0_rel;

    ImGui::Text("t = %.3f s", t);
    ImGui::Text("omega0 = %.3f rad/s   mu = %.2f s^-1", omega0, mu);
    ImGui::Separator();

    ImGui::TextColored({0.4f,1,0.4f,1}, "Analytique (IP + frottement) :");
    ImGui::Text("  x_pred = appui %+.4f m   delta = %+.4f m", x_pred, dx_pred);
    ImGui::Text("  v_pred =       %+.4f m/s", v_pred);

    ImGui::TextColored({1,0.8f,0.2f,1}, "Mesure :");
    ImGui::Text("  x_act  = appui %+.4f m   delta = %+.4f m", x_rel_act, dx_act);
    ImGui::Text("  v_act  =       %+.4f m/s", v_act);

    const double err_x = x_rel_act - x_pred;
    const double err_v = v_act - v_pred;
    ImGui::Text("  error  x = %+.4f m   v = %+.4f m/s", err_x, err_v);
    ImGui::Separator();

    if (dx_act > 0.001)
        ImGui::TextColored({0.2f,1,0.2f,1}, "PASS  le pendule inverse pousse bien le CM vers l'avant  (delta_x = %+.4f m)", dx_act);
    else if (dx_act > -0.001)
        ImGui::TextColored({1,1,0.2f,1}, "WAIT  le CM ne bouge pas encore...");
    else
        ImGui::TextColored({1,0.2f,0.2f,1}, "FAIL  le CM recule !");
}
