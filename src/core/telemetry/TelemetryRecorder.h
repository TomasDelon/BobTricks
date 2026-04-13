#pragma once

#include <functional>
#include <iosfwd>
#include <string>
#include <vector>

#include "core/telemetry/TelemetryRow.h"
#include "core/simulation/SimState.h"

/**
 * @brief Enregistre la télémétrie et exécute des assertions de scénario.
 */
class TelemetryRecorder
{
public:
    /** @brief Ajoute une ligne à partir de l'état courant. */
    void record(const SimState& state);

    /** @brief Écrit la télémétrie au format CSV. */
    void writeCsv(std::ostream& out) const;

    /** @brief Accès en lecture seule à toutes les lignes enregistrées. */
    const std::vector<TelemetryRow>& rows() const { return m_rows; }

    /** @brief Efface les lignes et les assertions enregistrées. */
    void clear();

    /** @brief Ajoute une assertion évaluée après l'exécution du scénario. */
    void addAssertion(const std::string& name,
                      std::function<bool(const std::vector<TelemetryRow>&)> fn);

    /** @brief Exécute toutes les assertions enregistrées. */
    bool runAssertions(std::ostream& report) const;

private:
    std::vector<TelemetryRow> m_rows;

    struct Assertion {
        std::string name;
        std::function<bool(const std::vector<TelemetryRow>&)> check;
    };
    std::vector<Assertion> m_asserts;
};
