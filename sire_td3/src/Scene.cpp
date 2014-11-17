#include "Scene.h"
#include "Mesh.h"
#include "Sphere.h"
#include "Plane.h"
#include "DomUtils.h"
#include "AreaLight.h"

#include <Eigen/Geometry>

#include <QDomElement>
#include <QFile>
#include <QColor>

using namespace Eigen;

void Scene::draw() const
{
    for(int i=0; i<mObjectList.size(); ++i)
    {
        mObjectList[i]->draw();
    }
}

void Scene::clear()
{
    mObjectList.clear();
    mLightList.clear();
}

void Scene::createDefaultScene(Shader &Program)
{
    Object* pObj = 0;

    nbLightWInAL = 4.f;
    nbLightHInAL = 4.f;

    // create a sphere
    Sphere* pSphere1 = new Sphere(Eigen::Vector3f(0.0,0.0,0.0),0.1);
    // create an object
    pObj = new Object;
    pObj->attachShape(pSphere1);
    Affine3f M = Affine3f(Translation3f(0.6,-0.6,0.1));
    pObj->setTransformation(M.matrix());
    pObj->attachShader(&Program);
    // create a material
    BlinnPhong* ball_mat = new BlinnPhong(Array3f(0.3, 0.3, 0.8), Array3f(1, 1, 1), 256);
    pObj->setMaterial(ball_mat);
    addObject(pObj);

    Sphere* pSphere2 = new Sphere(Eigen::Vector3f(0.0,0.0,0.0), 0.2);
    pObj = new Object;
    pObj->attachShape(pSphere2);
    M = Affine3f(Translation3f(-0.8,-0.8,0.2));
    pObj->setTransformation(M.matrix());
    pObj->attachShader(&Program);
    BlinnPhong* ball_mat2 = new BlinnPhong(Array3f(0.3, 0.8, 0.3), Array3f(0.04, 0.04, 0.04), 10);
    pObj->setMaterial(ball_mat2);
    addObject(pObj);

    // create an infinite plane
    Plane* pPlane = new Plane();
    pObj = new Object;
    pObj->attachShape(pPlane);
    pObj->setTransformation(Affine3f::Identity().matrix());
    pObj->attachShader(&Program);
    BlinnPhong* floor_mat = new BlinnPhong(Array3f(0.7, 0.7, 0.7), Array3f(1, 1, 1), 30);
    pObj->setMaterial(floor_mat);
    addObject(pObj);

    // create a mesh
    Mesh* pMesh = new Mesh(SIRE_DIR"/data/tw.off"); //less faces: tw503.off
    pMesh->makeUnitary();
    // uncomment to activate the creation of the BVH
    pMesh->buildBVH();
    pObj = new Object;
    pObj->attachShape(pMesh);
    pObj->attachShader(&Program);
    M = Translation3f(0,0,0.5) * AngleAxisf(M_PI/4, Vector3f::UnitZ());
    pObj->setTransformation(M.matrix());
    BlinnPhong* tw_mat = new BlinnPhong(Array3f(0.8, 0.4, 0.4), Array3f(0, 0, 0), 0);
    pObj->setMaterial(tw_mat);
    addObject(pObj);

    // setup the light sources
    mLightList.push_back(new DirectionalLight(-Vector3f(1,1,1).normalized(), Array3f(0.6,0.6,0.6)));
    mLightList.push_back(new PointLight(Vector3f(2,-5,5), Array3f(0.8,0.8,0.8), 20));

    mLightList.push_back(new AreaLight(Vector3f(-4,2,7), -Vector3f(-4,2,7).normalized(), 0.5, Array3f(1.9,1.9,1.9), 20, "light_source_color.png"));

    // setup the camera
    mCamera.setViewport(512,512);
    mCamera.setFovY(M_PI/2.);
    mCamera.lookAt(Vector3f(1.2, -1.2, 1.2), Vector3f(0, 0, 0.1), Vector3f::UnitZ());

    // set background color
    mBackgroundColor = Array3f(0.2,0.2,0.2);

    mProgram = &Program;
}

void Scene::addObject(Object* o)
{
    mObjectList.push_back(o);
}

void Scene::loadFromFile(const QString& filename)
{
    clear();

    QDomDocument doc(filename);
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
        return;
    if (!doc.setContent(&file))
    {
        file.close();
        return;
    }
    file.close();

    QDomElement docElem = doc.documentElement();

    mBackgroundColor = DomUtils::initColorFromDOMElement(docElem);

    QDomNode n = docElem.firstChild();
    while( !n.isNull() ) {
        QDomElement e = n.toElement();
        if( !e.isNull() ) {
            if (e.tagName() == "Sphere")
            {
                Sphere* pSphere = new Sphere(e);
                Object* pObj = new Object(e);
                pObj->attachShape(pSphere);
                pObj->attachShader(mProgram);
                addObject(pObj);
            }else if(e.tagName() == "Plane"){
                Plane* pPlane = new Plane();
                Object* pObj = new Object(e);
                pObj->attachShape(pPlane);
                pObj->attachShader(mProgram);
                addObject(pObj);
            }else if(e.tagName() == "Camera"){
                mCamera.initFromDomElement(e);
            }else if(e.tagName()=="DirectionalLight"){
                DirectionalLight* l = new DirectionalLight(e);
                mLightList.push_back(l);
            }else if(e.tagName()=="PointLight"){
                PointLight* l = new PointLight(e);
                mLightList.push_back(l);
            }else if(e.tagName()=="BackgroundColor"){
                mBackgroundColor = DomUtils::initColorFromDOMElement(e);
            }else{
                qWarning("Unsupported node : %s",qPrintable(e.tagName()));
            }
        }
        n = n.nextSibling();
    }
}

/** Search for the nearest intersection between the ray and the object list */
void Scene::intersect(const Ray& ray, Hit& hit) const
{
    for(int i=0; i<mObjectList.size(); ++i)
    {
        Ray local_ray;
        local_ray = ray;
        // apply transformation
        Eigen::Affine3f M(mObjectList[i]->transformation());
        Eigen::Affine3f invM = M.inverse();
        local_ray.origin = invM * ray.origin;
        local_ray.direction = invM.linear() * ray.direction;
        float old_t = hit.t();
        if(hit.foundIntersection())
        {
            // If previous intersection found, transform intersection point
            Eigen::Vector3f x = ray.at(hit.t());
            hit.setT( (invM * x - local_ray.origin).norm() );
        }
        Hit h;
        mObjectList[i]->shape()->intersect(local_ray, h);

        if(h.t()<hit.t())
        {
            // we found a new closest intersection point for this object, record it:
            hit.setObject(mObjectList[i]);
            Eigen::Vector3f x = local_ray.at(h.t());
            hit.setNormal( (invM.linear().transpose() * h.normal()).normalized() );
            hit.setT( (M * x - ray.origin).norm() );
        }else{
            hit.setT(old_t);
        }
    }
}

/// recursively trace a ray, \returns the light intensity (as a RGB color) received at the origin of the ray in the direction of the ray
Eigen::Array3f Scene::raytrace(const Ray& ray) const
{
    using namespace Eigen;
    Array3f value(0,0,0);

    // stopping criteria:
    if(ray.recursionLevel>=8 || ray.beta<0.1)
    {
        return value;
    }

    // find the first intersection point
    Hit hit;
    intersect(ray, hit);
    if(hit.foundIntersection())
    {
        Vector3f rayHit = ray.at(hit.t());

        // add ambient
        value += 0.1 * hit.object()->material()->ambientColor();

        float dist;

        for(int i = 0; i < mLightList.size(); ++i) {

            if (dynamic_cast<const AreaLight*> (mLightList[i])) {
                const AreaLight* light = dynamic_cast<const AreaLight*>(mLightList[i]);

                Array3f valueTmp(0,0,0);

                const float lightSize = light->size();

                Vector3f lightDir;

		QRgb color;

		float halfLightSize = lightSize / 2.f;
		float ratioLight = 100.f / lightSize + 12.5f;

		float ratioTexW = light->texture().width() / 100.f;
		float ratioTexH = light->texture().height() / 100.f;

                for (float x = -halfLightSize; x < halfLightSize; x += (float) (lightSize / nbLightWInAL)) {
                    for (float y = -halfLightSize; y < halfLightSize; y += (float) (lightSize / nbLightHInAL)) {

                        Vector3f pos = light->position() + x * light->uVec() + y * light->vVec();

                        lightDir = (pos - rayHit).normalized();

                        Ray ray(rayHit + hit.normal() * 1e-4, lightDir);
                        ray.shadowRay = true;

                        Hit shadow_hit;

                        intersect(ray, shadow_hit);

                        dist = (pos - rayHit).norm();

                        if(shadow_hit.t() < dist)
                            continue;

			if (!light->texture().isNull()) {
			
			  float ratioLightW = (x + halfLightSize) * ratioLight;
			  float ratioLightH = (y + halfLightSize) * ratioLight;
		   
			  color = light->texture().pixel(ratioTexW * ratioLightW, ratioTexH * ratioLightH);
			  const Array3f colorf (QColor(color).red() / 255.f, QColor(color).green() / 255.f, QColor(color).blue() / 255.f);
	
			  float cos_term = std::max(0.f, lightDir.dot(hit.normal()));
			  valueTmp += cos_term * light->intensity(rayHit, pos) * hit.object()->material()->brdf(-ray.direction, lightDir, hit.normal()) * colorf;
			}
			else {

			  float cos_term = std::max(0.f, lightDir.dot(hit.normal()));
			  valueTmp += cos_term * light->intensity(rayHit, pos) *  hit.object()->material()->brdf(-ray.direction, lightDir, hit.normal());
			}
		    }
                }

                value += valueTmp / (nbLightWInAL * nbLightHInAL);
            }
            else{

                Vector3f lightDir = mLightList[i]->direction(rayHit, &dist);
                Ray shadow_ray(rayHit+hit.normal()*1e-4, lightDir);
                shadow_ray.shadowRay = true;
                Hit shadow_hit;
                intersect(shadow_ray, shadow_hit);
                if(shadow_hit.t() < dist)
                    continue;

                float cos_term = std::max(0.f,lightDir.dot(hit.normal()));
                value += cos_term * mLightList[i]->intensity(rayHit) * hit.object()->material()->brdf(-ray.direction, lightDir, hit.normal());
            }
        }

        // reflexions
        {
            // compute the reflexion factor alpha
            Array3f alpha = hit.object()->material()->brdfReflect(-ray.direction, hit.normal());
            Vector3f r = (ray.direction - 2.*ray.direction.dot(hit.normal())*hit.normal()).normalized();
            Ray reflexion_ray(rayHit+hit.normal()*1e-4, r);
            reflexion_ray.recursionLevel = ray.recursionLevel + 1;
            reflexion_ray.beta = ray.beta * alpha.matrix().norm();
            value += alpha * raytrace(reflexion_ray);
        }

        // To display the normal
        // Array3f n = hit.normal();
        // value = Array3f(fabs(n[0]),fabs(n[1]),fabs(n[2]));
    }

    else if(ray.recursionLevel == 0) {
        value = mBackgroundColor;
    }

    return value;
}
