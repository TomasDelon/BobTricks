#pragma once

/**
 * \brief Initialise l'application SDL et ses ressources principales.
 *
 * \return `true` si l'initialisation reussit, `false` sinon.
 */
bool appInit();

/**
 * \brief Execute une iteration de la boucle principale.
 *
 * Cette fonction traite les evenements SDL, met a jour l'etat minimal de
 * l'application, puis effectue le rendu de la frame courante.
 *
 * \return `true` tant que l'application doit continuer a tourner, `false`
 * sinon.
 */
bool appStep();

/**
 * \brief Libere proprement les ressources SDL de l'application.
 */
void appShutdown();
