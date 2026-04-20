#pragma once

#include "config/AppConfig.h"
#include <string>

/**
 * @file ConfigIO.h
 * @brief Lecture et écriture de la configuration au format INI.
 */

/**
 * @brief Fonctions de sérialisation/désérialisation de `AppConfig` vers un fichier INI.
 *
 * Le format utilisé est un INI simple avec sections `[nom_section]` et paires
 * `clé = valeur`. Les clés inconnues sont ignorées lors du chargement, ce qui
 * permet de rester compatible avec des fichiers de configuration plus anciens.
 */
namespace ConfigIO
{
    /**
     * @brief Charge un fichier INI et remplit la configuration.
     *
     * Les valeurs manquantes conservent leur valeur par défaut dans `config`.
     *
     * @param path   Chemin vers le fichier `.ini`.
     * @param config Structure de configuration à remplir (modifiée en place).
     * @return Vrai si le fichier a pu être ouvert et parsé sans erreur fatale.
     */
    bool load(const std::string& path, AppConfig& config);

    /**
     * @brief Sérialise la configuration courante dans un fichier INI.
     *
     * @param path   Chemin de destination (créé ou écrasé).
     * @param config Configuration à sauvegarder.
     * @return Vrai si l'écriture a réussi.
     */
    bool save(const std::string& path, const AppConfig& config);
}
