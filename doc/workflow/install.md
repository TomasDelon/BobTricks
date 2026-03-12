# Installation des librairies et des outils de développement C++

Voici la liste des éléments à installer. Plus bas, il y a le détail pour chaque OS.

* g++/gdb ou le compilateur/débogueur de VisualStudio (windows) ou XCode (MacOS)
* git
* cmake
* doxygen
* vscode (même si vous codez avec un autre IDE, il peut servir)
* valgrind (linux)
* python pour le test du module à rendre (optionnel si vous utilisez le fork avec intégration continue)
* vcpkg est un gestionnaire de paquets C/C++ gratuit qui permet d'acquérir et de gérer des bibliothèques. Il connaît 1500 bibliothèques open source. Sous Linux, on peut s'en sortir avec les packages (apt), mais sinon c'est très bien.


Attention : ne confondez pas Visual Code et Visual Studio.

* Visual Code est un éditeur de code léger et extensible. Il offre une expérience de développement simplifiée et se concentre sur l'édition de code. Il est multi-plateforme.
* Visual Studio est un Environnement de Développement Intégré (IDE) complet : éditeur, compilateur, débogueur, profiler, etc.. Uniquement sous Windows.



## Sous Linux

Sous Linux ou WSL le plus simple est d'installer tous les packages
<code>
  sudo apt-get update -y ; sudo apt-get install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev libsdl2-mixer-dev libpng-dev libglew-dev cmake codeblocks doxygen valgrind build-essential manpages-dev imagemagick
</code>

Puis [installer VSCode](https://code.visualstudio.com/docs/setup/linux).

Sous Linux, on doit pouvoir se passer de VCPKG, mais c'est bien de savoir que cet outil existe.


## Sous Windows

* [Installer VSCode](https://code.visualstudio.com/docs/cpp/config-msvc) 
* ou [VisualStudio](https://visualstudio.microsoft.com/fr/downloads/)
* [Installer CMake](https://cmake.org/download/)
* Pour les librairies, [installer VCPKG](https://vcpkg.io/en/getting-started.html) dans `C:\dev\vcpkg`
  * Le gestionnaire de package sous window `winget` s'améliore ...

### VScode et C++

Il faut installer VSCode et également un compilateur. Sous Windows, sûrement que le plus logique ets de travailler avec le compilateur de Visual Studio. Mais si vous voulez avoir le même comportement que g++/Linux, vous pouvez installer MinGW.

* soit le compilateur de Visual studio "Build Tools for Visual Studio 2022". Suivez l'[installation ici](https://code.visualstudio.com/docs/cpp/config-msvc).
* [soit g++ avec mingw](https://code.visualstudio.com/docs/cpp/config-mingw)

Puis il faut les extensions de VScode : regardez dans la documentation spécifiques à [VSCode](./vscode.md).


### Librairies avec winget
A tester, `winget install libsdl2` ne marche pas encore, mais peut-être ...


### Librairies avec VCPKG

Installer VCPKG, le plus simple est de le mettre dans `C:\dev\vcpkg`.

* Ouvrez un "cmd prompt"
* `mkdir c:\dev`
* `cd c:\dev`
* Cloner VCPKG : `git clone https://github.com/Microsoft/vcpkg.git` ou avec l'interface de votre git (TortoiseGit, GitKraken, DesktopGit)
  * Il faut faire attention ici à la toolchain installée. Sous Windows, le plus simple est d'utiliser le compilateur de Visual Studio
  * Avec vcpkg, il faut regarder autour de "triplet" 
* `.\vcpkg\bootstrap-vcpkg.bat`
* `cd vcpkg`
* `vcpkg integrate install` pour rendre vcpkg et les librairies accessibles depuis les IDE et cmake
* Installez les librairies SDL2. Attention il faut spécifier votre "compilateur" (triplet)
  * Pour mingw : `vcpkg --triplet=x64-mingw-static install sdl2 sdl2-image sdl2-mixer`
  * Pour le MSVCC (visual studio)  : `vcpkg --triplet=x64-windows install sdl2 sdl2-image sdl2-mixer sdl2-ttf`

Il faut ensuite ajouter à la variable %PATH% les chemins vers les librairies `.dll` et vers les versions debug des `.dll`. IMPORTANT, car CMake compile en mode Debug avec les librairies de Debug.

* Appuyer sur la touche "windows" de votre clavier et taper "path" et ouvrez "Modifier les variables d'environnement du système"
* Aller modifier la variable `PATH`
* AJouter `C:\dev\vcpkg\installed\x64-windows\bin` et `C:\dev\vcpkg\installed\x64-windows\debug\bin` (changez le chemin vers vcpkg si besoin)



## Sous MacOS 

* [Installer XCode](https://apps.apple.com/fr/app/xcode/id497799835?mt=12)
* [Installer VCPKG](https://vcpkg.io/en/getting-started)
* [Installer CMake](https://cmake.org/download/)



# Tester l'installation

Pour tester votre installation, le plus simple est de compiler le projet [SDL2_Simple](../SDL2_Simple/).
