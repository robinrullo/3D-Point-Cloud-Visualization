# 🌍 3D Point Cloud Visualization

📌 **Description**  
Ce projet permet la visualisation interactive d'un nuage de points 3D à partir d'un fichier de points en ASCII. Il
utilise OpenGL et GLUT pour le rendu graphique et permet d'interagir avec la visualisation en temps réel.

---

## 🎯 Fonctionnalités

- 📡 Chargement d'un fichier ASCII contenant un nuage de points 3D avec couleurs.
- 🎨 Génération d'une triangulation en fonction de la hauteur et de la couleur des points.
- 🌀 Rotation et zoom sur la visualisation en temps réel.
- 🖼️ Sauvegarde d'images de la visualisation.
- 🔲 Affichage d'une boîte englobante (bounding box) du nuage de points.

---

## 📸 Aperçu

![Démonstration](result.gif)

---

## 🛠️ Installation et Exécution

### 🖥️ Prérequis

Avant de compiler et d'exécuter le projet, assurez-vous d'avoir installé les bibliothèques suivantes :

- **OpenGL**
- **GLUT** (`freeglut` recommandé)

Sous **Linux**, vous pouvez les installer avec :

```bash
sudo apt update
sudo apt install freeglut3-dev
```

Sous MacOS, utilisez Homebrew :

```bash
brew install freeglut
```

## 🎮 Contrôles

| Touche | Action                                       |
|:------:|----------------------------------------------|
|  `h`   | Affiche une carte de chaleur sur le maillage |
|  `x`   | Rotation autour de l’axe X                   |
|  `y`   | Rotation autour de l’axe Y                   |
|  `z`   | Rotation autour de l’axe Z                   |
|  `+`   | Augmenter la vitesse de rotation             | 
|  `-`   | Diminuer la vitesse de rotation              | 
|  `*`   | Inverser le sens de rotation                 |
|  `1`   | Augmenter la perspective                     |
|  `0`   | Diminuer la perspective                      |
|  `s`   | Sauvegarder une image de la scène            |
| `DEL`  | Réinitialiser les paramètres                 |
| `ESC`  | Quitter l’application                        |

## 📍 Chargement des Données

Le programme charge un fichier ASCII contenant les coordonnées (x, y, z), ainsi que les couleurs (r, g, b) de chaque
point. Ces données sont stockées dans une structure Point3D<double>.

## 🔺 Génération des Triangles

Les points sont organisés sous forme d’une grille permettant d’interpoler des hauteurs et des couleurs moyennes pour
générer des triangles formant une surface continue.
