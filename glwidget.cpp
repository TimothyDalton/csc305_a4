//-------------------------------------------------------------------------------------------
//   Painting with Flowsnakes
// fsnake program modified to use open gl vertex arrays  Brian Wyvill October 2012
// added save/restore and postscript driver November 2012
// fixed memory management November 2012 delete works properly now.
// added progress bar to show memory usage.
//-------------------------------------------------------------------------------------------

#include "glwidget.h"


GLWidget::GLWidget(QWidget *parent)
    : QGLWidget(parent)
{
    double ambience[3] = {0.6,0.6,0.6};
    double diffusal[3] = {0.6,0.6,0.6};
    double spec[3] = {0.6,0.6,0.6};
    double intensity[3] = {0.6,0.6,0.6};

   // QVector3D circlepoint(5.0,5.0,5.0);

    sceneAmbience = 0.2;

    spheres.append(Sphere(QVector3D(5.0,5.0,5.0),1.0,ambience,diffusal,spec));
    //spheres.append(Sphere(QVector3D(5.0,7.0,5.0),1.0,ambience,diffusal,spec));
    //Spheres.append()

    lightBulbs.append(LightBulb(QVector3D(4.0,2.0,7.0),0.5,intensity));


}

GLWidget::~GLWidget()
{    

}

void GLWidget::clear()
{
     updateGL();
}

void GLWidget::initializeGL()
{
    //Background color will be white
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glShadeModel( GL_FLAT );
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glPointSize(5);
}

void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    displayImage();
}

/* 2D */
void GLWidget::resizeGL( int w, int h )
{
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho(0.0,GLdouble(w),0,GLdouble(h),-10.0,10.0);
    glFlush();
    glMatrixMode(GL_MODELVIEW);
    glViewport( 0, 0, (GLint)w, (GLint)h );
    cerr << "gl new size "<< w SEP h NL;
    renderWidth = w;
    renderHeight = h;
}

// no mouse events in this demo
void GLWidget::mousePressEvent( QMouseEvent * )
{
}

void GLWidget::mouseReleaseEvent( QMouseEvent *)
{
}

void GLWidget::mouseMoveEvent ( QMouseEvent * )
{
}

// wheel event
void GLWidget::wheelEvent(QWheelEvent *)
{
}

void GLWidget::openImage(QString fileBuf)
{     
    QImage myImage;
    myImage.load(fileBuf);
    prepareImageDisplay(&myImage);
}

void GLWidget::prepareImageDisplay(QImage* myimage)
{   
    glimage = QGLWidget::convertToGLFormat( *myimage );  // flipped 32bit RGBA stored as mi
    updateGL();    
}

void GLWidget::displayImage()
{
    if (glimage.width()==0) {
        cerr << "Null Image\n";
        return;
    } else {
        glRasterPos2i(0,0);
        glDrawPixels(glimage.width(), glimage.height(), GL_RGBA, GL_UNSIGNED_BYTE, glimage.bits());
        glFlush();
    }
}

void GLWidget::saveImage( QString fileBuf )
{
    // there is no conversion  back toQImage
    // hence the need to keep qtimage as well as glimage
    qtimage.save ( fileBuf );   // note it is the qtimage in the right format for saving
}

void GLWidget::makeImage( )
{   
    QImage myimage(renderWidth, renderHeight, QImage::Format_RGB32);
    qDebug() << renderWidth;
    qDebug() << renderHeight;
    double widthratio = 10.0;
    double heightratio = renderHeight / (renderWidth/widthratio);

    QVector3D camera(widthratio/2, heightratio/2, 20);

    qDebug() << camera;

    QVector3D pixelposition;
    QVector3D ray;
    QVector<double> rayTraceResult;

    for(int i=0;i<renderWidth;i++)
    {
        for(int j=0;j<renderHeight;j++)
        {
            pixelposition = QVector3D(double(i)*widthratio/renderWidth,double(j)*heightratio/renderHeight,10.0);
            ray = (pixelposition-camera).normalized();
            //qDebug() << "ray: " << ray;
            if(i == renderWidth/2 && j == renderHeight/2)
                qDebug() << "ray: " << ray;

            rayTraceResult = rayTracer(ray,camera);

            double r = rayTraceResult[1]*255;
            double g = rayTraceResult[2]*255;
            double b = rayTraceResult[3]*255;
            myimage.setPixel(i,renderHeight-1-j,qRgb(r,g,b));
        }
    }

    //myimage.setPixel(i,	j,	qRgb(R,	G,	B));

    qtimage=myimage.copy(0, 0,  myimage.width(), myimage.height());

    prepareImageDisplay(&myimage);
}

QVector<double> GLWidget::intersectionSpheres(QVector3D ray, QVector3D camera, double closestPolygon)
{
    QVector<double> result;
    QVector3D pointOfIntersection,sphereNormal,lightVector,lightPosition,spherePosition,EO,cameraVector,h;
    result = QVector<double>(5);

    double r,cc,v,disc,d, shadeR,shadeG,shadeB;


    result[0] = 0;

    for(int i=0;i<spheres.length();i++)//sphere index
    {
        r = spheres[i].radius;
        spherePosition = spheres[i].position;



        EO = spherePosition-camera;

        cc = QVector3D::dotProduct(EO,EO);
        v = QVector3D::dotProduct(EO,ray);

        disc = r*r - (cc-v*v);

        if(disc>0)
        {
            d = sqrt(disc);
            if(v-d<closestPolygon)
            {
                closestPolygon = v-d;
                result[0] = 1;
                pointOfIntersection = camera + (v-d)*ray;


                //Ambience

                shadeR = spheres[i].ambience[0]*sceneAmbience;
                shadeG = spheres[i].ambience[1]*sceneAmbience;
                shadeB = spheres[i].ambience[2]*sceneAmbience;

                for(int j=0;j<lightBulbs.size();j++)
                {

                    lightPosition = lightBulbs[j].position;

                    //Lambertian

                    sphereNormal = (pointOfIntersection - spherePosition).normalized();
                    lightVector = (lightPosition - pointOfIntersection).normalized();
                    double lightmagnitude = QVector3D::dotProduct(sphereNormal,lightVector);
                    double l = max(0.0,(lightmagnitude));

                    shadeR += spheres[i].diffuse[0]*lightBulbs[j].intensity[0]*l;
                    shadeG += spheres[i].diffuse[1]*lightBulbs[j].intensity[1]*l;
                    shadeB += spheres[i].diffuse[2]*lightBulbs[j].intensity[2]*l;

                    //Blinn-Phong

                    cameraVector = (camera-pointOfIntersection).normalized();
                    h = (cameraVector + lightVector).normalized();
                    double specularmagnitude = QVector3D::dotProduct(sphereNormal,h);
                    double s = pow(max(0.0,specularmagnitude),100);

                    shadeR += spheres[i].specular[0]*lightBulbs[j].intensity[0]*s;
                    shadeG += spheres[i].specular[1]*lightBulbs[j].intensity[1]*s;
                    shadeB += spheres[i].specular[2]*lightBulbs[j].intensity[2]*s;




                }
                result[1] = shadeR;
                result[2] = shadeG;
                result[3] = shadeB;
            }
        }

    }

    result[4] = closestPolygon;
    return result;

}

QVector<double> GLWidget::rayTracer(QVector3D ray, QVector3D camera)
{
    QVector<double> result(5),intersectionResult;
    double closestPolygon = pow(10,10);
    QVector3D EO;

    double rr,cc,v,disc;

    intersectionResult = intersectionSpheres(ray,camera,closestPolygon);
    if(intersectionResult[0] = 1)
    {
        result[1] = intersectionResult[1];
        result[2] = intersectionResult[2];
        result[3] = intersectionResult[3];
        closestPolygon = intersectionResult[4];

    }

    return result;


//    if(disc<=0)//ray =! intersect sphere
//    {
//        result.append(0.0);
//        return result;
//    }

//    else//intersected
//    {
//        result.append(1.0);
//        return result;
//    }

}

void GLWidget::about()
{
    QString vnum;
    QString mess, notes;
    QString title="Images in Qt and Opengl ";

    vnum.setNum ( MYVERSION );
    mess="Qt OpenGl image demo Release Version: ";
    mess = mess+vnum;
    notes = "\n\n News: Every QImage is now on stack, there is no way for memory leak. -- Lucky";
    mess = mess+notes;
    QMessageBox::information( this, title, mess, QMessageBox::Ok );
}

void GLWidget::help()
{
    QString vnum;
    QString mess, notes;
    QString title="qtglimages";

    vnum.setNum ( MYVERSION);
    mess="Simple Image Handling in Qt/Opengl by Brian Wyvill Release Version: ";
    mess = mess+vnum;
    notes = "\n\n Save and Load images for display.  Also Make an image such as output from a ray tracer. ";
    mess = mess+notes;
    QMessageBox::information( this, title, mess, QMessageBox::Ok );
}

