#pragma once

#include "config/AppConfig.h"
#include <string>

/**
 * @brief Fonctions de chargement et de sauvegarde de la configuration INI.
 */
namespace ConfigIO
{
    /**
     * @brief Charge un fichier INI dans la configuration donnée.
     * @param path   Chemin du fichier INI à lire.
     * @param config Configuration cible remplie à partir du fichier.
     * @return `true` si le chargement a réussi.
     */
    bool load(const std::string& path, AppConfig& config);

    /**
     * @brief Sauvegarde la configuration courante dans un fichier INI.
     * @param path   Chemin du fichier INI à écrire.
     * @param config Configuration source à sérialiser.
     * @return `true` si l'écriture a réussi.
     */
    bool save(const std::string& path, const AppConfig& config);
}
