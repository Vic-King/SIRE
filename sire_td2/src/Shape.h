
#ifndef SIRE_SHAPE_H
#define SIRE_SHAPE_H

#include "Ray.h"

/** represents a shape, i.e., its geometry
 */
class Shape
{
public:
    /** draw the geometry of the shape using OpenGL.
    * It must be implemented in the derived class.
    * \param prg_id id of the GLSL program used to draw the geometry */
    virtual void drawGeometry(int prg_id) const = 0;

    virtual const Eigen::AlignedBox3f& AABB() const = 0;

    virtual bool intersect(const Ray& ray, Hit& hit) const = 0;
};

#endif
