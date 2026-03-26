# BobTricks — Contribution minimale

Ce dépôt n'est pas encore figé, mais il a déjà quelques invariants qu'il faut
protéger pour éviter de casser la base commune.

## Avant commit

Exécuter au minimum :

```sh
make test
make check_architecture
```

Pour un changement SDL / rendu / UI, exécuter aussi :

```sh
make run
```

Puis vérifier manuellement le comportement visible affecté.

## Ce qui ne doit pas être commité

- `build/`
- les binaires ou sorties locales produits depuis `analysis/`

Les outils sources d'analyse peuvent rester dans `analysis/*.cpp`, mais leurs
binaires doivent vivre sous `build/analysis/`.

## Règles d'architecture à respecter

- `src/core/` reste sans SDL, sans ImGui et sans code de rendu.
- `SimulationCore` reste propriétaire de l'état physique.
- `src/headless/` est un citoyen de premier rang, pas un mode de test annexe.
- `experiments/curves_lab/` reste un bac à sable tant qu'un module n'est pas promu dans
  `src/core/`.

## Stratégie de refactor

Préférer de petits commits cohérents :

1. refactor structurel sans changement de comportement ;
2. validation `make test` + `make check_architecture` ;
3. seulement ensuite changement fonctionnel.
