# Présentation BobTricks

Diaporama interactif du projet, affiché directement dans le navigateur — aucune installation requise.

## Lancer la présentation

Ouvrir `presentation_shell.html` dans un navigateur moderne (Chrome, Firefox, Edge).

> Si les assets (images, vidéos) ne se chargent pas, servir le dossier localement :
> ```bash
> python3 -m http.server 8000
> # puis ouvrir http://localhost:8000/presentation_shell.html
> ```

## Navigation

| Action | Résultat |
|---|---|
| `→` / `Espace` | Slide suivant |
| `←` | Slide précédent |
| `N` | Afficher / masquer les notes orales |
| `F` ou `F11` | Plein écran |

## Structure

```
presentation_shell.html   — fichier unique à ouvrir
system/                   — styles et logique du shell
data/
  images/                 — captures et pictogrammes
  videos/                 — démonstrations intégrées
  diagrams/               — diagrammes SVG
  notes/                  — notes orales (notes.json / notes.js)
```

Les notes orales sont chargées depuis `data/notes/notes.js` et associées à chaque slide via l'attribut `data-notes`.
