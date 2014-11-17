#include "Scene.h"
#include "Mesh.h"
#include "Sphere.h"
#include "Plane.h"
#include "DomUtils.h"

#include <Eigen/Geometry>

#include <QDomElement>
#include <QFile>

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
  Mesh* pMesh = 0;

  // create a sphere
  Sphere* pSphere1 = new Sphere(Eigen::Vector3f(0.3,-0.3,0.1), 0.1);
  // create an object
  pObj = new Object;
  pObj->attachShape(pSphere1);
  pObj->attachShader(&Program);
  // create a material
  BlinnPhong* ball_mat = new BlinnPhong(Array3f(0.3, 0.3, 0.8), Array3f(1, 1, 1), 256);
  pObj->setMaterial(ball_mat);
  mObjectList.push_back(pObj);

  // create a first mesh
  pMesh = new Mesh(SIRE_DIR"/data/quad.obj");
  pMesh->makeUnitary();
  pObj = new Object;
  pObj->attachShape(pMesh);
  pObj->setTransformation(Affine3f(Scaling(2.f)).matrix());
  pObj->attachShader(&Program);
  BlinnPhong* floor_mat = new BlinnPhong(Array3f(0.7, 0.7, 0.7), Array3f(1, 1, 1), 30);
  pObj->setMaterial(floor_mat);
  mObjectList.push_back(pObj);

  // create a second mesh
  pMesh = new Mesh(SIRE_DIR"/data/tw.off");
  pMesh->makeUnitary();
  pObj = new Object;
  pObj->attachShape(pMesh);
  pObj->attachShader(&Program);
  Affine3f M = Translation3f(0,0,0.5) * AngleAxisf(M_PI/4, Vector3f::UnitZ());
  pObj->setTransformation(M.matrix());
  BlinnPhong* tw_mat = new BlinnPhong(Array3f(0.8, 0.4, 0.4), Array3f(0, 0, 0), 0);
  pObj->setMaterial(tw_mat);
  mObjectList.push_back(pObj);

  // setup the light sources
  mLightList.push_back(new DirectionalLight(-Vector3f(1,1,1).normalized(), Array3f(0.9,0.9,0.9)));
  mLightList.push_back(new PointLight(Vector3f(2,-5,5), Array3f(1.9,1.9,1.9), 20));

  // setup the camera
  mCamera.setViewport(16,16);
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
  // récupérer Matrix mtransformation classe obj et en faire une 3x3

  Ray rayMod;

  std::cout << std::endl;

  for(int i=0; i<mObjectList.size(); ++i)
    {    
      Matrix4f objTransformation4x4 = mObjectList[i]->transformation();
      Matrix3f objTransformation3x3 = objTransformation4x4.block<3,3>(0,0);

      rayMod.origin = (objTransformation4x4.inverse() * Vector4f(ray.origin(0),ray.origin(1),ray.origin(2), 1.f)).head(3);
      rayMod.direction = objTransformation3x3.inverse() * ray.direction;
      
      mObjectList[i]->shape()->intersect(rayMod, hit);
    }
}

/// recursively trace a ray, \returns the light intensity (as a RGB color) received at the origin of the ray in the direction of the ray
Eigen::Array3f Scene::raytrace(const Ray& ray) const
{
  // TODO

  Hit hit;
  intersect(ray, hit);
  if(hit.foundIntersection())
    return Eigen::Array3f(1,0.2,0.2);

  return Eigen::Array3f(0.2, 0.2, 0.2);
}
