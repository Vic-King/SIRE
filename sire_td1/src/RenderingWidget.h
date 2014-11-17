
#ifndef RENDERINGWIDGET_H
#define RENDERINGWIDGET_H

#include "OpenGL.h"
#include <QGLWidget>
#include <QApplication>

#include "Shader.h"
#include "trackball.h"
#include "Scene.h"

class RenderingWidget : public QGLWidget
{
    Q_OBJECT

    // A scene contains a list of objects, a list of light sources and a camera.
    Scene mScene;
    
    // main shader program
    Shader mProgram;
    // line shader program
    Shader mFlatProgram;

    // OpenGL camera
    Camera mGLCamera;
    bool mDrawCamera;

    Ray mRay;
    Hit mHit;

    enum TrackMode {
      TM_NO_TRACK=0, TM_ROTATE_AROUND, TM_ZOOM,
      TM_LOCAL_ROTATE, TM_FLY_Z, TM_FLY_PAN
    };

    TrackMode mCurrentTrackingMode;
    Eigen::Vector2i mMouseCoords;
    Trackball mTrackball;

  protected:

    /** This method is automatically called by Qt once the GL context has been created.
      * It is called only once per execution */
    virtual void initializeGL();

    /** This method is automatically called by Qt everytime the opengl windows is resized.
      * \param width the new width of the windows (in pixels)
      * \param height the new height of the windows (in pixels)
      *
      * This function must never be called directly. To redraw the windows, emit the updateGL() signal:
      \code
      emit updateGL();
      \endcode
      */
    virtual void resizeGL(int width, int height);

    /** This method is automatically called by Qt everytime the opengl windows has to be refreshed. */
    virtual void paintGL();

    /** This method is automatically called by Qt everytime a key is pressed */
    void keyPressEvent(QKeyEvent * e);

    /** Internal function to load a 3D scene from a file */
    virtual void loadScene();

    void select(const QPoint point);

    //--------------------------------------------------------------------------------
    virtual void mousePressEvent(QMouseEvent * e);
    virtual void mouseReleaseEvent(QMouseEvent * e);
    virtual void mouseMoveEvent(QMouseEvent * e);
    virtual void wheelEvent(QWheelEvent * e);
    //--------------------------------------------------------------------------------

  public: 
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    
    // default constructor
    RenderingWidget();
    ~RenderingWidget();
};

#endif // RENDERINGWIDGET_H

