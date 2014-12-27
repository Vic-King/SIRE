
#include "Raytracing.h"
#include "camera.h"

#include <Eigen/Geometry>
#include <QProgressDialog>

/** Render a scene using a raytracer.
  *
  */
QImage Raytracing::raytraceImage(const Scene &scene)
{
    QProgressDialog progress("Raytracing...", "Cancel", 0, scene.camera().vpWidth() * scene.camera().vpHeight());
    progress.setWindowModality(Qt::WindowModal);

    using namespace Eigen;
    float tanfovy2 = tan(scene.camera().fovY()*0.5);
    Vector3f camX = scene.camera().right() * tanfovy2 * scene.camera().nearDist() * float(scene.camera().vpWidth())/float(scene.camera().vpHeight());
    Vector3f camY = scene.camera().up() * tanfovy2 * scene.camera().nearDist();
    Vector3f camF = scene.camera().direction() * scene.camera().nearDist();
    QImage img(scene.camera().vpWidth(), scene.camera().vpHeight(), QImage::Format_ARGB32);
    for(int j=0; j<scene.camera().vpHeight(); ++j)
        for(int i=0; i<scene.camera().vpWidth(); ++i)
        {
            progress.setValue(j*scene.camera().vpHeight() + i);
            if (progress.wasCanceled())
                return img;

            // compute the primary ray parameters
            Ray ray;
            ray.origin = scene.camera().position();
            scene.camera().direction();
            ray.direction = (camF + camX * (2.0*float(i+0.5)/float(scene.camera().vpWidth()) - 1.) - camY * (2.0*float(j+0.5)/float(scene.camera().vpHeight()) - 1.0)).normalized();

            // raytrace the ray
            Eigen::Array3f color = scene.raytrace(ray);

            // Basic tone mapping, and mapping from 0:1 to 0:255
            color /= (color + 0.25);
            color = 255*Array3f(fmin(color(0),1.f),fmin(color(1),1.f),fmin(color(2),1.f));
	    
            //color = Array3f(255, 0, 0);

            img.setPixel(i, j, qRgb(color(0), color(1), color(2)));
        }
    return img;
}
