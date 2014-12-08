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

        double ambience[3];
        double diffuse[3];
        double specular[3];

        Sphere () {}

        Sphere (QVector3D p, double r, double a[3], double d[3], double s[3])

        {
            position = p;
            radius = r;

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

    QVector<double> intersectionSpheres(QVector3D ray, QVector3D camera, double closestPolygon);


    void openImage(QString fileBuf);
    void saveImage( QString fileBuf);
    void makeImage();
    void about();
    void help();

    QVector<double> rayTracer(QVector3D ray, QVector3D camera);

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
    double sceneAmbience;
};


#endif
