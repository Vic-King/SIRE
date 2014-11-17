#include "Frame.h"

#include <QStringList>
#include <iostream>

using namespace Eigen;

Frame::Frame(const QDomElement& e)
{
    QDomElement child = e.firstChild().toElement();
    while (!child.isNull())
    {
        QStringList attribute;
        if (child.tagName() == "position"){
            attribute << "x" << "y" << "z";
            for (int i=0; i<attribute.size(); ++i){
                if (child.hasAttribute(attribute[i])) {
                    const QString s = child.attribute(attribute[i]);
                    position[i] = s.toFloat();
                }
            }
        }
        if (child.tagName() == "orientation"){
            attribute << "q0" << "q1" << "q2" << "q3" ;
            Vector4f v;
            for (int i=0; i<attribute.size(); ++i){
                if (child.hasAttribute(attribute[i])) {
                    const QString s = child.attribute(attribute[i]);
                    v[i] = s.toFloat();
                }
            }
            orientation = Quaternionf(v);
            orientation.normalize();
        }
        child = child.nextSibling().toElement();
    }
}

Frame Frame::lerp(float alpha, const Frame& other) const
{
    return Frame((1.f-alpha)*position + alpha * other.position,
                 orientation.slerp(alpha,other.orientation));
}


Matrix4f Frame::getMatrix() const
{
    Matrix3f rotation = orientation.toRotationMatrix();
    Translation3f translation(position);
    Affine3f transformation = translation * rotation;
    return transformation.matrix();
}
