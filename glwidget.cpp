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
    double specular[3] = {0.6,0.6,0.6};
    double intensity[3] = {0.6,0.6,0.6};

   // QVector3D circlepoint(5.0,5.0,5.0);

    sceneAmbience = 0.2;

    //BackWall

    triangles.append(Triangle(QVector3D(0,0,0),QVector3D(0,10,0),QVector3D(10,10,0),ambience,diffusal,specular,100));
    triangles.append(Triangle(QVector3D(0,0,0),QVector3D(10,10,0),QVector3D(10,0,0),ambience,diffusal,specular,100));

    //LeftWall

    triangles.append(Triangle(QVector3D(0,0,0),QVector3D(0,0,10),QVector3D(0,10,0),ambience,diffusal,specular,100));
    triangles.append(Triangle(QVector3D(0,0,10),QVector3D(0,10,10),QVector3D(0,10,0),ambience,diffusal,specular,100));

    //Floor

    triangles.append(Triangle(QVector3D(0,0,0),QVector3D(10,0,0),QVector3D(0,0,10),ambience,diffusal,specular,100));
    triangles.append(Triangle(QVector3D(0,0,10),QVector3D(10,0,0),QVector3D(10,0,10),ambience,diffusal,specular,100));



    spheres.append(Sphere(QVector3D(5.0,5.0,5.0),1.0,ambience,diffusal,specular,100));
    //spheres.append(Sphere(QVector3D(5.0,7.0,5.0),1.0,ambience,diffusal,spec));
    //Spheres.append()

    lightBulbs.append(LightBulb(QVector3D(4.0,2.0,7.0),0.25,intensity));


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

            double r = rayTraceResult[0]*255;
            double g = rayTraceResult[1]*255;
            double b = rayTraceResult[2]*255;
            myimage.setPixel(i,renderHeight-1-j,qRgb(r,g,b));
        }
    }

    //myimage.setPixel(i,	j,	qRgb(R,	G,	B));

    qtimage=myimage.copy(0, 0,  myimage.width(), myimage.height());

    prepareImageDisplay(&myimage);
}

//surfaceinfo:    - 0 / 1 / 2 / 3 where 0 = nothing 1 = sphere, 2 = light, 3 = area light 4 = triangle
//                - surfaceintersectionX
//                - surfaceintersectionY
//                - surfaceintersectionZ
//                - index

QVector<double> GLWidget::intersects(QVector3D ray, QVector3D rayOrigin, double closestPolygon)
{
    QVector<double> result(5);
    QVector3D a,b,c,d,e,pointOfIntersection,position,EO;

    double radius,cc,v,disc,distancetosurface;

    result[0] = 0;

    /// check against all spheres in scene
    for(int i=0;i<spheres.length();i++)//sphere index
    {
        radius = spheres[i].radius;
        position = spheres[i].position;

        EO = position-rayOrigin;

        cc = QVector3D::dotProduct(EO,EO);
        v = QVector3D::dotProduct(EO,ray);

        disc = radius*radius - (cc-v*v);

        if(disc > 0)
        {
            distancetosurface = v-sqrt(disc);
            if(distancetosurface < closestPolygon && distancetosurface > 0.01)
            {
                result[0] = 1;//for spheres
                result[4] = double(i);
                closestPolygon = distancetosurface;
                pointOfIntersection = rayOrigin + (distancetosurface)*ray;
            }
        }
    }
    ///check against all light sources

    for(int j=0;j<lightBulbs.length();j++)//lights index
    {
        radius = lightBulbs[j].radius;
        position = lightBulbs[j].position;

        EO = position-rayOrigin;

        cc = QVector3D::dotProduct(EO,EO);
        v = QVector3D::dotProduct(EO,ray);

        disc = radius*radius - (cc-v*v);

        if(disc>0)
        {
            distancetosurface = v-sqrt(disc);
            if(distancetosurface < closestPolygon)
            {
                result[0] = 2;//for lightBulbs
                result[4] = double(j);
                closestPolygon = distancetosurface;
                pointOfIntersection = rayOrigin + (distancetosurface)*ray;
            }
        }
    }

    for(int k=0;k<triangles.length();k++)
    {
        a = triangles[k].a;
        b = triangles[k].b;
        c = triangles[k].c;
        d = ray;
        e = rayOrigin;

        double aDeterminated = QMatrix4x4(a.x()-b.x(),a.x()-c.x(),d.x(),0,
                                    a.y()-b.y(),a.y()-c.y(),d.y(),0,
                                    a.z()-b.z(),a.z()-c.z(),d.z(),0,
                                              0,0,0,1).determinant();

        double tee = QMatrix4x4(a.x()-b.x(),a.x()-c.x(),a.x()-e.x(),0,
                          a.y()-b.y(),a.y()-c.y(),a.y()-e.y(),0,
                          a.z()-b.z(),a.z()-c.z(),a.z()-e.z(),0,
                                     0,0,0,1).determinant()/aDeterminated;

        if(tee < 0.01 || tee > closestPolygon)
        {
            continue;
        }


        double ess = QMatrix4x4(a.x()-b.x(),a.x()-e.x(),d.x(),0,
                          a.y()-b.y(),a.y()-e.y(),d.y(),0,
                          a.z()-b.z(),a.z()-e.z(),d.z(),0,
                                     0,0,0,1).determinant()/aDeterminated;



        if(ess < 0 || ess > 1)
            continue;

        double beta = QMatrix4x4(a.x()-e.x(),a.x()-c.x(),d.x(),0,
                           a.y()-e.y(),a.y()-c.y(),d.y(),0,
                           a.z()-e.z(),a.z()-c.z(),d.z(),0,
                                     0,0,0,1).determinant()/aDeterminated;


        if(beta < 0 || beta > 1 - ess)
            continue;

        result[0] = 4;//for triangle
        result[4] = double(k);
        closestPolygon = tee;
        pointOfIntersection = rayOrigin + (tee)*ray;

    }


    result[1] = pointOfIntersection.x();
    result[2] = pointOfIntersection.y();
    result[3] = pointOfIntersection.z();

    return result;
}

QVector<double> GLWidget::shadePolygons(QVector<double> polygoninfo, QVector3D rayOrigin, QVector3D ray)
{
    QVector<double> result(3),checkerino;
    QVector3D surfaceNormal,lightVector,cameraVector,h;
    double ambience[3],diffuse[3],specular[3];
    double shadeR,shadeG,shadeB,lightVectorDistance,lightMagnitude,l,specularReflection;

    QVector3D pointOfIntersection(polygoninfo[1],polygoninfo[2],polygoninfo[3]);

    if(polygoninfo[0] == 1)
    {
        ambience[0] = spheres[polygoninfo[4]].ambience[0];
        ambience[1] = spheres[polygoninfo[4]].ambience[1];
        ambience[2] = spheres[polygoninfo[4]].ambience[2];

        diffuse[0] = spheres[polygoninfo[4]].diffuse[0];
        diffuse[1] = spheres[polygoninfo[4]].diffuse[1];
        diffuse[2] = spheres[polygoninfo[4]].diffuse[2];

        specular[0] = spheres[polygoninfo[4]].specular[0];
        specular[1] = spheres[polygoninfo[4]].specular[1];
        specular[2] = spheres[polygoninfo[4]].specular[2];

        specularReflection = spheres[polygoninfo[4]].shinyness;

        surfaceNormal = (pointOfIntersection - spheres[polygoninfo[4]].position).normalized();
        //qDebug() << "SPHERE";

    }

    else{//is 4, a triangle

        ambience[0] = triangles[polygoninfo[4]].ambience[0];
        ambience[1] = triangles[polygoninfo[4]].ambience[1];
        ambience[2] = triangles[polygoninfo[4]].ambience[2];

        diffuse[0] = triangles[polygoninfo[4]].diffuse[0];
        diffuse[1] = triangles[polygoninfo[4]].diffuse[1];
        diffuse[2] = triangles[polygoninfo[4]].diffuse[2];

        specular[0] = triangles[polygoninfo[4]].specular[0];
        specular[1] = triangles[polygoninfo[4]].specular[1];
        specular[2] = triangles[polygoninfo[4]].specular[2];

        specularReflection = triangles[polygoninfo[4]].shinyness;

        surfaceNormal = triangles[polygoninfo[4]].normal;
    }

    //Ambience

    shadeR = ambience[0]*sceneAmbience;
    shadeG = ambience[1]*sceneAmbience;
    shadeB = ambience[2]*sceneAmbience;

    for(int i=0;i<lightBulbs.size();i++)
    {
        lightVector = (lightBulbs[i].position - pointOfIntersection).normalized();
        lightVectorDistance = (lightBulbs[i].position - pointOfIntersection).length();

        //Shaderino

        checkerino = intersects(lightVector, pointOfIntersection, (lightVector-lightBulbs[i].position).length());

        if(checkerino[0] == 1 || checkerino[0] == 4)
        {
            qDebug() << checkerino[0];
            continue;
        }


        //Lambertian

        lightMagnitude = QVector3D::dotProduct(surfaceNormal,lightVector);
        l = max(0.0,(lightMagnitude));

        shadeR += diffuse[0]*lightBulbs[i].intensity[0]*l;
        shadeG += diffuse[1]*lightBulbs[i].intensity[1]*l;
        shadeB += diffuse[2]*lightBulbs[i].intensity[2]*l;

        //Blinn-Phong

        cameraVector = (rayOrigin-pointOfIntersection).normalized();
        h = (cameraVector + lightVector).normalized();
        double specularmagnitude = QVector3D::dotProduct(surfaceNormal,h);
        double s = pow(max(0.0,specularmagnitude),specularReflection);

        shadeR += specular[0]*lightBulbs[i].intensity[0]*s;
        shadeG += specular[1]*lightBulbs[i].intensity[1]*s;
        shadeB += specular[2]*lightBulbs[i].intensity[2]*s;
    }
    result[0] = shadeR;
    result[1] = shadeG;
    result[2] = shadeB;

    return result;
}

QVector<double> GLWidget::rayTracer(QVector3D ray, QVector3D camera)
{
    QVector<double> result(3),intersectionResult,polygoninfo,voxel;
    double closestPolygon = pow(10,10);
    polygoninfo = intersects(ray,camera,closestPolygon);

    if(polygoninfo[0] == 1 || polygoninfo[0] == 4)
    {
        voxel = shadePolygons(polygoninfo,camera,ray);
        result[0] = voxel[0];
        result[1] = voxel[1];
        result[2] = voxel[2];
//        qDebug() << result[0];
//        qDebug() << result[1];
//        qDebug() << result[2];
    }

    else if(polygoninfo[0] == 2)
    {
        result[0] = lightBulbs[polygoninfo[4]].intensity[0];
        result[1] = lightBulbs[polygoninfo[4]].intensity[1];
        result[2] = lightBulbs[polygoninfo[4]].intensity[2];
    }
    else{
        result[0] = 0.0;
        result[1] = 0.0;
        result[2] = 0.0;
    }
    return result;
}

QVector<double> GLWidget::rayTracer2(QVector3D ray, QVector3D camera)
{
    QVector<double> result(5),intersectionResult;
    double closestPolygon = pow(10,10);
    QVector3D EO;

    double rr,cc,v,disc;

    intersectionResult = intersects(ray,camera,closestPolygon);
    if(intersectionResult[0] = 1)
    {
        result[1] = intersectionResult[1];
        result[2] = intersectionResult[2];
        result[3] = intersectionResult[3];
        closestPolygon = intersectionResult[4];

    }
    return result;
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

