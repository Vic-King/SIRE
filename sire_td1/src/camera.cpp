
// Copyright (C) 2008 Gael Guennebaud <gael.guennebaud@inria.fr>
//
// Eigen is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or (at your option) any later version.
//
// Alternatively, you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// Eigen is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License or the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License and a copy of the GNU General Public License along with
// Eigen. If not, see <http://www.gnu.org/licenses/>.

#include "camera.h"
#include "OpenGL.h"

#include <iostream>
#include <QMessageBox>

#include <Eigen/LU>
using namespace Eigen;

Camera::Camera()
    : mViewIsUptodate(false), mProjIsUptodate(false)
{
    mViewMatrix.setIdentity();
    
    mFovY = M_PI/3.;
    mNearDist = 0.1;
    mFarDist = 50000.;
    
    mVpX = 0;
    mVpY = 0;

    setPosition(Vector3f::Constant(100.));
    lookAt(Vector3f::Constant(100.), Vector3f::Zero(), Vector3f::UnitZ());

    mIsInitialized = false;
}

Camera& Camera::operator=(const Camera& other)
{
    mViewIsUptodate = false;
    mProjIsUptodate = false;
    
    mVpX = other.mVpX;
    mVpY = other.mVpY;
    mVpWidth = other.mVpWidth;
    mVpHeight = other.mVpHeight;

    mTarget = other.mTarget;
    mFovY = other.mFovY;
    mNearDist = other.mNearDist;
    mFarDist = other.mFarDist;
    
    mViewMatrix = other.mViewMatrix;
    mProjectionMatrix = other.mProjectionMatrix;

    mFrame = other.mFrame;

    mIsInitialized = false;

    return *this;
}

Camera::Camera(const Camera& other)
{
    *this = other;
}

Camera::~Camera()
{
}

void Camera::setPerspective(float fovY, float near, float far)
{
    mFovY = fovY;
    mNearDist = near;
    mFarDist = far;
    mProjIsUptodate = false;
}

void Camera::setViewport(uint offsetx, uint offsety, uint width, uint height)
{
    mVpX = offsetx;
    mVpY = offsety;
    mVpWidth = width;
    mVpHeight = height;

    mProjIsUptodate = false;
}

void Camera::setViewport(uint width, uint height)
{
    mVpWidth = width;
    mVpHeight = height;

    mProjIsUptodate = false;
}

void Camera::setFovY(float value)
{
    mFovY = value;
    mProjIsUptodate = false;
}

Vector3f Camera::direction() const
{
    updateViewMatrix();
    return -mViewMatrix.linear().row(2);
}
Vector3f Camera::up() const
{
    updateViewMatrix();
    return mViewMatrix.linear().row(1);
}
Vector3f Camera::right() const
{
    updateViewMatrix();
    return mViewMatrix.linear().row(0);
}

void Camera::lookAt(const Eigen::Vector3f& position, const Eigen::Vector3f& target, const Eigen::Vector3f& up)
{
    mTarget = target;
    mFrame.position = position;
    Matrix3f R;
    R.col(2) = (position-target).normalized();
    R.col(0) = up.cross(R.col(2)).normalized();
    R.col(1) = R.col(2).cross(R.col(0));
    setOrientation(Quaternionf(R));
    mViewIsUptodate = false;
}

void Camera::setPosition(const Vector3f& p)
{
    mFrame.position = p;
    mViewIsUptodate = false;
}

void Camera::setOrientation(const Quaternionf& q)
{
    mFrame.orientation = q;
    mViewIsUptodate = false;
}

void Camera::setFrame(const Frame& f)
{
    mFrame = f;
    mViewIsUptodate = false;
}

void Camera::rotateAroundTarget(const Quaternionf& q)
{
    Matrix4f mrot, mt, mtm;

    // update the transform matrix
    updateViewMatrix();
    Vector3f t = mViewMatrix * mTarget;

    mViewMatrix = Translation3f(t)
            * q
            * Translation3f(-t)
            * mViewMatrix;

    Quaternionf qa(mViewMatrix.linear());
    qa = qa.conjugate();
    setOrientation(qa);
    setPosition(- (qa * mViewMatrix.translation()) );

    mViewIsUptodate = true;
}

void Camera::localRotate(const Quaternionf& q)
{
    float dist = (position() - mTarget).norm();
    setOrientation(orientation() * q);
    mTarget = position() + dist * direction();
    mViewIsUptodate = false;
}

void Camera::zoom(float d)
{
    float dist = (position() - mTarget).norm();
    if(dist > d)
    {
        setPosition(position() + direction() * d);
        mViewIsUptodate = false;
    }
}

void Camera::localTranslate(const Vector3f& t)
{
    Vector3f trans = orientation() * t;
    setPosition( position() + trans );
    mTarget += trans;

    mViewIsUptodate = false;
}

void Camera::updateViewMatrix() const
{
    if(!mViewIsUptodate)
    {
        Quaternionf q = orientation().conjugate();
        mViewMatrix.linear() = q.toRotationMatrix();
        mViewMatrix.translation() = - (mViewMatrix.linear() * position());

        mViewIsUptodate = true;
    }
}

const Affine3f& Camera::viewMatrix() const
{
    updateViewMatrix();
    return mViewMatrix;
}

void Camera::updateProjectionMatrix() const
{
    if(!mProjIsUptodate)
    {
        mProjectionMatrix.setIdentity();
        float aspect = float(mVpWidth)/float(mVpHeight);
        float theta = mFovY*0.5;
        float range = mFarDist - mNearDist;
        float invtan = 1./tan(theta);

        mProjectionMatrix(0,0) = invtan / aspect;
        mProjectionMatrix(1,1) = invtan;
        mProjectionMatrix(2,2) = -(mNearDist + mFarDist) / range;
        mProjectionMatrix(3,2) = -1;
        mProjectionMatrix(2,3) = -2 * mNearDist * mFarDist / range;
        mProjectionMatrix(3,3) = 0;

        mProjIsUptodate = true;
    }
}

const Matrix4f& Camera::projectionMatrix() const
{
    updateProjectionMatrix();
    return mProjectionMatrix;
}


Vector3f Camera::unProject(const Vector2f& uv, float depth) const
{
    Matrix4f inv = mViewMatrix.inverse().matrix();
    return unProject(uv, depth, inv);
}

Vector3f Camera::unProject(const Vector2f& uv, float depth, const Matrix4f& invModelview) const
{
    updateViewMatrix();
    updateProjectionMatrix();

    Vector3f a(2.*uv.x()/float(mVpWidth)-1., 2.*uv.y()/float(mVpHeight)-1., 1.);
    a.x() *= depth/mProjectionMatrix(0,0);
    a.y() *= depth/mProjectionMatrix(1,1);
    a.z() = -depth;
    //Vector4f b = invModelview * Vector4f(a.x(), a.y(), a.z(), 1.);
    //Vector3f b = mViewMatrix.linear().transpose() * a + position();//Vector4f(a.x(), a.y(), a.z(), 1.);

    Vector3f b = a.x() * right() + a.y() * up() - a.z() * direction() + position();

    return Vector3f(b.x(), b.y(), b.z());
}

void Camera::initFromDomElement(const QDomElement &e)
{
    if (e.tagName() != "Camera")
    {
        QMessageBox::critical(NULL, "Camera init error", "Camera::initFromDOMElement, bad DOM tagName.\nExpecting 'Camera', got "+e.tagName());
        return;
    }

    if (!e.hasAttribute("fieldOfView"))
        QMessageBox::warning(NULL, "Camera error", "Camera has undefined fieldOfView. Using pi/4.");
    mFovY = e.attribute("fieldOfView", "0.7854").toFloat();

    if (!e.hasAttribute("xResolution"))
        QMessageBox::warning(NULL, "Camera error", "Camera has undefined xResolution. Using 64.");
    mVpWidth = e.attribute("xResolution", "64").toInt();

    if (!e.hasAttribute("yResolution"))
        QMessageBox::warning(NULL, "Camera error", "Camera has undefined yResolution. Using 64.");
    mVpHeight = e.attribute("yResolution", "64").toInt();

    QDomNode n = e.firstChild();
    while (!n.isNull())
    {
        QDomElement e = n.toElement();
        if (!e.isNull())
        {
            if (e.tagName() == "Frame")
                mFrame = Frame(e);
            else
                QMessageBox::warning(NULL, "Camera child error", "Unsupported Camera child : "+e.tagName());
        }
        else
            QMessageBox::warning(NULL, "Camera XML error", "Error while parsing Camera XML document");
        n = n.nextSibling();
    }

    mViewIsUptodate = false;
    updateViewMatrix();
    mProjIsUptodate = false;
    updateProjectionMatrix();

    // Clean up OpenGL buffers
    mIsInitialized = false;
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &mVertexBufferId);
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &mVertexArrayId);
}

void Camera::draw(int prg_id)
{
    if(!mIsInitialized)
    {
        mIsInitialized = true;
        mPoints.clear();

        // grille
        float ym = tan(mFovY*0.5);
        float xm = ((float)mVpWidth)*(ym*1.0/mVpHeight);
        float zm = 0.75f;
        for(int x=1; x<mVpWidth; ++x){
            mPoints.push_back(Vector3f(xm*(x*2.0/mVpWidth-1.0),ym,-zm));
            mPoints.push_back(Vector3f(xm*(x*2.0/mVpWidth-1.0),-ym,-zm));
        }
        for(int y=1; y<mVpHeight; ++y){
            mPoints.push_back(Vector3f(xm,ym*(y*2.0/mVpHeight-1.0),-zm));
            mPoints.push_back(Vector3f(-xm,ym*(y*2.0/mVpHeight-1.0),-zm));
        }

        //pyramide
        mPoints.push_back(Vector3f(0.0,0.0,0.0));
        mPoints.push_back(Vector3f(xm,ym,-zm));
        mPoints.push_back(Vector3f(xm,ym,-zm));
        mPoints.push_back(Vector3f(xm,-ym,-zm));
        mPoints.push_back(Vector3f(xm,-ym,-zm));
        mPoints.push_back(Vector3f(0.0,0.0,0.0));
        mPoints.push_back(Vector3f(0.0,0.0,0.0));
        mPoints.push_back(Vector3f(-xm,ym,-zm));
        mPoints.push_back(Vector3f(-xm,ym,-zm));
        mPoints.push_back(Vector3f(-xm,-ym,-zm));
        mPoints.push_back(Vector3f(-xm,-ym,-zm));
        mPoints.push_back(Vector3f(0.0,0.0,0.0));
        mPoints.push_back(Vector3f(0.0,0.0,0.0));
        mPoints.push_back(Vector3f(xm,ym,-zm));
        mPoints.push_back(Vector3f(xm,ym,-zm));
        mPoints.push_back(Vector3f(-xm,ym,-zm));
        mPoints.push_back(Vector3f(-xm,ym,-zm));
        mPoints.push_back(Vector3f(0.0,0.0,0.0));
        mPoints.push_back(Vector3f(0.0,0.0,0.0));
        mPoints.push_back(Vector3f(xm,-ym,-zm));
        mPoints.push_back(Vector3f(xm,-ym,-zm));
        mPoints.push_back(Vector3f(-xm,-ym,-zm));
        mPoints.push_back(Vector3f(-xm,-ym,-zm));
        mPoints.push_back(Vector3f(0.0,0.0,0.0));

        glGenBuffers(1,&mVertexBufferId);
        glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferId);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3f)*mPoints.size(), mPoints[0].data(), GL_STATIC_DRAW);

        glGenVertexArrays(1,&mVertexArrayId);GL_TEST_ERR;
    }

    // bind the vertex array
    glBindVertexArray(mVertexArrayId);

    glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferId);

    int vertex_loc = glGetAttribLocation(prg_id, "vtx_position");
    glVertexAttribPointer(vertex_loc, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(vertex_loc);GL_TEST_ERR;

    glUniformMatrix4fv(glGetUniformLocation(prg_id,"mat_obj"), 1, GL_FALSE, mFrame.getMatrix().data());

    glDrawArrays(GL_LINES,0,mPoints.size());GL_TEST_ERR;

    glDisableVertexAttribArray(vertex_loc);

    // release the vertex array
    glBindVertexArray(0);GL_TEST_ERR;
}

void Camera::convertClickToLine(const QPoint& pixel, Vector3f& orig, Vector3f& dir) const
{
    orig = position();
    dir = Vector3f( ((2.0 * pixel.x() / vpWidth()) - 1.0) * tan(fovY()/2.0) * vpWidth() / vpHeight(),
                    ((2.0 * (vpHeight() - pixel.y()) / vpHeight()) - 1.0) * tan(fovY()/2.0),
                    -1.0 );
    Matrix3f rotation = mFrame.orientation.toRotationMatrix();
    dir = rotation * dir + mFrame.position;
    dir = dir - orig;
    dir.normalize();
}
