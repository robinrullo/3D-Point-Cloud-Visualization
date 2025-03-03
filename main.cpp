//3D Point Cloud Visualization
//Filename: main.cpp
//Release: 1.0 (version ECM)
//*********************************************************
//Librairie STL
#include <iostream>
#include <vector>
#include <utility>
#include <limits>
#include <math.h>
#include <fstream>
#include <string>
#include <algorithm>
#include <string.h>
#include <stdio.h>
#include <iomanip>
#include <time.h>
#include <stdlib.h>
//********************************************************
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/glut.h>
//********************************************************
using namespace std;

template<class T>
class point3D {
public:
    T x;
    T y;
    T z;

    point3D();

    point3D(T &a, T &b, T &c);
};


template<class T>
point3D<T>::point3D(): x(0),
                       y(0),
                       z(0) {
}

template<class T>
point3D<T>::point3D(T &a, T &b, T &c): x(a),
                                       y(b),
                                       z(c) {
}

//********************************************************
template<class T>
class point2D {
public:
    T x;
    T y;

    point2D();

    point2D(T &a, T &b);
};


template<class T>
point2D<T>::point2D(): x(0),
                       y(0) {
}

template<class T>
point2D<T>::point2D(T &a, T &b): x(a),
                                 y(b) {
}

point3D<double> pointTemp, barycentre;
vector<point3D<double> > nuage;
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

    //affichage du nuage de points
    double scale = 50;
    glEnable(GL_COLOR_MATERIAL);

    //3D point cloud
    glPointSize(1);
    glBegin(GL_POINTS);
    glColor3d(0, 0, 1);
    for (vector<point3D<double> >::iterator j = nuage.begin(); j != nuage.end(); ++j) {
        pointTemp = *j;
        glVertex3f((pointTemp.x - barycentre.x) / scale, (pointTemp.y - barycentre.y) / scale,
                   (pointTemp.z - barycentre.z) / scale);
    }
    glEnd();

    //centre
    glPointSize(5);
    glBegin(GL_POINTS);
    glColor3d(1, 0, 0);
    glVertex3f(0, 0, 0);
    glEnd();

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

    glBegin(GL_LINE_STRIP);

    // Dessus (face supérieure)
    glVertex3f((Xmin - barycentre.x) / scale, (Zmax - barycentre.z) / scale, (Ymin - barycentre.y) / scale);
    glVertex3f((Xmax - barycentre.x) / scale, (Zmax - barycentre.z) / scale, (Ymin - barycentre.y) / scale);
    glVertex3f((Xmax - barycentre.x) / scale, (Zmax - barycentre.z) / scale, (Ymax - barycentre.y) / scale);
    glVertex3f((Xmin - barycentre.x) / scale, (Zmax - barycentre.z) / scale, (Ymax - barycentre.y) / scale);
    glVertex3f((Xmin - barycentre.x) / scale, (Zmax - barycentre.z) / scale, (Ymin - barycentre.y) / scale);
    // Retour au premier point

    glEnd();

    glBegin(GL_LINE_STRIP);

    // Liaisons entre la base et le dessus
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


    glutSwapBuffers();
    //glFlush();// ATTENTION : si on flush au lieu de swapper tout l'ordi plante !
}

void Init() {
    glEnable(GL_DEPTH_TEST); // activation du test de Z-Buffering
    glutSetCursor(GLUT_CURSOR_NONE); // curseur invisible
}

void Reshape(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(perspective, (float) width / height, 0.001, 100000);
}

void Idle() {
    angle = angle + vitesse;
    if (angle > 360) angle = 0;
    glutPostRedisplay();
}

void Keyboard(unsigned char key, int x, int y) {
    //printf("%d\n", key);
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

int main(int argc, char **argv)
//int main(int argc, char argv[])
{
    //declare variables

    /* Fenétre GLUT*/
    glutInit(&argc, argv);

    point3D<double> pointTemp;

    int compteur = 0;
    string ligne;
    double a, b, c, d, e, f, g, h;
    nuage.reserve(400000);

    //*********************************

    //ifstream fichier("/Users/rrullo/CLionProjects/generation_enrichissement_environnement2d3d/Facade.xyz");
    ifstream fichier(
        "/Users/rrullo/CLionProjects/generation_enrichissement_environnement2d3d/st-helens.las.remaining.segmented.txt");
    if (!fichier) {
        cout << "fichier inexistant" << endl;
    } else {
        while (!fichier.eof()) {
            getline(fichier, ligne);
            sscanf(ligne.c_str(), "%lf %lf %lf %lf %lf %lf %lf %lf", &a, &b, &c, &d, &e, &f, &g, &h);
            //sscanf_s(ligne.c_str(), "%lf %lf %lf %lf %lf %lf %lf %lf", &a, &b, &c, &d, &e, &f,&g, &h);
            cout << "a=" << a << ", b=" << b << ", c=" << c << ", d=" << d << ", e=" << e << ", f=" << f << ", g=" << g
                    << ", h=" << h << endl;
            point3D<double> pointTemp;
            //b coordonnees en x
            //c coordonnees en y
            //d coordonnees en z
            pointTemp.x = b;
            pointTemp.y = c;
            pointTemp.z = d;


            // mise à jour de la bounding box
            if (compteur == 0) {
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

            // calcul du barycentre
            barycentre.x += pointTemp.x;
            barycentre.y += pointTemp.y;
            barycentre.z += pointTemp.z;

            nuage.push_back(pointTemp);


            ++compteur;
        }

        // calcul du barycentre
        barycentre.x /= compteur;
        barycentre.y /= compteur;
        barycentre.z /= compteur;
    }
    fichier.close();
    cout << "fichier xyz charge" << endl;
    cout << "nb de points dans le nuage : " << compteur << endl;


    //cout << "coordonnees du barycentre : (" << barycentre.x << "," << barycentre.y <<  "," << barycentre.z << ")" << endl;


    cout << "Xmin : " << Xmin << endl;
    cout << "Ymin : " << Ymin << endl;
    cout << "Zmin : " << Zmin << endl;
    cout << "Xmax : " << Xmax << endl;
    cout << "Ymax : " << Ymax << endl;
    cout << "Zmax : " << Zmax << endl;


    ::nuage;

    //-------------------------------------------------------------------------
    //Affichage SGL
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(40, 40);

    glutCreateWindow("3D Point Cloud Visualization (ECM)");
    //glutFullScreen();

    Init();
    glutDisplayFunc(Display);

    glutReshapeFunc(Reshape);
    glutIdleFunc(Idle);
    glutKeyboardFunc(Keyboard);

    //ShowInfos();
    glutMainLoop();
    //------------------------------------------------------------------------

    return 0;
}
