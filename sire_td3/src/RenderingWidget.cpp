#include "RenderingWidget.h"

#include "Raytracing.h"
#include "GLPrimitives.h"
#include "Mesh.h"

#include <Eigen/Geometry>
#include <iostream>
#include <QKeyEvent>
#include <QFileDialog>
#include <QImage>

using namespace Eigen;

RenderingWidget::RenderingWidget()
//#ifdef __APPLE__
//    :
//      QGLWidget(new Core3_2_context(QGLFormat::defaultFormat()))
//    #endif
{
    mDrawCamera = false;
    mDrawRay = false;
    mDrawAABB = false;
}

RenderingWidget::~RenderingWidget()
{
}

void RenderingWidget::paintGL()
{
    makeCurrent();
    GL_TEST_ERR;

    glClearColor(mScene.backgroundColor()[0],mScene.backgroundColor()[1],mScene.backgroundColor()[2],1);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // configure the rendering target size (viewport)
    glViewport(0, 0, mGLCamera.vpWidth(), mGLCamera.vpHeight());

    mProgram.activate();
    glUniformMatrix4fv(glGetUniformLocation(mProgram.id(),"mat_view"), 1, GL_FALSE, mGLCamera.viewMatrix().data());
    glUniformMatrix4fv(glGetUniformLocation(mProgram.id(),"mat_proj"), 1, GL_FALSE, mGLCamera.projectionMatrix().data());
    Vector3f lightDir = Vector3f(1,1,1).normalized();
    glUniform3fv(glGetUniformLocation(mProgram.id(),"light_dir"),  1, lightDir.data());
    glUniform3fv(glGetUniformLocation(mProgram.id(),"cam_pos"),  1, mGLCamera.position().data());
    GL_TEST_ERR;

    mScene.draw();

    GL_TEST_ERR;

    mFlatProgram.activate();
    glUniformMatrix4fv(glGetUniformLocation(mFlatProgram.id(),"mat_view"), 1, GL_FALSE, mGLCamera.viewMatrix().data());
    glUniformMatrix4fv(glGetUniformLocation(mFlatProgram.id(),"mat_proj"), 1, GL_FALSE, mGLCamera.projectionMatrix().data());
    GL_TEST_ERR;

    if(mDrawCamera)
    {
        glUniform3f(glGetUniformLocation(mFlatProgram.id(),"color"),0.75f,0.75f,0.75f);
        mScene.camera().draw(mFlatProgram.id());
    }

    if(mDrawAABB) // Draw bounding boxes
    {
        for(int i=0; i<mScene.objectList().size(); ++i)
        {
            glUniformMatrix4fv(glGetUniformLocation(mFlatProgram.id(),"mat_obj"), 1, GL_FALSE, mScene.objectList()[i]->transformation().data());
            glUniform3f(glGetUniformLocation(mFlatProgram.id(),"color"),0.9f,0.9f,0.9f);
            GLBox::draw(mFlatProgram.id(), mScene.objectList()[i]->shape()->AABB());
        }
    }

    glUniformMatrix4fv(glGetUniformLocation(mFlatProgram.id(),"mat_obj"), 1, GL_FALSE, Eigen::Affine3f::Identity().data());

    if(mDrawRay){
        glUniform3f(glGetUniformLocation(mFlatProgram.id(),"color"),0.9f,0.9f,0.9f);
        GLLine::draw(mFlatProgram.id(), mRay.origin, mRay.at(10.f));

        if(mHit.foundIntersection())
        {
            Vector3f x = mRay.at(mHit.t());
            glUniform3f(glGetUniformLocation(mFlatProgram.id(),"color"),0.9f,0.f,0.f);
            GLPoint::draw(mFlatProgram.id(), x);

            glUniform3f(glGetUniformLocation(mFlatProgram.id(),"color"),0.f,0.8f,0.0f);
            GLLine::draw(mFlatProgram.id(), x, x + 0.25f * mHit.normal());
        }
    }

    GL_TEST_ERR;
}

void RenderingWidget::initializeGL()
{
    makeCurrent();
    GL_TEST_ERR;
    std::cout << "Using OpenGL version: \"" << glGetString(GL_VERSION) << "\"" << std::endl;

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.6,0.6,0.6,1);

    // load the default shaders
    mProgram.loadFromFiles(SIRE_DIR"/shaders/simple.vert", SIRE_DIR"/shaders/simple.frag");
    mFlatProgram.loadFromFiles(SIRE_DIR"/shaders/flat.vert", SIRE_DIR"/shaders/flat.frag");

    mScene.createDefaultScene(mProgram);
    mGLCamera = Camera(mScene.camera());

    // Assign camera to trackball
    mTrackball.setCamera(&mGLCamera);
    GL_TEST_ERR;
}

void RenderingWidget::resizeGL(int width, int height)
{
    mGLCamera.setViewport(width,height);
}

void RenderingWidget::loadScene()
{
    QString name = QFileDialog::getOpenFileName(this, "Select a scene", ".", "Scenes (*.scn);;All files (*)");

    if (name.isEmpty())
        return;

    mScene.loadFromFile(name);
    updateGL();
}

void RenderingWidget::keyPressEvent(QKeyEvent * e)
{
    switch(e->key())
    {
    case Qt::Key_Up:
        break;
    case Qt::Key_Down:
        break;
    case Qt::Key_Left:
        break;
    case Qt::Key_Right:
        break;
    case Qt::Key_L:
    {
        loadScene();
        break;
    }
    case Qt::Key_C:
    {
        mScene.setCamera(mGLCamera);
        updateGL();
        break;
    }
    case Qt::Key_H:
    {
        mDrawCamera = !mDrawCamera;
        updateGL();
        break;
    }
    case Qt::Key_R:
    {
        int t = clock();
        Mesh::ms_itersection_count = 0;
        QImage img = Raytracing::raytraceImage(mScene);
        t = clock() - t;
        std::cout << "Raytracing time : " << float(t)/CLOCKS_PER_SEC << "s  -  nb triangle intersection: " << Mesh::ms_itersection_count << "\n";
        img.save("filename.png");
        break;
    }
    case Qt::Key_B:
    {
        mDrawAABB = !mDrawAABB;
        updateGL();
    break;
    }
    default:
        break;
    }
}

void RenderingWidget::select(const QPoint point)
{
    Eigen::Vector3f orig, dir;
    mGLCamera.convertClickToLine(point,orig,dir);
    mRay.origin = orig;
    mRay.direction = dir;
    mHit = Hit();
    mScene.intersect(mRay,mHit);
    mDrawRay = true;
    updateGL();
}

void RenderingWidget::mousePressEvent(QMouseEvent* e)
{
    mMouseCoords = Vector2i(e->pos().x(), e->pos().y());
    bool fly = (e->modifiers()&Qt::ControlModifier);
    bool shift = (e->modifiers()&Qt::ShiftModifier);
    switch(e->button())
    {
    case Qt::LeftButton:
        if(shift){
            select(e->pos());
        }
        else if(fly)
        {
            mCurrentTrackingMode = TM_LOCAL_ROTATE;
            mTrackball.start(Trackball::Local);
            mTrackball.track(mMouseCoords);
        }
        else
        {
            mCurrentTrackingMode = TM_ROTATE_AROUND;
            mTrackball.start(Trackball::Around);
            mTrackball.track(mMouseCoords);
        }
        break;
    case Qt::MidButton:
        if(fly)
            mCurrentTrackingMode = TM_FLY_Z;
        else
            mCurrentTrackingMode = TM_ZOOM;
        break;
    case Qt::RightButton:
        mCurrentTrackingMode = TM_FLY_PAN;
        break;
    default:
        break;
    }
}
void RenderingWidget::mouseReleaseEvent(QMouseEvent*)
{
    mCurrentTrackingMode = TM_NO_TRACK;
    updateGL();
}

void RenderingWidget::wheelEvent(QWheelEvent * e)
{
    mGLCamera.zoom(e->delta()*0.01);
    updateGL();
}

void RenderingWidget::mouseMoveEvent(QMouseEvent* e)
{
    // tracking
    if(mCurrentTrackingMode != TM_NO_TRACK)
    {
        float dx =   float(e->x() - mMouseCoords.x()) / float(mGLCamera.vpWidth());
        float dy = - float(e->y() - mMouseCoords.y()) / float(mGLCamera.vpHeight());

        // speedup the transformations
        if(e->modifiers() & Qt::ShiftModifier)
        {
            dx *= 10.;
            dy *= 10.;
        }

        switch(mCurrentTrackingMode)
        {
        case TM_ROTATE_AROUND:
        case TM_LOCAL_ROTATE:
            mTrackball.track(Vector2i(e->pos().x(), e->pos().y()));
            break;
        case TM_ZOOM :
            mGLCamera.zoom(dy*100);
            break;
        case TM_FLY_Z :
            mGLCamera.localTranslate(Vector3f(0, 0, -dy*200));
            break;
        case TM_FLY_PAN :
            mGLCamera.localTranslate(Vector3f(dx*200, dy*200, 0));
            break;
        default:
            break;
        }

        updateGL();
    }

    mMouseCoords = Vector2i(e->pos().x(), e->pos().y());
}

#include <RenderingWidget.moc>
