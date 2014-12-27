#include "Object.h"
#include "Frame.h"

#include <QMessageBox>

#include <Eigen/LU>
#include <Eigen/Geometry>

using namespace Eigen;

BlinnPhong Object::ms_defaultMaterial(Eigen::Array3f::Constant(0.7), Eigen::Array3f::Constant(0.7), 5);

Object::Object()
    : mShader(0), mShape(0), mMaterial(&ms_defaultMaterial), mTransformation(Eigen::Matrix4f::Identity())
{}

Object::Object(const QDomElement& e)
    : mShader(0), mShape(0)
{
    QDomNode n = e.firstChild();
    while (!n.isNull())
    {
        QDomElement e = n.toElement();
        if (!e.isNull())
        {
            if (e.tagName() == "Material"){
                mMaterial = new BlinnPhong(e);
            }
            else if (e.tagName() == "Frame")
            {
                Frame f(e);
                mTransformation = f.getMatrix();
            }
        }
        else
            QMessageBox::warning(NULL, "Object XML error", "Error while parsing Object XML document");
        n = n.nextSibling();
    }
}

void Object::attachShape(const Shape* shape)
{
    mShape = shape;
}

void Object::attachShader(const Shader* shader)
{
    mShader = shader;
}

void Object::setTransformation(const Eigen::Matrix4f& mat)
{
    mTransformation = mat;
}

void Object::draw()
{
    if(mShape && mShader)
    {
        mShader->activate();
        GL_TEST_ERR;
        glUniformMatrix4fv(glGetUniformLocation(mShader->id(),"mat_obj"),  1, GL_FALSE, mTransformation.data());
        GL_TEST_ERR;
        glUniformMatrix3fv(glGetUniformLocation(mShader->id(),"mat_normal"),  1, GL_FALSE, Eigen::Matrix3f(mTransformation.topLeftCorner<3,3>().inverse().transpose()).data());
        GL_TEST_ERR;
        glUniform3fv(glGetUniformLocation(mShader->id(),"ambient_color"), 1, mMaterial->ambientColor().data());
        mShape->drawGeometry(mShader->id());
        GL_TEST_ERR;
    }
}

