//-------------------------------------------------------------------------------------------
//  University of Victoria Computer Science Department
//	FrameWork for OpenGL application under QT
//  Course title: Computer Graphics CSC305
//-------------------------------------------------------------------------------------------
//These two lines are header guiardians against multiple includes
#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QGLWidget>
#include <QProgressBar>
#include "foundation.h"
#include <QtGui>
#include <QtOpenGL>
#include <math.h>


//This is our OpenGL Component we built it on top of QGLWidget
class GLWidget : public QGLWidget
{
    Q_OBJECT

    struct Triangle {

        QVector3D a;
        QVector3D b;
        QVector3D c;
        QVector3D d;
        QVector3D e;
        double ambience[3];
        double diffuse[3];
        double specular[3];
        QVector3D normal;

        double shinyness;


        Triangle () {}

        Triangle(QVector3D tA,QVector3D tB,QVector3D tC, double amb[3], double diff[3], double spec[3], double shiny)
        {
            a = tA;
            b = tB;
            c = tC;

            shinyness = shiny;


            ambience[0] = amb[0];
            ambience[1] = amb[1];
            ambience[2] = amb[2];

            diffuse[0] = diff[0];
            diffuse[1] = diff[1];
            diffuse[2] = diff[2];

            specular[0] = spec[0];
            specular[1] = spec[1];
            specular[2] = spec[2];

            normal = QVector3D::crossProduct(c-a,b-a).normalized();



        }
    };

    struct LightBulb {

        QVector3D position;
        double radius;
        double intensity[3];


        LightBulb () {}

        LightBulb(QVector3D p, double r, double intens[3])
        {
            position = p;
            radius = r;
            intensity[0] = intens[0];
            intensity[1] = intens[1];
            intensity[2] = intens[2];

        }
    };

    struct Sphere {

        QVector3D position;
        double radius;

        double shinyness;
        double ambience[3];
        double diffuse[3];
        double specular[3];

        Sphere () {}

        Sphere (QVector3D p, double r, double a[3], double d[3], double s[3], double shiny)

        {
            position = p;
            radius = r;
            shinyness = shiny;

            ambience[0] = a[0];
            ambience[1] = a[1];
            ambience[2] = a[2];

            diffuse[0] = d[0];
            diffuse[1] = d[1];
            diffuse[2] = d[2];

            specular[0] = s[0];
            specular[1] = s[1];
            specular[2] = s[2];
        }

    };

public:
    //Constructor for GLWidget
    GLWidget(QWidget *parent = 0);

    //Destructor for GLWidget
    ~GLWidget();

    QVector<double> intersects(QVector3D ray, QVector3D rayOrigin, double closestPolygon);
    QVector<double> shadePolygons(QVector<double> polygoninfo, QVector3D rayOrigin, QVector3D ray);


    void openImage(QString fileBuf);
    void saveImage( QString fileBuf);
    void makeImage();
    void about();
    void help();
    QVector<double> rayTracer(QVector3D ray, QVector3D camera);
    QVector<double> rayTracer2(QVector3D ray, QVector3D camera);

protected:
    //Initialize the OpenGL Graphics Engine
    void initializeGL();

    //All our painting stuff are here
    void paintGL();

    //When user resizes main window, the scrollArea will be resized and it will call this function from
    //its attached GLWidget
    void resizeGL(int w, int h);

    //Handle mouse press event in scrollArea
    void mousePressEvent(QMouseEvent * );
    void mouseReleaseEvent(QMouseEvent * );
    //Handle Mouse Move Event
    void mouseMoveEvent(QMouseEvent * );
    void wheelEvent(QWheelEvent * );  // for zoom


private:
    void clear();
    int renderWidth, renderHeight;
    void displayImage();
    QProgressBar* pbar;
    void prepareImageDisplay(QImage* myimage); // converts from Qt to opengl format
    QImage glimage, qtimage;  // paintGL will display the gl formatted image
    // keep the qtimage around for saving (one is a copy of the other
    QVector<LightBulb> lightBulbs;
    QVector<Sphere> spheres;
    QVector<Triangle> triangles;
    double sceneAmbience;
};


#endif
