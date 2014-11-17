
#ifndef SIRE_SPHERE_H
#define SIRE_SPHERE_H

#include <Eigen/Core>
#include <QDomElement>
#include "Shape.h"
#include "Mesh.h"

/** represents a sphere
 */
class Sphere : public Shape
{
public:

    Sphere(const Eigen::Vector3f& c, float r);
    Sphere(const QDomElement& e);
    virtual ~Sphere();

    virtual void drawGeometry(int prg_id) const;

    // TODO add an intersection by a ray method
    virtual bool intersect(const Ray& ray, Hit& hit) const;

    float radius() const { return mRadius; }
    const Eigen::Vector3f& center() const { return mCenter; }

protected:
    Eigen::Vector3f mCenter;
    float mRadius;
    Mesh* mpMesh;
};

#endif
