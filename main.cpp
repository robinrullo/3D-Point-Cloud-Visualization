//3D Point Cloud Visualization
//Filename: main.cpp
//Release: 1.0 (version ECM)
//*********************************************************
//Librairie STL
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <cstdlib>
//********************************************************
#include <GL/glut.h>
#ifndef STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#endif
//********************************************************
using namespace std;

string PROJECT_PATH = "/Users/rrullo/CLionProjects/generation_enrichissement_environnement2d3d/";

template<typename T>
struct Point3D {
    T x, y, z;
    T r, g, b;

    explicit Point3D(
        const double x = 0,
        const double y = 0,
        const double z = 0,
        const double r = 0,
        const double g = 0,
        const double b = 1
    )
        : x(x), y(y), z(z), r(r), g(g), b(b) {
    }
};

Point3D<double> pointTemp, barycentre;
vector<Point3D<double> > nuage;
double Xmin, Ymin, Zmin, Xmax, Ymax, Zmax;
bool heatmap_mode = true;

//*********************************************
//fonctions d'affichage des données avec GLUT
//*********************************************
void Display();

void Init();

void Reshape(int width, int height);

void Idle();

void Keyboard(unsigned char, int, int);

void ShowInfos();

void SaveFrameBufferAsImage();

// VUE
int WINDOW_WIDTH = 640;
int WINDOW_HEIGHT = 400;
float ANGLE = 92;
float SPEED = .2;
char AXIS = 'y';
int PERSPECTIVE = 60;
double SCALE = 300;
int NUMBER_OF_TRIANGLES = 60;

int nX = 0, nY = 0;
float tileSizeX = 0, tileSizeY = 0;
vector<vector<float> > heights;
vector<vector<array<float, 3> > > colors;
vector<vector<array<float, 3> > > heatmapColors;

/**
 * Calcul de la hauteur moyenne et de la couleur moyenne des triangles
 */
void ComputeTriangles() {
    double xLength = Xmax - Xmin;
    double yLength = Ymax - Ymin;

    double minLength = min(xLength, yLength);
    double tileSize = minLength / NUMBER_OF_TRIANGLES;

    nX = ceil(xLength / tileSize);
    nY = ceil(yLength / tileSize);

    tileSizeX = (Xmax - Xmin) / nX;
    tileSizeY = tileSizeX;

    heights.resize(nX + 1, vector<float>(nY + 1, 0));
    colors.resize(nX + 1, vector<array<float, 3> >(nY + 1, {0, 0, 0}));
    heatmapColors.resize(nX + 1, vector<array<float, 3> >(nY + 1, {0, 0, 0}));

    vector numberOfTriangles(nX + 1, vector<int>(nY + 1, 0));

    for (int i = 0; i <= nX; i++) {
        for (int j = 0; j <= nY; j++) {
            float xMinCell = Xmin + i * tileSizeX;
            float xMaxCell = xMinCell + tileSizeX;
            float yMinCell = Ymin + j * tileSizeY;
            float yMaxCell = yMinCell + tileSizeY;

            for (const auto &p: nuage) {
                if (p.x >= xMinCell && p.x < xMaxCell && p.y >= yMinCell && p.y < yMaxCell) {
                    heights[i][j] += p.z;
                    colors[i][j][0] += p.r / 255.0f;
                    colors[i][j][1] += p.g / 255.0f;
                    colors[i][j][2] += p.b / 255.0f;
                    numberOfTriangles[i][j]++;
                }
            }
        }
    }

    for (int i = 0; i <= nX; i++) {
        for (int j = 0; j <= nY; j++) {
            if (numberOfTriangles[i][j] > 0) {
                heights[i][j] /= numberOfTriangles[i][j];
                colors[i][j][0] /= numberOfTriangles[i][j];
                colors[i][j][1] /= numberOfTriangles[i][j];
                colors[i][j][2] /= numberOfTriangles[i][j];

                float norm = (heights[i][j] - Zmin) / (Zmax - Zmin);

                if (norm < 0.5) {
                    // Interpolation du vert au bleu
                    heatmapColors[i][j][0] = 0; // R reste à 0
                    heatmapColors[i][j][1] = 2 * (0.5 - norm); // G diminue
                    heatmapColors[i][j][2] = 2 * norm; // B augmente
                } else {
                    // Interpolation du bleu au rouge
                    heatmapColors[i][j][0] = 2 * (norm - 0.5); // R augmente
                    heatmapColors[i][j][1] = 0; // G reste à 0
                    heatmapColors[i][j][2] = 2 * (1 - norm); // B diminue
                }
            }
        }
    }
}

/**
 * Dessin des triangles
 */
void DrawTriangles() {
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < nX; i++) {
        for (int j = 0; j < nY; j++) {
            // Coins des cellules
            float x1 = Xmin + i * tileSizeX;
            float x2 = Xmin + (i + 1) * tileSizeX;
            float y1 = Ymin + j * tileSizeY;
            float y2 = Ymin + (j + 1) * tileSizeY;

            // Hauteurs correspondantes
            float z1 = heights[i][j];
            float z2 = heights[i + 1][j];
            float z3 = heights[i][j + 1];
            float z4 = heights[i + 1][j + 1];

            if (z1 == 0 || z2 == 0 || z3 == 0 || z4 == 0) {
                continue;
            }

            // Normalisation des coordonnées
            float normX1 = (x1 - barycentre.x) / SCALE;
            float normX2 = (x2 - barycentre.x) / SCALE;
            float normY1 = (y1 - barycentre.y) / SCALE;
            float normY2 = (y2 - barycentre.y) / SCALE;
            float normZ1 = (z1 - barycentre.z) / SCALE;
            float normZ2 = (z2 - barycentre.z) / SCALE;
            float normZ3 = (z3 - barycentre.z) / SCALE;
            float normZ4 = (z4 - barycentre.z) / SCALE;


            // Récupérer la couleur de la case
            std::array<float, 3> color = colors[i][j];
            if (heatmap_mode) {
                color = heatmapColors[i][j];
            }
            glColor3f(color[0], color[1], color[2]);

            // Triangulation : 1er triangle
            glVertex3f(normX1, normZ1, normY1);
            glVertex3f(normX2, normZ2, normY1);
            glVertex3f(normX1, normZ3, normY2);

            // Triangulation : 2ème triangle
            glVertex3f(normX2, normZ2, normY1);
            glVertex3f(normX2, normZ4, normY2);
            glVertex3f(normX1, normZ3, normY2);
        }
    }
    glEnd();
}

void DrawPointCloud() {
    glEnable(GL_COLOR_MATERIAL);

    //3D point cloud
    glPointSize(1);
    glBegin(GL_POINTS);
    glColor3d(0, 0, 1);
    for (auto j = nuage.begin(); j != nuage.end(); ++j) {
        glVertex3f((j->x - barycentre.x) / SCALE, (j->z - barycentre.z) / SCALE,
                   (j->y - barycentre.y) / SCALE);
    }
    glEnd();
}

void DrawBoundingBox() {
    //bounding box 3D
    glPointSize(2);
    glBegin(GL_LINE_STRIP);
    glColor3d(1, 1, 1);
    glVertex3f((Xmin - barycentre.x) / SCALE, (Zmin - barycentre.z) / SCALE, (Ymin - barycentre.y) / SCALE);
    glVertex3f((Xmax - barycentre.x) / SCALE, (Zmin - barycentre.z) / SCALE, (Ymin - barycentre.y) / SCALE);
    glVertex3f((Xmax - barycentre.x) / SCALE, (Zmin - barycentre.z) / SCALE, (Ymax - barycentre.y) / SCALE);
    glVertex3f((Xmin - barycentre.x) / SCALE, (Zmin - barycentre.z) / SCALE, (Ymax - barycentre.y) / SCALE);
    glVertex3f((Xmin - barycentre.x) / SCALE, (Zmin - barycentre.z) / SCALE, (Ymin - barycentre.y) / SCALE);
    // Retour au premier point
    glEnd();

    // Dessus (face supérieure)
    glBegin(GL_LINE_STRIP);

    glVertex3f((Xmin - barycentre.x) / SCALE, (Zmax - barycentre.z) / SCALE, (Ymin - barycentre.y) / SCALE);
    glVertex3f((Xmax - barycentre.x) / SCALE, (Zmax - barycentre.z) / SCALE, (Ymin - barycentre.y) / SCALE);
    glVertex3f((Xmax - barycentre.x) / SCALE, (Zmax - barycentre.z) / SCALE, (Ymax - barycentre.y) / SCALE);
    glVertex3f((Xmin - barycentre.x) / SCALE, (Zmax - barycentre.z) / SCALE, (Ymax - barycentre.y) / SCALE);
    glVertex3f((Xmin - barycentre.x) / SCALE, (Zmax - barycentre.z) / SCALE, (Ymin - barycentre.y) / SCALE);
    // Retour au premier point

    glEnd();

    // Liaisons entre la base et le dessus
    glBegin(GL_LINE_STRIP);
    glVertex3f((Xmin - barycentre.x) / SCALE, (Zmin - barycentre.z) / SCALE, (Ymin - barycentre.y) / SCALE);
    glVertex3f((Xmin - barycentre.x) / SCALE, (Zmax - barycentre.z) / SCALE, (Ymin - barycentre.y) / SCALE);
    glEnd();

    glBegin(GL_LINE_STRIP);
    glVertex3f((Xmax - barycentre.x) / SCALE, (Zmin - barycentre.z) / SCALE, (Ymin - barycentre.y) / SCALE);
    glVertex3f((Xmax - barycentre.x) / SCALE, (Zmax - barycentre.z) / SCALE, (Ymin - barycentre.y) / SCALE);
    glEnd();

    glBegin(GL_LINE_STRIP);
    glVertex3f((Xmax - barycentre.x) / SCALE, (Zmin - barycentre.z) / SCALE, (Ymax - barycentre.y) / SCALE);
    glVertex3f((Xmax - barycentre.x) / SCALE, (Zmax - barycentre.z) / SCALE, (Ymax - barycentre.y) / SCALE);
    glEnd();

    glBegin(GL_LINE_STRIP);
    glVertex3f((Xmin - barycentre.x) / SCALE, (Zmin - barycentre.z) / SCALE, (Ymax - barycentre.y) / SCALE);
    glVertex3f((Xmin - barycentre.x) / SCALE, (Zmax - barycentre.z) / SCALE, (Ymax - barycentre.y) / SCALE);
    glEnd();
}

void DrawCenterPoint() {
    glPointSize(5);
    glBegin(GL_POINTS);
    glColor3d(1, 0, 0);
    glVertex3f(0, 0, 0);
    glEnd();
}


void Display() {
    glClearColor(0, 0, 0, 0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(4, 3, 3, 0, 0, 0, 0, 1, 0);

    switch (AXIS) {
        case 'x': glRotated(ANGLE, 1, 0, 0);
            break;
        case 'y': glRotated(ANGLE, 0, 1, 0);
            break;
        case 'z': glRotated(ANGLE, 0, 0, 1);
            break;
        default: break;
    }


    DrawCenterPoint();
    //DrawPointCloud();
    DrawBoundingBox();
    DrawTriangles();

    glutSwapBuffers();
}


void Init(int *pargc, char **argv) {
    glutInit(pargc, argv);

    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(40, 40);

    glutCreateWindow("3D Point Cloud Visualization (ECM)");

    glEnable(GL_DEPTH_TEST); // activation du test de Z-Buffering
    glutSetCursor(GLUT_CURSOR_NONE); // curseur invisible
}

void Reshape(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(PERSPECTIVE, static_cast<float>(width) / height, 1, 10);
}

void Idle() {
    ANGLE = ANGLE + SPEED;
    if (ANGLE > 360) ANGLE = 0;
    glutPostRedisplay();
}

void Keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 27: // 'ESC'
            printf("\nFermeture en cours...\n");
            exit(0);

        case 120: // 'x'
            AXIS = 'x';
            ShowInfos();
            break;

        case 121: // 'y'
            AXIS = 'y';
            ShowInfos();
            break;

        case 122: // 'z'
            AXIS = 'z';
            ShowInfos();
            break;

        case 42: // '*'
            SPEED = -SPEED;
            ShowInfos();
            break;

        case 43: // '+'
            if (SPEED < 359.9) SPEED += .1;
            ShowInfos();
            break;

        case 45: // '-'
            if (SPEED > 0.1) SPEED -= .1;
            ShowInfos();
            break;

        case 48: // '0'
            PERSPECTIVE--;
            Reshape(WINDOW_WIDTH, WINDOW_HEIGHT);
            ShowInfos();
            break;

        case 49: // '1'
            PERSPECTIVE++;
            Reshape(WINDOW_WIDTH, WINDOW_HEIGHT);
            ShowInfos();
            break;

        case 104: // 'h'
            heatmap_mode = !heatmap_mode;
            glutPostRedisplay();
            ShowInfos();
            break;

        case 115: // Sauvegarde de l'image
            SaveFrameBufferAsImage();
            break;


        case 127: // 'DEL'
            AXIS = 'y';
            SPEED = .2;
            PERSPECTIVE = 60;
            Reshape(WINDOW_WIDTH, WINDOW_HEIGHT);
            ShowInfos();
            break;
        default: break;
    }
}

void ShowInfos() {
    system("clear");
    printf("+ augmente la vitesse\n");
    printf("- diminue la vitesse\n");
    printf("1 augmente la perspective\n");
    printf("0 diminue la perspective\n");
    printf("* inverse le sens de rotation\n");
    printf("x, y et z pour changer l'axe de rotation\n");
    printf("DEL pour restaurer les parametres par defaut\n");
    printf("ESC pour quitter\n\n");
    printf("Vitesse de rotation : %.1f\n", SPEED);
    printf("Axe de rotation : %c\n", AXIS);
    printf("Perspective : %d\n", PERSPECTIVE);
}

int imageCounter = 1; // Compteur d'images
void SaveFrameBufferAsImage() {
    const int width = WINDOW_WIDTH; // Largeur de la fenêtre
    const int height = WINDOW_HEIGHT; // Hauteur de la fenêtre
    std::vector<unsigned char> pixels(width * height * 3);

    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    // OpenGL stocke l'image à l'envers, il faut inverser les lignes
    std::vector<unsigned char> flippedPixels(width * height * 3);
    for (int i = 0; i < height; i++) {
        memcpy(&flippedPixels[i * width * 3], &pixels[(height - i - 1) * width * 3], width * 3);
    }

    // Générer le nom du fichier
    const std::string filename = PROJECT_PATH + std::to_string(imageCounter++) + ".png";

    // Sauvegarder l'image
    stbi_write_png(filename.c_str(), width, height, 3, flippedPixels.data(), width * 3);

    std::cout << "Image enregistree : " << filename << std::endl;
}

//********************************************************

void LoadPointCloud(const string &filename) {
    int numberOfPointsLoaded = 0;
    string line;
    double x, y, z, red, green, blue, classification, pointSourceId;

    ifstream file(filename);
    if (!file) {
        cout << "Erreur: Le fichier n'existe pas" << endl;
        exit(1);
    }

    const int linesCount = std::count(std::istreambuf_iterator<char>(file),
                                      std::istreambuf_iterator<char>(), '\n');
    nuage.reserve(linesCount + 1);

    file.seekg(0);

    while (!file.eof()) {
        // Lecture d'une ligne du fichier ASCII Cloud File
        getline(file, line);
        sscanf(line.c_str(), "%lf %lf %lf %lf %lf %lf %lf %lf", &x, &y, &z, &red, &green, &blue, &classification,
               &pointSourceId);

        // Création du point
        Point3D<double> pointTemp(x, y, z, red, green, blue);

        // Mise à jour de la bounding box
        if (numberOfPointsLoaded == 0) {
            Xmin = Xmax = pointTemp.x;
            Ymin = Ymax = pointTemp.y;
            Zmin = Zmax = pointTemp.z;
        } else {
            if (pointTemp.x < Xmin) Xmin = pointTemp.x;
            if (pointTemp.y < Ymin) Ymin = pointTemp.y;
            if (pointTemp.z < Zmin) Zmin = pointTemp.z;
            if (pointTemp.x > Xmax) Xmax = pointTemp.x;
            if (pointTemp.y > Ymax) Ymax = pointTemp.y;
            if (pointTemp.z > Zmax) Zmax = pointTemp.z;
        }

        // Calcul du barycentre
        barycentre.x += pointTemp.x;
        barycentre.y += pointTemp.y;
        barycentre.z += pointTemp.z;

        // Ajout du point au nuage
        nuage.push_back(pointTemp);


        // Incrémentation du nombre de points chargés
        ++numberOfPointsLoaded;
    }

    // Calcul du barycentre
    barycentre.x /= numberOfPointsLoaded;
    barycentre.y /= numberOfPointsLoaded;
    barycentre.z /= numberOfPointsLoaded;
    file.close();


    cout << "Fichier charge: " << filename << endl;
    cout << "Nombre de points : " << numberOfPointsLoaded << endl;
    cout << "Xmin : " << Xmin << endl;
    cout << "Xmax : " << Xmax << endl;
    cout << "Ymin : " << Ymin << endl;
    cout << "Ymax : " << Ymax << endl;
    cout << "Zmin : " << Zmin << endl;
    cout << "Zmax : " << Zmax << endl;
}

int main(int argc, char **argv) {
    LoadPointCloud(
        PROJECT_PATH + "st-helens.las.segmented.subsampled.xyz");
    ComputeTriangles();

    Init(&argc, argv);

    glutDisplayFunc(Display);

    glutReshapeFunc(Reshape);
    glutIdleFunc(Idle);
    glutKeyboardFunc(Keyboard);

    glutMainLoop();
    //------------------------------------------------------------------------

    return 0;
}
