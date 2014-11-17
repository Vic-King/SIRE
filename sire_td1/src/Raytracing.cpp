
#include "Raytracing.h"
#include "camera.h"
#include <Eigen/Geometry>

/** Render a scene using a raytracer.
  *
  */
QImage Raytracing::raytraceImage(const Scene &scene)
{
    using namespace Eigen;

    float tanfovy2 = tan(scene.camera().fovY()*0.5);

    Vector3f camX = scene.camera().right() * tanfovy2 * scene.camera().nearDist() * float(scene.camera().vpWidth())/float(scene.camera().vpHeight());
    Vector3f camY = scene.camera().up() * tanfovy2 * scene.camera().nearDist();
    Vector3f camF = scene.camera().direction() * scene.camera().nearDist();

    QImage img(scene.camera().vpWidth(), scene.camera().vpHeight(), QImage::Format_ARGB32);

    for(int j=0; j<scene.camera().vpHeight(); ++j) {
        for(int i=0; i<scene.camera().vpWidth(); ++i) {
            // compute the primary ray parameters
            Ray ray;
            ray.origin = scene.camera().position();
	    Vector3f direction = camF + (2 * (i / scene.camera().vpWidth() - 0.5)) * camX + (2 * (j / scene.camera().vpHeight() - 0.5)) * camY;
	    ray.direction.normalize();

            // raytrace the ray
            Eigen::Array3f color = scene.raytrace(ray);

	    std::cout << color << std::endl;

            // Basic tone mapping, and mapping from 0:1 to 0:255
            color = 255 * Array3f(fmin(color(0), 1.f), fmin(color(1), 1.f), fmin(color(2), 1.f));

            img.setPixel(i, j, qRgb(color(0), color(1), color(2)));
        }
    }

    return img;
}
