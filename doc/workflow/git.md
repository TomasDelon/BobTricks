# Git

Git est un système de gestion de version décentralisé (DVCS), conçu pour suivre les modifications dans les fichiers source d'un projet au fil du temps. Il a été créé par Linus Torvalds en 2005 pour le développement du noyau Linux, mais il est depuis devenu l'un des outils de gestion de version les plus populaires et largement utilisés dans le développement logiciel.


## Installation de git sur machines personnelles
Sous Linux
```
sudo apt install git
```

Sous windows, voir l'adresse [https://git-scm.com/](https://git-scm.com/) et téléchargez l'installeur pour Windows. Pendant l'installation, vous serez invité à choisir le composant d'éditeur de texte que vous souhaitez utiliser avec Git. Pendant l'installation, vous aurez également la possibilité d'ajouter Git Bash à votre menu contextuel Windows. Cela vous permettra d'exécuter des commandes Git à partir de l'invite de commandes Git Bash. Installez également  [TorsoiseGIt](https://tortoisegit.org/) qui ajoutera les commandes git au menu contextuel.


## La base

Git utilise trois principaux niveaux pour gérer les modifications dans un projet :

* Répertoire de travail (Working Directory) : C'est l'espace où vous travaillez réellement sur les fichiers de votre projet. C'est la version actuelle des fichiers sur laquelle vous apportez des modifications.
* Dépôt local (.git) est le dépôt sur votre disque où Git stocke définitivement l'historique de toutes les modifications apportées au projet. Cela inclut tous les commits que vous avez créés. C'est l'endroit où son stocker vos modifications après un commit.
* Dépôt sur le serveur (Repository) : le dépôt Git distant (sur le serveur) est l'endroit où Git stocke définitivement l'historique de toutes les modifications apportées au projet.
* On parle également souvent d'Index ou de zone de préparation (Staging Area). L'index est un espace temporaire qui précède le commit. Avant de créer un commit, vous devez ajouter les modifications de votre répertoire de travail à l'index. Cela permet de sélectionner spécifiquement les modifications que vous souhaitez inclure dans le prochain commit.


Pour que vos modifications naviguent entre ces niveaux voici un exemple des 3 commandes de base.

**Commit** Après une modification, pour créer un commit avec un message de commit, vous pouvez utiliser la commande git commit. Par exemple :
```
git commit -m "Ajout d'une nouvelle fonctionnalité"
```
Cette commande enregistre les modifications actuelles dans le dépôt local/dans l'historique du dépôt avec le message "Ajout d'une nouvelle fonctionnalité".

**Push** Pour pousser (envoyer) vos commits vers un dépôt distant (serveur), utilisez la commande git push. Par exemple :
```
git push
```

**Pull** Pour récupérer les dernières modifications d'un dépôt distant (serveur) et fusionner ces modifications avec votre branche locale, utilisez la commande :
```
git pull
```


## Configurer nom, login et mot de passe

#### Nom et login (le minimum à faire)

Lancer les commande suivante (ou éditer le .gitconfig avec la commande `code ~/.gitconfig`)
```
git config --global user.name "Your Name"
git config --global user.email "your_email@example.com"
```

#### Mot de passe

Configurer le gestionnaire d'informations d'identification (credential helper) : Git propose plusieurs méthodes pour gérer les informations d'identification, comme stocker les informations d'identification en mémoire pendant un certain temps, utiliser un gestionnaire d'informations d'identification système, ou utiliser un gestionnaire d'informations d'identification tiers.

Par exemple, pour utiliser le gestionnaire d'informations d'identification Git intégré qui stocke les informations d'identification en mémoire pendant un certain temps, vous pouvez exécuter :
```
git config --global credential.helper cache
```


Si vous préférez utiliser un gestionnaire d'informations d'identification système comme le gestionnaire d'informations d'identification basé sur le trousseau d'informations d'identification GNOME Keyring sous Linux, vous pouvez exécuter :
 ```
 git config --global credential.helper gnome-keyring
 ```




## Premier pas avec la forge de l'Université
[https://forge.univ-lyon1.fr/](https://forge.univ-lyon1.fr/)

* Créez un projet sur la forge ou dupliquer un projet existant (fork)
* Ajoutez les membres de votre équipe de développement dans le menu `Manage/Members`
* Récupérer l'URL HTTPS git de votre projet dans la page d'accueil du projet, sous le titre après avec avoir basculé de SSH en HTTPS. L'adresse doit être de la forme `https://forge.univ-lyon1.fr/NUMERO_ETU/NOM_PROJET.git`
* Sur votre machine dans un terminal, placez vous dans un répertoire où vous travaillez : `cd LIFXYZ`
* Clonez le projet. Par exemple :

```
    git clone https://forge.univ-lyon1.fr/Alexandre.Meyer/L2_ConceptionDevApp.git
```

Tous les membres du projet doivent éxecuter cette commande de clônage sur leur propre ordinateur pour pouvoir travailler sur le projet (après qu'ils aient été ajoutés en tant que membres). Cette opération n'est à faire qu'une seule fois pour chaque étudiant, sauf si vous voulez plusieurs versions de travail du projet sur votre disque.


Une fois le répertoire de travail mis en place, vous pouvez effectuer des modifications (ajout de fichiers, suppression, modification etc.).

* Pour ajouter un fichier (fichier déjà ajouté localement) : `git add nom_fichier`
* Pour supprimer un fichier (fichier encore présent localement) : `git rm nom_fichier`
* Pour déplacer un fichier : `git mv nom_fichier_origine nom_fichier_cible`
* Pour valider les modifications : `git commit -a -m “un message pour expliquer les modifications`
* Pour envoyer les modifications sur le dépôt : `git push`
* Pour récupérer des modifications depuis le dépôt : `git pull`

La réconciliation des changements peut nécessiter une fusion que git tentera de faire automatiquement (remplacer pull par fetch pour fusionner manuellement ensuite via git merge ou git rebase). S'il y a des conflits, git laissera un marquage directement dans le fichier, contenant le code de la branche courante, et celui de la branche que vous voulez fusionner. Vous devrez alors corriger le problème manuellement et finir avec un commit.




## Les 12 commandements de Git

* **git clone URL** : récupère en local une copie du dépôt central (web) repéré par l'URL
```
    git clone https://forge.univ-lyon1.fr/Alexandre.Meyer/L2_ConceptionDevApp.git
```

* **git commit --m "message" FILES** : envoie les modifications des fichiers FILES du répertoire de travail vers le dépôt local. FILES peut contenir un répertoire comme "." pour commiter tous les fichiers de ce répertoire. Si FILES est vide, git ne prendra que l'index (par exemple les git add que vous avez fait avant).

```
    git commit –m "ajout de f affichage"  .
```

* **git push** : envoie les modifications du dépôt local vers le dépôt central (web)
* **git pull** : mise à jour du dépôt local et des fichiers locaux
* **git status \[path\]** : affiche l'état des fichiers du répertoire path, tous les fichiers si vide
* **git log** : donne tout l'historique des versions
* **git add \[FILES\]** : le fichier path est noté "à ajouter" lors du prochain commit, fonctionne récursivement.
* **git rm \[FILES\]** : le fichier est noté "à effacer" lors du prochain commit, fonctionne récursivement.
* **git mv src dest** : déplace un fichier/répertoire et note le renommage pour le prochain commit, conserve l\'historique
* **git checkout \[FILES\]** : remet FILES de la copie de travail dans l'état HEAD (la dernière version commit)!!! Perte des changements.
* **git reset HEAD\^** : annule le dernier commit et reviens à l'avant dernier.
* **git tag** : Liste toutes les versions qui ont un tag
* **git tag --a TAG --m "msg"** : ajoute un TAG à la version courante

```
    git tag -a v1.4 -m "my version 1.4"
    $ git tag
    v0.1
    v1.3
    v1.4
```

* **git commit** : après résolution de conflit, ATTENTION : le git commit n'a pas de FILES. Par exemple après un conflit sur toto.cpp. Sans -m git ouvre l'éditeur de texte avec un message pré-écrit qui indique que le conflit est résolu.

```
    $ git commit -m "resolution conflit"
    $ git push
```


## Des outils intéressants
- L'extension git dans Visual Code
- [Gittyup](https://murmele.github.io/Gittyup/), sympa, simple et permet de gérer les commandes git dans une application. Windows, Mac, Linux. Gratuit.
- [Gitkraken](https://www.gitkraken.com/) similaire à Gittyup. App gratuite pour les étudiants, mais payante sinon. Windows, Mac, Linux.
