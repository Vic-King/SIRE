#ifndef PLANE_H
#define PLANE_H

#include "Shape.h"
#include "Mesh.h"

class Plane : public Shape
{
public:

    Plane();
    virtual ~Plane();

    virtual void drawGeometry(int prg_id) const;

    virtual const Eigen::AlignedBox3f& AABB() const;

    virtual bool intersect(const Ray& ray, Hit& hit) const;

protected:
    Mesh* mpMesh;
};

#endif // PLANE_H
