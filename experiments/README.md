# Experiments

Ce répertoire héberge les bacs à sable et prototypes qui ne font pas partie de
la base de production.

Règle simple :

- rien sous `experiments/` n'est une dépendance du build principal ;
- rien sous `experiments/` n'est requis pour `make test` ;
- si un module devient utile au produit, il est promu explicitement vers `src/`.
