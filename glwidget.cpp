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



    Spheres.append(Sphere(QVector3D(5.0,5.0,5.0),2.0,ambience,diffusal,spec));
    //Spheres.append()

    LightBulbs.append(LightBulb(QVector3D(2.0,2.0,2.0),0.5));


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
            double c = rayTraceResult[0]*255;
            myimage.setPixel(i,renderHeight-1-j,qRgb(c,c,c));
        }
    }


    //myimage.setPixel(i,	j,	qRgb(R,	G,	B));

    qtimage=myimage.copy(0, 0,  myimage.width(), myimage.height());

    prepareImageDisplay(&myimage);
}

QVector<double> GLWidget::rayTracer(QVector3D ray, QVector3D camera)
{
    QVector<double> result;
    QVector3D EO;
    double rr,cc,v,disc;

    result = QVector<double>();

    QVector3D circlepoint(5.0,5.0,5.0);//temp
    double circleradius = 1; //temp

    rr = 1;
    EO = circlepoint-camera;
    cc = QVector3D::dotProduct(EO,EO);
    v = QVector3D::dotProduct(EO,ray);

    disc = rr - (cc-v*v);

    if(disc<=0)//ray =! intersect sphere
    {
        result.append(0.0);
        return result;
    }

    else//intersected
    {
        result.append(1.0);
        return result;
    }

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

