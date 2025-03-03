//3D Point Cloud Visualization
//Filename: main.cpp
//Release: 1.0 (version ECM)
//*********************************************************
//Librairie STL
#include <iostream>
#include <vector>
#include <numeric>
#include <fstream>
#include <string>
#include <stdlib.h>
//********************************************************
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/glut.h>
//********************************************************
using namespace std;

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

//*********************************************
//fonctions d'affichage des données avec GLUT
//*********************************************
void Display();

void Init();

void Reshape(int width, int height);

void Idle();

void Keyboard(unsigned char, int, int);

void ShowInfos();

//vue de face
float angle = 92;
float vitesse = 0.0;
char axe = 'y';
int perspective = 120;
double scale = 200;


void drawTriangles(int nX) {
    int nY = round((Ymax - Ymin) / (Xmax - Xmin) * nX);

    float tileSizeX = (Xmax - Xmin) / nX;
    float tileSizeY = (Ymax - Ymin) / nY;


    std::vector<std::vector<float> > heights(nX + 1, std::vector<float>(nY + 1, Zmin));
    std::vector<std::vector<std::array<float, 3> > > colors(
        nX + 1, std::vector<std::array<float, 3> >(nY + 1, {0, 0, 0}));
    std::vector<std::vector<int> > count(nX + 1, std::vector<int>(nY + 1, 0));


    // Remplir les hauteurs à partir des points du nuage
    for (const auto &p: nuage) {
        const int i = (p.x - Xmin) / tileSizeX;
        const int j = (p.y - Ymin) / tileSizeY;
        if (i >= 0 && i <= nX && j >= 0 && j <= nY) {
            heights[i][j] += p.z;
            colors[i][j][0] += p.r / 255.0f;
            colors[i][j][1] += p.g / 255.0f;
            colors[i][j][2] += p.b / 255.0f;
            count[i][j]++;
        }
    }

    for (int i = 0; i <= nX; i++) {
        for (int j = 0; j <= nY; j++) {
            if (count[i][j] > 0) {
                heights[i][j] /= count[i][j] + 1; // TODO: pourquoi +1 ?
                colors[i][j][0] /= count[i][j];
                colors[i][j][1] /= count[i][j];
                colors[i][j][2] /= count[i][j];
            }
        }
    }

    // Dessiner la reconstruction en triangles
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

            // Normalisation des coordonnées
            float normX1 = (x1 - barycentre.x) / scale;
            float normX2 = (x2 - barycentre.x) / scale;
            float normY1 = (y1 - barycentre.y) / scale;
            float normY2 = (y2 - barycentre.y) / scale;
            float normZ1 = (z1 - barycentre.z) / scale;
            float normZ2 = (z2 - barycentre.z) / scale;
            float normZ3 = (z3 - barycentre.z) / scale;
            float normZ4 = (z4 - barycentre.z) / scale;

            // Récupérer la couleur de la case
            std::array<float, 3> color = colors[i][j];
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

void drawCloudPoints() {
    glEnable(GL_COLOR_MATERIAL);

    //3D point cloud
    glPointSize(1);
    glBegin(GL_POINTS);
    glColor3d(0, 0, 1);
    for (auto j = nuage.begin(); j != nuage.end(); ++j) {
        glVertex3f((j->x - barycentre.x) / scale, (j->z - barycentre.z) / scale,
                   (j->y - barycentre.y) / scale);
    }
    glEnd();
}

void drawBox() {
    //bounding box 3D
    glPointSize(2);
    glBegin(GL_LINE_STRIP);
    glColor3d(1, 1, 1);
    glVertex3f((Xmin - barycentre.x) / scale, (Zmin - barycentre.z) / scale, (Ymin - barycentre.y) / scale);
    glVertex3f((Xmax - barycentre.x) / scale, (Zmin - barycentre.z) / scale, (Ymin - barycentre.y) / scale);
    glVertex3f((Xmax - barycentre.x) / scale, (Zmin - barycentre.z) / scale, (Ymax - barycentre.y) / scale);
    glVertex3f((Xmin - barycentre.x) / scale, (Zmin - barycentre.z) / scale, (Ymax - barycentre.y) / scale);
    glVertex3f((Xmin - barycentre.x) / scale, (Zmin - barycentre.z) / scale, (Ymin - barycentre.y) / scale);
    // Retour au premier point
    glEnd();

    // Dessus (face supérieure)
    glBegin(GL_LINE_STRIP);

    glVertex3f((Xmin - barycentre.x) / scale, (Zmax - barycentre.z) / scale, (Ymin - barycentre.y) / scale);
    glVertex3f((Xmax - barycentre.x) / scale, (Zmax - barycentre.z) / scale, (Ymin - barycentre.y) / scale);
    glVertex3f((Xmax - barycentre.x) / scale, (Zmax - barycentre.z) / scale, (Ymax - barycentre.y) / scale);
    glVertex3f((Xmin - barycentre.x) / scale, (Zmax - barycentre.z) / scale, (Ymax - barycentre.y) / scale);
    glVertex3f((Xmin - barycentre.x) / scale, (Zmax - barycentre.z) / scale, (Ymin - barycentre.y) / scale);
    // Retour au premier point

    glEnd();

    // Liaisons entre la base et le dessus
    glBegin(GL_LINE_STRIP);
    glVertex3f((Xmin - barycentre.x) / scale, (Zmin - barycentre.z) / scale, (Ymin - barycentre.y) / scale);
    glVertex3f((Xmin - barycentre.x) / scale, (Zmax - barycentre.z) / scale, (Ymin - barycentre.y) / scale);
    glEnd();

    glBegin(GL_LINE_STRIP);
    glVertex3f((Xmax - barycentre.x) / scale, (Zmin - barycentre.z) / scale, (Ymin - barycentre.y) / scale);
    glVertex3f((Xmax - barycentre.x) / scale, (Zmax - barycentre.z) / scale, (Ymin - barycentre.y) / scale);
    glEnd();

    glBegin(GL_LINE_STRIP);
    glVertex3f((Xmax - barycentre.x) / scale, (Zmin - barycentre.z) / scale, (Ymax - barycentre.y) / scale);
    glVertex3f((Xmax - barycentre.x) / scale, (Zmax - barycentre.z) / scale, (Ymax - barycentre.y) / scale);
    glEnd();

    glBegin(GL_LINE_STRIP);
    glVertex3f((Xmin - barycentre.x) / scale, (Zmin - barycentre.z) / scale, (Ymax - barycentre.y) / scale);
    glVertex3f((Xmin - barycentre.x) / scale, (Zmax - barycentre.z) / scale, (Ymax - barycentre.y) / scale);
    glEnd();
}

void drawCenter() {
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

    switch (axe) {
        case 'x': glRotated(angle, 1, 0, 0);
            break;
        case 'y': glRotated(angle, 0, 1, 0);
            break;
        case 'z': glRotated(angle, 0, 0, 1);
            break;
    }


    drawCenter();
    //drawCloudPoints();
    drawBox();
    drawTriangles(40);
    glutSwapBuffers();
}


void Init(int *pargc, char **argv) {
    glutInit(pargc, argv);

    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(40, 40);

    glutCreateWindow("3D Point Cloud Visualization (ECM)");

    glEnable(GL_DEPTH_TEST); // activation du test de Z-Buffering
    glutSetCursor(GLUT_CURSOR_NONE); // curseur invisible
}

void Reshape(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(perspective, static_cast<float>(width) / height, 1, 10);
}

void Idle() {
    angle = angle + vitesse;
    if (angle > 360) angle = 0;
    glutPostRedisplay();
}

void Keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 27: // 'ESC'
            printf("\nFermeture en cours...\n");
            exit(0);
            break;

        case 120: // 'x'
            axe = 'x';
            ShowInfos();
            break;

        case 121: // 'y'
            axe = 'y';
            ShowInfos();
            break;

        case 122: // 'z'
            axe = 'z';
            ShowInfos();
            break;

        case 42: // '*'
            vitesse = -vitesse;
            ShowInfos();
            break;

        case 43: // '+'
            if (vitesse < 359.9) vitesse += 0.1;
            ShowInfos();
            break;

        case 45: // '-'
            if (vitesse > 0.1) vitesse -= 0.1;
            ShowInfos();
            break;

        case 48: // '0'
            perspective--;
            Reshape(640, 480);
            ShowInfos();
            break;

        case 49: // '1'
            perspective++;
            Reshape(640, 480);
            ShowInfos();
            break;

        case 127: // 'DEL'
            axe = 'y';
            vitesse = 0.1;
            perspective = 40; /*45*/
            Reshape(640, 480);
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
    printf("Vitesse de rotation : %.1f\n", vitesse);
    printf("Axe de rotation : %c\n", axe);
    printf("Perspective : %d\n", perspective);
}

//********************************************************

int main(int argc, char **argv) {

    int numberOfPointsLoaded = 0;
    string line;
    double a, b, c, d, e, f, g, h;
    nuage.reserve(400000);

    ifstream file(
        "/Users/rrullo/CLionProjects/generation_enrichissement_environnement2d3d/st-helens.las.segmented.subsampled.subsampled.txt");
    if (!file) {
        cout << "Fichier inexistant" << endl;
    } else {
        while (!file.eof()) {
            // Lecture d'une ligne du fichier XYZ (un point)
            getline(file, line);
            // File Header: X Y Z R G B Classification Point_Source_ID
            sscanf(line.c_str(), "%lf %lf %lf %lf %lf %lf %lf %lf", &a, &b, &c, &d, &e, &f, &g, &h);
            cout << "a=" << a << ", b=" << b << ", c=" << c << ", d=" << d << ", e=" << e << ", f=" << f << ", g=" << g
                    << ", h=" << h << endl;

            // Création du point
            Point3D<double> pointTemp;
            pointTemp.x = a;
            pointTemp.y = b;
            pointTemp.z = c;
            pointTemp.r = d;
            pointTemp.g = e;
            pointTemp.b = f;


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
    }
    file.close();
    cout << "Fichier xyz traite" << endl;
    cout << "Nuage de " << numberOfPointsLoaded << " point(s)" << endl;
    cout << "Xmin : " << Xmin << endl;
    cout << "Ymin : " << Ymin << endl;
    cout << "Zmin : " << Zmin << endl;
    cout << "Xmax : " << Xmax << endl;
    cout << "Ymax : " << Ymax << endl;
    cout << "Zmax : " << Zmax << endl;



    Init(&argc, argv);
    glutDisplayFunc(Display);

    glutReshapeFunc(Reshape);
    glutIdleFunc(Idle);
    glutKeyboardFunc(Keyboard);

    glutMainLoop();
    //------------------------------------------------------------------------

    return 0;
}
