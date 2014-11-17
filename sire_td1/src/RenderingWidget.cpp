#include "RenderingWidget.h"

#include "Raytracing.h"
#include "GLPrimitives.h"

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
}

RenderingWidget::~RenderingWidget()
{
}

void RenderingWidget::paintGL()
{
  makeCurrent();
  GL_TEST_ERR;

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

  glUniformMatrix4fv(glGetUniformLocation(mFlatProgram.id(),"mat_obj"), 1, GL_FALSE, Eigen::Affine3f::Identity().data());

  // TODO : draw ray
  GLLine::draw(mFlatProgram.id(), mRay.origin,  mRay.origin + mRay.direction);
    
  // Tester et dessiner si intersection
  if (mHit.foundIntersection())
    Point::draw(mFlatProgram.id(), mRay.at(mHit.t()));

  GL_TEST_ERR;
}

void RenderingWidget::initializeGL()
{
  makeCurrent();
  GL_TEST_ERR;
  std::cout << "Using OpenGL version: \"" << glGetString(GL_VERSION) << "\"" << std::endl;
  std::cout << "OpenGL context: " << context()->format().majorVersion()
	    << "." << context()->format().minorVersion()
	    << " " << ((context()->format().profile() == QGLFormat::CoreProfile)? "Core":
		       (context()->format().profile() == QGLFormat::CompatibilityProfile)? "Compatibility":
		       "No profile")
	    << std::endl;

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
        QImage img = Raytracing::raytraceImage(mScene);
        img.save("filename.png");
	break;
      }
    default:
      break;
    }
}

void RenderingWidget::select(const QPoint point)
{
  mGLCamera.convertClickToLine(point, mRay.origin, mRay.direction);
  mScene.intersect(mRay, mHit);
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
	mHit.reset();
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
	  mGLCamera.zoom(dy*10);
	  break;
        case TM_FLY_Z :
	  mGLCamera.localTranslate(Vector3f(0, 0, -dy*50));
	  break;
        case TM_FLY_PAN :
	  mGLCamera.localTranslate(Vector3f(dx*50, dy*50, 0));
	  break;
        default:
	  break;
        }

      updateGL();
    }

  mMouseCoords = Vector2i(e->pos().x(), e->pos().y());
}

#include <RenderingWidget.moc>
