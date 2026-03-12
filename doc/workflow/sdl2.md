
# SDL2

SDL2, acronyme de Simple DirectMedia Layer 2, est une bibliothèque multimédia open-source écrite en C qui fournit une abstraction simple et puissante pour l'accès aux entrées, la gestion des fenêtres, le rendu graphique 2D, et plus encore. Elle est largement utilisée dans le développement de jeux, de logiciels multimédias et d'applications interactives. Voici une présentation succincte de SDL2 :

* Accès aux Entrées : SDL2 permet la gestion des entrées utilisateur, y compris le clavier, la souris, les manettes de jeu, et d'autres périphériques d'entrée.
* Gestion des Fenêtres : SDL2 offre des fonctionnalités pour créer et gérer des fenêtres graphiques. Elle prend également en charge la gestion de plusieurs fenêtres.
* Rendu Graphique 2D : SDL2 offre un moteur de rendu simple pour le dessin 2D. Elle prend en charge le dessin de primitives graphiques, le rendu de textures, et d'autres opérations graphiques de base.
* Son et Musique : SDL2 permet la lecture de sons et de musique. Elle prend en charge divers formats audio et offre une interface simple pour le contrôle de l'audio.
* Contrôle du Temps : SDL2 fournit des fonctions pour la gestion du temps, ce qui facilite la création de jeux ou d'applications avec une gestion du temps précise.
* Portabilité : SDL2 est conçue pour être portable et fonctionne sur plusieurs plates-formes, y compris Windows, macOS, Linux, Android, et d'autres.


## Installation

* Regardez le processus d'installation des outils [ici](./install.md).
* Vous pouvez aussi regarder [ce github](https://github.com/YoCodingJosh/sdl2_cmake_vcpkg).



## Des exemples simples de code

Allez regarder le code qui se trouve dans le [répertoire SDL2_Simple](../SDL2_Simple/) ou dans [l'embryon du jeu Pacman](../Pacman/).

```
#include <SDL.h>
#include <SDL_image.h>

int main(int argc, char* argv[]) {
    // Initialisation de SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Erreur lors de l'initialisation de SDL : %s\n", SDL_GetError());
        return -1;
    }

    // Création de la fenêtre
    SDL_Window* window = SDL_CreateWindow("SDL2 Affichage Image", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Erreur lors de la création de la fenêtre : %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    // Initialisation du module IMG de SDL pour la manipulation d'images
    if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
        printf("Erreur lors de l'initialisation du module IMG : %s\n", IMG_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    // Chargement de l'image
    SDL_Surface* surface = IMG_Load("chemin_vers_votre_image.png");
    if (!surface) {
        printf("Erreur lors du chargement de l'image : %s\n", IMG_GetError());
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    // Création de la texture à partir de la surface
    SDL_Texture* texture = SDL_CreateTextureFromSurface(SDL_GetRenderer(window), surface);
    SDL_FreeSurface(surface);

    // Boucle principale
    SDL_Event e;
    int quit = 0;
    while (!quit) {
        // Gestion des événements
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = 1;
            }
        }

        // Effacement de l'écran
        SDL_RenderClear(SDL_GetRenderer(window));

        // Affichage de la texture
        SDL_RenderCopy(SDL_GetRenderer(window), texture, NULL, NULL);

        // Mise à jour de l'écran
        SDL_RenderPresent(SDL_GetRenderer(window));
    }

    // Libération des ressources
    SDL_DestroyTexture(texture);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
```



## Pour aller plus loin

[Regardez les tutorieux de LazyFoo](https://lazyfoo.net/tutorials/SDL/)
