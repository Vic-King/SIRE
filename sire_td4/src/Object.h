#ifndef SIRE_OBJECT_H
#define SIRE_OBJECT_H

#include "Shape.h"
#include "Shader.h"
#include "Material.h"
#include <Eigen/Core>
#include <QDomElement>

class Object
{
    static BlinnPhong ms_defaultMaterial;
public:
    Object();
    Object(const QDomElement& e);
    void attachShape(const Shape* shape);
    void attachShader(const Shader* shader);

    void setTransformation(const Eigen::Matrix4f& mat);
    const Eigen::Matrix4f& transformation() const { return mTransformation; }

    const Shape* shape() const { return mShape; }
    void draw();

    const Material* material() const { return mMaterial; }
    void setMaterial(const Material* mat) { mMaterial = mat; }

protected:
    const Shader* mShader;
    const Shape*  mShape;
    const Material* mMaterial;
    Eigen::Matrix4f mTransformation;
};

#endif
