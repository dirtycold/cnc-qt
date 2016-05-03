/****************************************************************************
 * Main developer, C# developing:                                           *
 * Copyright (C) 2014-2016 by Sergey Zheigurov                              *
 * Russia, Novy Urengoy                                                     *
 * zheigurov@gmail.com                                                      *
 *                                                                          *
 * C# to Qt portation, Linux developing                                     *
 * Copyright (C) 2015-2016 by Eduard Kalinowski                             *
 * Germany, Lower Saxony, Hanover                                           *
 * eduard_kalinowski@yahoo.de                                               *
 *                                                                          *
 * C# project CNC-controller-for-mk1                                        *
 * https://github.com/selenur/CNC-controller-for-mk1                        *
 *                                                                          *
 * The Qt project                                                           *
 * https://github.com/eduard-x/cnc-qt                                       *
 *                                                                          *
 * CNC-Qt is free software; may be distributed and/or modified under the    *
 * terms of the GNU General Public License version 3 as published by the    *
 * Free Software Foundation and appearing in the file LICENSE_GPLv3         *
 * included in the packaging of this file.                                  *
 *                                                                          *
 * This program is distributed in the hope that it will be useful,          *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 * GNU General Public License for more details.                             *
 *                                                                          *
 * You should have received a copy of the GNU Lesser General Public         *
 * License along with CNC-Qt. If not, see  http://www.gnu.org/licenses      *
 ****************************************************************************/

#include "includes/version.h"

#include "includes/GLWidget.h"

#include <QDebug>

// #include <QtOpenGL/QtOpenGL>

#include <QtOpenGL/QGLWidget>
#include <QtOpenGL/QGLShaderProgram>

// for GLES2 are QGLFunctions to implement

#include <deque>
#include <utility>
#include "includes/vec.h"

#include "includes/Settings.h"
#include "includes/MainWindow.h"
#include "includes/mk1Controller.h"


#include <math.h>

/**
 * @brief constructor
 *
 */
GLWidget::GLWidget(QWidget *p)
    : QGLWidget(p)
{
    if (p == NULL) {
        return;
    }

    coordArray.clear();
    colorArray.clear();

    parent = (MainWindow*)p;

    cnc = parent->cnc;

    initStaticElements();

    initializeGL();

    fps = 0;

    QTimer* fpsTimer = new QTimer();
    QObject::connect(fpsTimer, SIGNAL(timeout()), this, SLOT(showFPS()));
    fpsTimer->start(1000);

    //     glTimer = new QTimer();
    //     QObject::connect(glTimer, SIGNAL(timeout()), this, SLOT(processing()));
    //     glTimer->start( 20 );
    //

    QGLFormat::OpenGLVersionFlags f = QGLFormat::openGLVersionFlags();
    qDebug() << "GL enabled" << QGLFormat::hasOpenGL() << "flags:" << f;
    QString glStr = QString().sprintf("%s %s %s", (char*)glGetString(GL_VENDOR), (char*)glGetString(GL_RENDERER), (char*)glGetString(GL_VERSION));

    qDebug() << glStr;
    initPreviewSettings();

    paintGL();
}

/**
 * @brief destructor
 *
 */
GLWidget::~GLWidget()
{
}

/**
 * @brief
 *
 */
void GLWidget::initStaticElements()
{
    xAxis = {
        { 0.0, 0.0, 0.0 },
        { 10.0, 0.0, 0.0 },
        { 10.0, 0.0, 0.0 },
        { 9.0, 1.0, 0.0 },
        { 10.0, 0.0, 0.0 },
        { 9.0, -1.0, 0.0 }
    };

    yAxis = {
        { 0.0, 0.0, 0.0 },
        { 0.0, 10.0, 0.0 },
        { 0.0, 10.0, 0.0 },
        { 1.0, 9.0, 0.0 },
        { 0.0, 10.0, 0.0 },
        { -1.0, 9.0, 0.0 }
    };

    zAxis = {
        { 0.0, 0.0, 0.0 },
        { 0.0, 0.0, 10.0 },
        { 0.0, 0.0, 10.0 },
        { 1.0, 1.0, 9.0 },
        { 0.0, 0.0, 10.0 },
        { -1.0, -1.0, 9.0 }
    };

    instrumentArray = {
        { 0.0, 0.0, 0.0 },
        { 0.0, 0.0, 4.0 },
        { -1.0, -1.0, 2.0 },
        { -1.0, 1.0, 2.0 },
        { 1.0, -1.0, 2.0 },
        { 1.0, 1.0, 2.0 },
        { 1.0, 1.0, 2.0 },
        { -1.0, 1.0, 2.0 },
        { 1.0, -1.0, 2.0 },
        { -1.0, -1.0, 2.0 },
        { -1.0, -1.0, 2.0 },
        { 0.0, 0.0, 0.0 },
        { 1.0, 1.0, 2.0 },
        { 0.0, 0.0, 0.0 },
        { 1.0, -1.0, 2.0 },
        { 0.0, 0.0, 0.0 },
        { -1.0, 1.0, 2.0 },
        { 0.0, 0.0, 0.0 }
    };

    footArray = { // GL_LINE_LOOP array
        { 0.0, 0.0, 0.0 }, // 0
        { 0.0, 22.0, 0.0 }, // 1
        { 0.0, 22.0, 29.0 }, // 2
        { 0.0, 12.0, 29.0 }, // 3
        { 0.0, 0.0, 12.0 }, // 4
        { 0.0, 0.0, 0.0 }, // 5
        { 3.6, 0.0, 0.0 }, // 6
        { 3.6, 22.0, 0.0 }, // 7
        { 3.6, 22.0, 29.0 }, // 8
        { 3.6, 12.0, 29.0 }, // 9
        { 3.6, 0.0, 12.0 }, // 10
        { 3.6, 0.0, 0.0 }, // 11
        { 0.0, 0.0, 0.0 }, // 12
        { 0.0, 0.0, 12.0 }, // 13
        { 3.6, 0.0, 12.0 }, // 14
        { 3.6, 12.0, 29.0 }, // 15
        { 0.0, 12.0, 29.0 }, // 16
        { 0.0, 22.0, 29.0 }, // 17
        { 3.6, 22.0, 29.0 }, // 18
        { 3.6, 22.0, 0.0 }, // 19
        { 0.0, 22.0, 0.0 }, // 20
        { 0.0, 0.0, 0.0 } // 21
    };

    traverseArray = { // width of traverse is 64 cm
        { 0.0, 0.0, 0.0 },
        { 64.0, 0.0, 0.0 },
        { 0.0, 0.0, 0.0 },
        { 0.0, 1.2, 0.0 },
        { 0.0, 0.0, 0.0 },
        { 0.0, 0.0, 12.0 },
        { 0.0, 1.2, 0.0 },
        { 64.0, 1.2, 0.0 },
        { 0.0, 1.2, 0.0 },
        { 0.0, 1.2, 12.0 },
        { 0.0, 0.0, 12.0 },
        { 0.0, 1.2, 12.0 },
        { 0.0, 0.0, 12.0 },
        { 64.0, 0.0, 12.0 },
        { 0.0, 1.2, 12.0 },
        { 64.0, 1.2, 12.0 },
        { 64.0, 0.0, 0.0 },
        { 64.0, 1.2, 0.0 },
        { 64.0, 0.0, 12.0 },
        { 64.0, 1.2, 12.0 },
        { 64.0, 0.0, 0.0 },
        { 64.0, 0.0, 12.0 },
        { 64.0, 1.2, 0.0 },
        { 64.0, 1.2, 12.0 }
    };
}

/**
 * @brief after reload of surface matrix
 *
 */
void GLWidget::surfaceReloaded()
{
    for (int i = 0; i < coordArray.count(); i++) {
        if (parent->deltaFeed) {
            pointGL p;
            float pointX = parent->gCodeList.at(i).X;
            float pointY = parent->gCodeList.at(i).Y;
            float pointZ = parent->gCodeList.at(i).Z;
            pointZ += parent->getDeltaZ(pointX, pointY);

            p = (pointGL) {
                pointX, pointY, pointZ
            };

            coordArray[i] = p;
        }
    }
}

/**
 * @brief send current FPS number to the main widget
 *
 */
void GLWidget::showFPS()
{
    emit fpsChanged(fps);

    fps = 0;
}

/**
 * @brief processing of fps
 *
 */
void GLWidget::processing()
{
    paintGL();
    fps++;
}

/**
 * @brief main object data is reloaded, update the visualisation
 *
 */
void GLWidget::matrixReloaded()
{
    int workNum = 0;

    workNum = parent->gCodeList.count();

    coordArray.clear();
    colorArray.clear();

    if (workNum > 1) {
        int currWorkPoint = 0;

        foreach (GCodeData vv, parent->gCodeList) {
            colorGL cl;

            if (vv.movingCode != RAPID_LINE_CODE) {
                cl = Settings::colorSettings[COLOR_WORK];
            } else {
                cl = Settings::colorSettings[COLOR_RAPID];
            }

            pointGL p;
            // coordinates of next point
            float pointX = vv.X;
            float pointY = vv.Y;
            float pointZ = vv.Z;

            // moving in G-code
            if (parent->Correction) {
                // proportions
                pointX *= parent->koeffSizeX;
                pointY *= parent->koeffSizeY;

                // offset
                pointX += parent->deltaX;
                pointY += parent->deltaY;
                pointZ += parent->deltaZ;

                // to use the scanned surface, z correcture
                if (parent->deltaFeed) {
                    pointZ += parent->getDeltaZ(pointX, pointY);
                }
            }

            p = (pointGL) {
                pointX, pointY, pointZ
            };

            coordArray << p;
            colorArray << cl;

            currWorkPoint++;
        }
    }

    initPreviewSettings();

    initializeGL();
}

/**
 * @brief send new data about angles to main widget for displaying on buttons
 *
 */
void GLWidget::initPreviewSettings()
{
    emit rotationChanged();
    //     emit yRotationChanged(parent->PosAngleY);
    //     emit zRotationChanged(parent->PosAngleZ);

    updateGL();
}

/**
 * @brief init of 3d viewing
 *
 */
void GLWidget::initializeGL()//Init3D()//*OK*
{
    makeCurrent();

    // OpenGLES2
    initializeGLFunctions();

    //     glClearColor(qglColor(Qt::black));

#if 1 // USE_GLES2 == true
    //     glScalef( 1, 1, -1 ); // negative z is top
    glEnable(GL_DEPTH_TEST);
     
    Projection.perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
   
    View.lookAt(QVector3D(4, 3, 3), // Camera is at (4,3,3), in Worl$
                QVector3D(0, 0, 0), // and looks at the origin
                QVector3D(0, 1, 0) // Head is up (set to 0,-1,0 to $
               );

    Model.scale(1.0f);

    MVP  = Projection * View * Model; // Remember, matrix multiplication is the other way arou$

#else
    // activate projection matrix
    glMatrixMode(GL_PROJECTION);

    // clening
    glLoadIdentity();
    glScalef( 1, 1, -1 ); // negative z is top
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_DEPTH_TEST);
#endif
}

/**
 * @brief redraw the scene
 *
 */
void GLWidget::paintGL()
{
    Draw();
    fps++;
}

/**
 * @brief resize the scene
 *
 */
void GLWidget::resizeGL(int w, int h)
{
#if 0
      // Calculate aspect ratio
    qreal aspect = qreal(w) / qreal(h ? h : 1);

    // Set near plane to 3.0, far plane to 7.0, field of view 45 degrees
    const qreal zNear = 3.0, zFar = 7.0, fov = 45.0;

    // Reset projection
    Projection.setToIdentity();

    // Set perspective projection
    Projection.perspective(fov, aspect, zNear, zFar);
#else
    int left = 0, top = 0;
    int width = w, height = h;
    float scale = ((float)w / (float)h);

    if (w > h * 4 / 3) {// wenn screen width over 4 / 3
        width = h * 4 * scale / 3;
        height = h * scale;
        left = (w - width) / (2 * scale);
    } else { // wenn screen width under 4 / 3
        height = w * 3 / (4 * scale);
        top = (h - height) / (2 * scale);
    }

    glViewport(left, top, width, height); // set the current size of view
#endif
}


void GLWidget::mousePressEvent(QMouseEvent *event)
{
    lastPos = event->pos();
}

/**
 * @brief
 *
 */
void GLWidget::wheelEvent(QWheelEvent *e)
{
    e->delta() > 0 ? parent->PosZoom++ : parent->PosZoom--;

    e->setAccepted(true);
    updateGL();
}

/**
 * @brief
 *
 */
void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();

    if (event->buttons() & Qt::LeftButton) {
        setXCoord(parent->PosX += dx);
        setYCoord(parent->PosY -= dy);
    } else if (event->buttons() & Qt::RightButton) {
        switch (parent->fixedAxes) {
            case FixY: {
                setXRotation(parent->PosAngleX + dy);
                setZRotation(parent->PosAngleZ + dx);
                break;
            }

            case FixX: {
                setYRotation(parent->PosAngleY + dx);
                setZRotation(parent->PosAngleZ + dy);
                break;
            }

            case FixZ: {
                setXRotation(parent->PosAngleX + dy);
                setYRotation(parent->PosAngleY + dx);
                break;
            }
        }
    }

    lastPos = event->pos();

    event->setAccepted(true);
    updateGL();
}


#define GLSCALE 2000.0

/**
 * @brief
 *
 */
void GLWidget::Draw() // drawing, main function
{
    //
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clean color buffer and depth buff
    glClearColor(Settings::colorSettings[COLOR_BACKGROUND].r, Settings::colorSettings[COLOR_BACKGROUND].g, Settings::colorSettings[COLOR_BACKGROUND].b, 1.0);//()0.45f, 0.45f, 0.45f, 1);                  // set gray color of background

    glLoadIdentity();                                   // clean

    glPushMatrix();                                     // push current matrix

    if (windowState() != Qt::WindowMinimized) {
        int w = width();
        int h = height();

        float n = 1;

        if (h < w) {
            n = (w / h);
        }

        /// move eyes point for better view of object
        glTranslated(parent->PosX / GLSCALE, parent->PosY / GLSCALE, parent->PosZ / GLSCALE);


        float scaleX = parent->PosZoom / (GLSCALE * n);
        float scaleY = parent->PosZoom / GLSCALE;
        float scaleZ = parent->PosZoom / GLSCALE;

        glScalef(scaleX, scaleY, scaleZ);
    }

    /// угловое вращение
    glRotatef(parent->PosAngleX, 1, 0, 0);
    glRotatef(parent->PosAngleY, 0, 1, 0);
    glRotatef(parent->PosAngleZ, 0, 0, 1);


    glEnable(GL_LINE_SMOOTH);

    // axes
    if (parent->ShowAxes) {
        drawAxes();
    }

    // coordinate grid
    if (parent->ShowGrid) {
        drawGrid();
    }

    //  the scanned surface
    if (parent->ShowSurface) {
        drawSurface();
    }

    // drawTool();

    // draw the tool
    if (parent->ShowInstrument) {
        drawInstrument();
    }

    drawWorkField();

    // draw the border
    if (parent->ShowGrate) {
        drawGrate();
    }

    // end of drawing
    glDisable(GL_LINE_SMOOTH);

    //
    glPopMatrix();
    //
    glFlush();
}

/**
 * @brief
 *
 */
void GLWidget::drawAxes()
{
    glLineWidth(2);

    glDisable(GL_DEPTH_TEST); // because of text rendering

    glEnable(GL_VERTEX_ARRAY);

    glDisable(GL_NORMAL_ARRAY);

    // x
    glColor3f(Settings::colorSettings[COLOR_X].r, Settings::colorSettings[COLOR_X].g, Settings::colorSettings[COLOR_X].b);
    glVertexPointer(3, GL_FLOAT, 0, &xAxis[0]);
    glDrawArrays(GL_LINES, 0, xAxis.count()); // draw array of lines
    renderText(12.0, 0.0, 0.0, QString("X")); //coordinates of text

    // y
    glColor3f(Settings::colorSettings[COLOR_Y].r, Settings::colorSettings[COLOR_Y].g, Settings::colorSettings[COLOR_Y].b);
    glVertexPointer(3, GL_FLOAT, 0, &yAxis[0]);
    glDrawArrays(GL_LINES, 0, yAxis.count());
    renderText(0.0, 12.0, 0.0, QString("Y")); //coordinates of text

    // z
    glColor3f(Settings::colorSettings[COLOR_Z].r, Settings::colorSettings[COLOR_Z].g, Settings::colorSettings[COLOR_Z].b);
    glVertexPointer(3, GL_FLOAT, 0, &zAxis[0]);
    glDrawArrays(GL_LINES, 0, zAxis.count());
    renderText(0.0, 0.0, 12.0, QString("Z")); //coordinates of text

    glDisable(GL_VERTEX_ARRAY);

    glEnable(GL_NORMAL_ARRAY);

    glEnable(GL_DEPTH_TEST);
}

/**
 * @brief
 *
 */
void GLWidget::drawWorkField()
{
    if (coordArray.count() < 2) {
        return;
    }

    glPushMatrix();

    glEnable(GL_VERTEX_ARRAY);
    glEnable(GL_COLOR_ARRAY);
    glDisable(GL_NORMAL_ARRAY);
    glDisable(GL_TEXTURE_COORD_ARRAY);

    glLineWidth(0.3f);

    // the object
    glVertexPointer(3, GL_FLOAT, 0, &coordArray[0]);
    glColorPointer(4, GL_FLOAT, 0, &colorArray[0]);
    glDrawArrays(GL_LINE_STRIP, 0, coordArray.count());
    //

    // select with 3.0 the current cut of object
    switch (parent->getStatus()) {
        case Task::Waiting: {
            int numSelectStart = Task::lineCodeStart;

            if (numSelectStart < 0) {
                break;
            }

            int numSelectStop = Task::lineCodeEnd;
            glLineWidth(3.0f);
            glVertexPointer(3, GL_FLOAT, 0, &coordArray[numSelectStart]);
            glColorPointer(4, GL_FLOAT, 0, &colorArray[numSelectStart]);
            glDrawArrays(GL_LINE_STRIP, 0, numSelectStop - numSelectStart - 1);
            break;
        }

        case Task::Stop:  {
            if (Task::lineCodeStart < 0) {
                break;
            }

            glLineWidth(3.0f);
            glVertexPointer(3, GL_FLOAT, 0, &coordArray[Task::lineCodeStart]);
            glColorPointer(4, GL_FLOAT, 0, &colorArray[Task::lineCodeStart]);
            glDrawArrays(GL_LINE_STRIP, 0, 2);
            break;
        }

        case Task::Paused: {
            int numSelect = cnc->numberCompleatedInstructions() - 1;

            if (numSelect < 0 ) {
                break;
            }

            glLineWidth(3.0f);
            glVertexPointer(3, GL_FLOAT, 0, &coordArray[numSelect]);
            glColorPointer(4, GL_FLOAT, 0, &colorArray[numSelect]);
            glDrawArrays(GL_LINE_STRIP, 0, 2);

            break;
        }

        case Task::Working: {
            int numSelect = cnc->numberCompleatedInstructions() - 1;

            if (numSelect >= 0 && numSelect < coordArray.count()) {
                glLineWidth(3.0f);
                glVertexPointer(3, GL_FLOAT, 0, &coordArray[numSelect]);
                glColorPointer(4, GL_FLOAT, 0, &colorArray[numSelect + 1]);
                glDrawArrays(GL_LINE_STRIP, 0, 2);
            }

            break;
        }
    }

    glDisable(GL_VERTEX_ARRAY);
    glDisable(GL_COLOR_ARRAY);
    glEnable(GL_NORMAL_ARRAY);
    glEnable(GL_TEXTURE_COORD_ARRAY);

    glPopMatrix();
}

/**
 * @brief
 *
 */
void GLWidget::drawGrid()
{
    if (parent->ShowLines) {
        glLineWidth(0.1f);
        glColor3f(Settings::colorSettings[COLOR_GRID].r, Settings::colorSettings[COLOR_GRID].g, Settings::colorSettings[COLOR_GRID].b);

        glBegin(GL_LINES);

        for (int gX = parent->GridXstart; gX < parent->GridXend + 1; gX += parent->GrigStep) {
            glVertex3f(gX, parent->GridYstart, 0);
            glVertex3f(gX, parent->GridYend, 0);
        }

        for (int gY = parent->GridYstart; gY < parent->GridYend + 1; gY += parent->GrigStep) {
            glVertex3f(parent->GridXstart, gY, 0);
            glVertex3f(parent->GridXend, gY, 0);
        }

        glEnd();
    }

    if (parent->ShowPoints) {
        glPointSize(1.0f);
        //         glLineWidth(0.3f);
        glColor3f(Settings::colorSettings[COLOR_GRID].r, Settings::colorSettings[COLOR_GRID].g, Settings::colorSettings[COLOR_GRID].b); // white

        glBegin(GL_POINTS);


        for (int y = parent->GridYstart; y <  parent->GridYend + 1; y += parent->GrigStep) {
            for (int x = parent->GridXstart; x < parent->GridXend + 1; x += parent->GrigStep) {
                //point
                glVertex3f(x, y, 0);
            }
        }

        glEnd();
    }
}

/**
 * @brief
 *
 */
void GLWidget::drawTool()
{
    glPushMatrix();

    glLineWidth(2);

    glDisable(GL_DEPTH_TEST); // because of text rendering

    glEnable(GL_VERTEX_ARRAY);

    glDisable(GL_NORMAL_ARRAY);

    glScalef(1.5, 1.5, 1.5);

    // foot
    glColor3f(Settings::colorSettings[COLOR_TRAVERSE].r, Settings::colorSettings[COLOR_TRAVERSE].g, Settings::colorSettings[COLOR_TRAVERSE].b);
    glVertexPointer(3, GL_FLOAT, 0, &footArray[0]);
    // GL_LINE_LOOP or GL_POLYGON
    glDrawArrays(GL_LINE_LOOP, 0, footArray.count()); // draw array of lines

    glTranslatef(3.6, 16.0, 12.0);

    glVertexPointer(3, GL_FLOAT, 0, &traverseArray[0]);
    // GL_LINE_LOOP or GL_POLYGON
    glDrawArrays(GL_LINE_LOOP, 0, traverseArray.count()); // draw array of lines

    glTranslatef(64.0, -16.0, -12.0);
    glVertexPointer(3, GL_FLOAT, 0, &footArray[0]);
    // GL_LINE_LOOP or GL_POLYGON
    glDrawArrays(GL_LINE_LOOP, 0, footArray.count()); // draw array of lines

    glDisable(GL_VERTEX_ARRAY);

    glEnable(GL_NORMAL_ARRAY);

    glEnable(GL_DEPTH_TEST);

    glPopMatrix();
}

/**
 * @brief
 *
 */
void GLWidget::drawSurface()
{
    int maxY, maxX;
    maxY = parent->surfaceMatrix.size();

    if (maxY == 0) {
        return;
    }

    maxX = parent->surfaceMatrix[0].size(); // because of rectangle matrix

    if (maxX == 0) {
        return;
    }

    //points
    glColor3f(Settings::colorSettings[COLOR_SURFACE].r, Settings::colorSettings[COLOR_SURFACE].g, Settings::colorSettings[COLOR_SURFACE].b); // red
    glPointSize(10.0F);

    glBegin(GL_POINTS);

    for (int y = 0; y < maxY; y++) {
        for (int x = 0; x < maxX; x++) {
            //point
            glVertex3f(parent->surfaceMatrix[y][x].X, parent->surfaceMatrix[y][x].Y, parent->surfaceMatrix[y][x].Z);
        }
    }

    glEnd();

    // connections between the points
    //     glColor3f(0.678f, 1.000f, 0.184f); // green
    glColor3f(Settings::colorSettings[COLOR_CONNECTION].r, Settings::colorSettings[COLOR_CONNECTION].g, Settings::colorSettings[COLOR_CONNECTION].b);
    glLineWidth(0.4f);
    glBegin(GL_LINES);

    for (int y = 0; y < maxY; y++) {
        for (int x = 0; x < maxX; x++) {
            if (y > 0) {
                //line 1
                glVertex3f(parent->surfaceMatrix[y][x].X, parent->surfaceMatrix[y][x].Y, parent->surfaceMatrix[y][x].Z);
                glVertex3f(parent->surfaceMatrix[y - 1][x].X, parent->surfaceMatrix[y - 1][x].Y, parent->surfaceMatrix[y - 1][x].Z);
            }

            if (y < maxY - 1) {
                //line2
                glVertex3f(parent->surfaceMatrix[y][x].X, parent->surfaceMatrix[y][x].Y, parent->surfaceMatrix[y][x].Z);
                glVertex3f(parent->surfaceMatrix[y + 1][x].X, parent->surfaceMatrix[y + 1][x].Y, parent->surfaceMatrix[y + 1][x].Z);
            }

            if (x > 0) {
                //line 3
                glVertex3f(parent->surfaceMatrix[y][x].X, parent->surfaceMatrix[y][x].Y, parent->surfaceMatrix[y][x].Z);
                glVertex3f(parent->surfaceMatrix[y][x - 1].X, parent->surfaceMatrix[y][x - 1].Y, parent->surfaceMatrix[y][x - 1].Z);
            }

            if (x < maxX - 1) {
                //line4
                glVertex3f(parent->surfaceMatrix[y][x].X, parent->surfaceMatrix[y][x].Y, parent->surfaceMatrix[y][x].Z);
                glVertex3f(parent->surfaceMatrix[y][x + 1].X, parent->surfaceMatrix[y][x + 1].Y, parent->surfaceMatrix[y][x + 1].Z);
            }
        }
    }

    glEnd();
}

/**
 * @brief
 *
 */
void GLWidget::drawInstrument()
{
    // instrument
    float startX =  Settings::coord[X].posMm();
    float startY =  Settings::coord[Y].posMm();
    float startZ =  Settings::coord[Z].posMm();

    glColor3f(Settings::colorSettings[COLOR_TOOL].r, Settings::colorSettings[COLOR_TOOL].g, Settings::colorSettings[COLOR_TOOL].b);
    glLineWidth(3);

    glTranslatef(startX, startY, startZ);

    glVertexPointer(3, GL_FLOAT, 0, &instrumentArray[0]);

    glEnable(GL_VERTEX_ARRAY);

    glDisable(GL_NORMAL_ARRAY);
    glDisable(GL_TEXTURE_COORD_ARRAY);

    glDrawArrays(GL_LINES, 0, instrumentArray.count());

    glDisable(GL_VERTEX_ARRAY);

    glEnable(GL_NORMAL_ARRAY);
    glEnable(GL_TEXTURE_COORD_ARRAY);
}

/**
 * @brief
 *
 */
void GLWidget::drawGrate()
{
    //
    glLineWidth(2.0f);

    glColor3f(Settings::colorSettings[COLOR_GRID].r, Settings::colorSettings[COLOR_GRID].g, Settings::colorSettings[COLOR_GRID].b);

    glEnable(GL_LINE_STIPPLE);

    GLushort pattern = 0x00FF;
    GLint factor = 2;

    glLineStipple( factor, pattern );

    glBegin(GL_LINE_STRIP); // normal lines

    glVertex3f( Settings::coord[X].softLimitMin,  Settings::coord[Y].softLimitMin, 0);
    glVertex3f( Settings::coord[X].softLimitMax,  Settings::coord[Y].softLimitMin, 0);
    glVertex3f( Settings::coord[X].softLimitMax,  Settings::coord[Y].softLimitMax, 0);
    glVertex3f( Settings::coord[X].softLimitMin,  Settings::coord[Y].softLimitMax, 0);
    glVertex3f( Settings::coord[X].softLimitMin,  Settings::coord[Y].softLimitMin, 0);

    glEnd();

    glDisable(GL_LINE_STIPPLE);
}

/**
 * @brief when angle over 360 or under 0, normalize it
 *
 */
void GLWidget::normalizeAngle(int &angle)
{
    while (angle < 0) {
        angle += 360;
    }

    while (angle > 360) {
        angle -= 360;
    }
}


/**
 * @brief
 *
 */
// from slider
void GLWidget::setXCoord(int x)
{
    if (parent->PosX != x) {
        parent->PosX = x;
    }
}

/**
 * @brief
 *
 */
void GLWidget::setYCoord(int y)
{
    if (parent->PosY != y) {
        parent->PosY = y;
    }
}


/**
 * @brief
 *
 */
// from slider
void GLWidget::setXRotation(int angle)
{
    normalizeAngle(angle);

    if (angle != parent->PosAngleX) {
        parent->PosAngleX = angle;
        emit rotationChanged();
    }
}

/**
 * @brief
 *
 */
void GLWidget::setYRotation(int angle)
{
    normalizeAngle(angle);

    if (angle != parent->PosAngleY) {
        parent->PosAngleY = angle;
        emit rotationChanged();
    }
}

/**
 * @brief
 *
 */
void GLWidget::setZRotation(int angle)
{
    normalizeAngle(angle);

    if (angle != parent->PosAngleZ) {
        parent->PosAngleZ = angle;
        emit rotationChanged();
    }
}

/**
 * @brief
 *
 */
void GLWidget::onPosAngleXm()
{
    --parent->PosAngleX;
    normalizeAngle(parent->PosAngleX);
    emit rotationChanged();
    updateGL();
}

/**
 * @brief
 *
 */
void GLWidget::onPosAngleX()
{
    parent->PosAngleX = 0;
    emit rotationChanged();
    updateGL();
}

/**
 * @brief
 *
 */
void GLWidget::onPosAngleXp()
{
    ++parent->PosAngleX;
    normalizeAngle(parent->PosAngleX);
    emit rotationChanged();
    updateGL();
}

/**
 * @brief
 *
 */
void GLWidget::onPosAngleYp()
{
    ++parent->PosAngleY;
    normalizeAngle(parent->PosAngleY);
    emit rotationChanged();
    updateGL();
}

/**
 * @brief
 *
 */
void GLWidget::onPosAngleY()
{
    parent->PosAngleY = 0;
    emit rotationChanged();
    updateGL();

}

/**
 * @brief
 *
 */
void GLWidget::onPosAngleYm()
{
    --parent->PosAngleY;
    normalizeAngle(parent->PosAngleY);
    emit rotationChanged();
    updateGL();
}

/**
 * @brief
 *
 */
void GLWidget::onPosAngleZp()
{
    ++parent->PosAngleZ;
    normalizeAngle(parent->PosAngleZ);
    emit rotationChanged();
    updateGL();
}

/**
 * @brief
 *
 */
void GLWidget::onPosAngleZ()
{
    parent->PosAngleZ = 0;
    emit rotationChanged();
    updateGL();
}

/**
 * @brief
 *
 */
void GLWidget::onPosAngleZm()
{
    --parent->PosAngleZ;
    normalizeAngle(parent->PosAngleZ);
    emit rotationChanged();
    updateGL();
}

/**
 * @brief
 *
 */
void GLWidget::onDefaulPreview()
{
    initPreviewSettings();
}

/**
 * @brief
 *
 */
void GLWidget::onRenderTimer()
{
    // ??
    Draw();
}

/**
 * @brief
 *
 */
void GLWidget::saveGLState()
{
#if USE_GLES2 == false
    glPushAttrib(GL_ALL_ATTRIB_BITS);
#endif
//     glMatrixMode(GL_PROJECTION);
//     glPushMatrix();
//     glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
}

/**
 * @brief
 *
 */
void GLWidget::restoreGLState()
{
//     glMatrixMode(GL_PROJECTION);
//     glPopMatrix();
//     glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
#if USE_GLES2 == false
    glPopAttrib();
#endif
}

