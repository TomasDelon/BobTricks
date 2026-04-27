#pragma once

#include <functional>
#include <iosfwd>
#include <string>
#include <vector>

#include "core/telemetry/TelemetryRow.h"
#include "core/simulation/SimState.h"

/**
 * @file TelemetryRecorder.h
 * @brief Enregistreur de télémétrie avec support d'assertions pour les scénarios headless.
 */

/**
 * @brief Enregistre la télémétrie frame par frame et exécute des assertions de validation.
 *
 * La télémétrie est stockée en mémoire dans un vecteur de `TelemetryRow` et peut
 * être exportée en CSV via `writeCsv()`. Les assertions sont des prédicats nommés
 * évalués sur l'ensemble des lignes après la fin du scénario.
 */
class TelemetryRecorder
{
public:
    /**
     * @brief Ajoute une ligne de télémétrie à partir de l'état de simulation courant.
     * @param state Instantané de simulation à enregistrer.
     */
    void record(const SimState& state);

    /**
     * @brief Écrit toutes les lignes enregistrées au format CSV sur le flux donné.
     * @param out Flux de sortie (fichier ou `std::cout`).
     */
    void writeCsv(std::ostream& out) const;

    /** @brief Accès en lecture seule à toutes les lignes enregistrées. */
    const std::vector<TelemetryRow>& rows() const;

    /** @brief Efface les lignes et les assertions enregistrées. */
    void clear();

    /**
     * @brief Enregistre un prédicat de validation nommé.
     *
     * @param name Nom de l'assertion affiché dans le rapport.
     * @param fn   Fonction qui reçoit toutes les lignes et retourne vrai si le critère est satisfait.
     */
    void addAssertion(const std::string& name,
                      std::function<bool(const std::vector<TelemetryRow>&)> fn);

    /**
     * @brief Évalue toutes les assertions et écrit un rapport sur `report`.
     * @param report Flux de sortie pour le compte-rendu d'assertions.
     * @return Vrai si toutes les assertions passent.
     */
    bool runAssertions(std::ostream& report) const;

private:
    std::vector<TelemetryRow> m_rows;

    struct Assertion {
        std::string name;
        std::function<bool(const std::vector<TelemetryRow>&)> check;
    };
    std::vector<Assertion> m_asserts;
};
