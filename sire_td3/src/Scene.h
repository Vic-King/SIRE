#ifndef SCENE_H
#define SCENE_H

#include "camera.h"
#include "Object.h"
#include "Light.h"

typedef std::vector<Object*> ObjectList;
typedef std::vector<Light*> LightList;

class Scene
{
public :
    Scene() : mBackgroundColor(0.6,0.6,0.6) {}
    void draw() const;
    void clear();
    void addObject(Object* o);
    void createDefaultScene(Shader &Program);
    void loadFromFile(const QString& filename);

    void setCamera(const Camera& camera) { mCamera = camera; }
    const Camera& camera() const { return mCamera; }
    Camera& camera() { return mCamera; }

    const ObjectList& objectList() { return mObjectList; }
    const LightList& lightList() { return mLightList; }

    const Eigen::Array3f& backgroundColor() { return mBackgroundColor; }

    /** recursively trace a ray, \returns the light intensity (as a RGB color) received at the origin of the ray in the direction of the ray **/
    Eigen::Array3f raytrace(const Ray& ray) const;
    /** Search for the nearest intersection between the ray and the object list */
    void intersect(const Ray& ray, Hit& hit) const;

protected:


  private:
    // Recall an object is the association of a shape, a shader, a texture ID, and a transformation (position, scale, orientation)
    ObjectList mObjectList;

    Camera mCamera;

    LightList mLightList;

    float nbLightWInAL, nbLightHInAL;

    Eigen::Array3f mBackgroundColor;

    Shader* mProgram;
};

#endif // SCENE_H
