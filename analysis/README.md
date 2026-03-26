# Analysis Tools

Ce dossier contient des outils ponctuels de diagnostic headless.

Règles :

- les sources `analysis/*.cpp` peuvent être versionnées ;
- les binaires ne vivent pas ici, mais sous `build/analysis/` ;
- compilation via `make analysis/<nom>`.

Exemple :

```sh
make analysis/swing_foot_height
./build/analysis/swing_foot_height
```
