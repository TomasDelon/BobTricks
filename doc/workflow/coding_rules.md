
# Le code de la route de la conception de projet

**Votre note va dépendre de ces points.**

## Outils

* Editeur de **code** propre (évitez les éditeurs de texte comme Emacs, Vi, Scite, etc.) qui doit comporter :
  * Indentation et code couleur
  * Compilation dans l'éditeur avec lien direct sur les erreurs
  * Auto-complétion lors de la frappe
  * Refactoring évolués
  * Les plus : recherche de définition de variable, etc.
* Makefile au minimum, ou un projet CMake
  * Il est important de savoir compiler dans un terminal mais en pratique on compile avec un prpjet (Make)
* Utiliser un débugeur (par ex. gdb), un profiler (par ex. gprof ou valgrind)
* Gestion du code sous Git
* Code documenté sous Doxygen
* Un projet comprend un README.txt expliquant comment l'utiliser et décrivant ce qu'il fait


## Règles de conception

* Un programme est constitué de modules (c-à-d .h/.cpp)
* Un module est habituellement constitué d'une seule classe : par exemple `class Image` donne le module `Image.h/.cpp`
* Pour chaque classe, définissez un ensemble de fonctions membres travaillant sur celle-ci. Ces fonctions membres auront un nom explicite et standardisée : ex. fait de un ou plusieurs mots indiquant ce que fait la fonction. Par exemple :

```
void effacer(const Pixel & couleur);
void sauver(const string & filename) const;
```

* Ecrivez des accesseurs **get** SEULEMENT quand cela est nécessaire. Il ne devraient y avoir que très peu de mutateurs **set**  et ils font des vérifications importantes. Par exemple :

```
void setPix(const unsigned int x, const unsigned int y, const Pixel & couleur);
// modifie la couleur du pixel de coordonnées (x,y)
// si x ou y ne sont pas valides, la procédure s'arrête (par exemple avec un assert)
```

* **A part les fonctions du module, aucune autre partie du code n'accède directement aux champs de la classe**. Par exemple le module `TraitementImage` n'accède pas directement à la donnée membre `dimx` pour éviter qu'il ne le change. Changer une dimension demande de rédéfinir le tableau de pixel, c'est un traitement qui doit passer par une fonction `void resize(int new_dx, int new_dy)`.


## Les pointeurs

Vous pouvez quasiment vous passer des pointeurs si vous utilisez des `std::vector`` pour stocker vos éléments. Moins de pointeurs = moins de bug ! Notamment attention à ce bug :
```
class A
{
pubic:
   A() : pi(new int[10]) { for(int i=0;i<10;i++) pi[i]=i; }
   ~A()  { delete[] pi; }
private:
   int* pi;
};


int truc()
{ 
   A a;// a.pi pointe sur un tableau
   A aa;   // aa.pi pointe sur un autre tableau
   aa = a; // aa.pi pointe maintenant sur le tableau a.pi : car le "=" recopie a dans aa. 
   // Le 1er tableau de aa n'est plus accessible, ce qui est déjà une grosse erreur mais ne fait pas planter.
}  // Le destructeur de a détruit son tableau a.pi. 
   // Le destructeur de aa détruit également le même tableau car aa.pi et a.pi sont les mêmes 


int machin()
{
  A a;
  std::vector<A> tabA;
  tabA.push_back(a);
}  // Même erreur que précédemment
```

Pour résoudre ce problème il faut soit écrire un `operator=` et un constructeur par copie (1ere solution) ou ne pas utiliser de pointeur
(2e solution).


Solution 1 :

```
class A
{
pubic:
   A() : pi(new int[10]) { for(int i=0;i<10;i++) pi[i]=i; }
   A(const A& a) : pi(new int[10]) { for(int i=0;i<10;i++) pi[i] = a.pi[i]; }
   const A& operator=(const A& a)  { for(int i=0;i<10;i++) pi[i] = a.pi[i]; } // recopie du tableau, pas du pointeur
   ~A()  { delete[] pi; }
private:
   int* pi;
};
```

Solution 2 (vraiment plus courte) :

```
class A
{
pubic:
   A() { for(int i=0;i<10;i++)  tabi.push_back(i);  } 
   ~A()  {}
private:
   std::vector<int> tabi;       // le tableau vector de la STL comporte l'operator= et le constructeur par copie
};
```


## Règles de programmation

* Faites des vérifications avec **assert**
* Passage de paramètres irréprochable : les paramètres sont quasiment toujours passés par référence
  * si en mode donnée => `const Type &` ou `Type`
  * si en mode donnée-résultat => `Type &`

```
void Image::fonctionInOut(Pixel &); // donnée-résultat (IN-OUT)
void Image::fonctionIn(const Pixel &);  // donnée (IN)
```

* Mettez des **const** partout où vous pouvez
* Définissez et respectez une norme sur les noms de variable et donnez du sens au nom de vos variables
* Initialisation de toutes les variables



## Gestion de la mémoire

* Allocation dans  le tas différent de la pile (relire le cours de LIFAPSD)
* Pour toute allocation mémoire dans le tas, il faut **absolument une désallocation** à un moment
* **Avec valgrind tout doit être propre!**



## Resources autres

* Thierry Excoffier propose également [quelques règles de bonnes pratiques de programmation](http://perso.univ-lyon1.fr/thierry.excoffier/COURS/COURS/TRANS_COMP_IMAGE/prog.html)  
