
#include "Sphere.h"
#include <QMessageBox>

#include <iostream>


//--------------------------------------------------------------------------------
// icosahedron data
//--------------------------------------------------------------------------------
#define X .525731112119133606
#define Z .850650808352039932

static float vdata[12][3] = {
    {-X, 0.0, Z}, {X, 0.0, Z}, {-X, 0.0, -Z}, {X, 0.0, -Z},
    {0.0, Z, X}, {0.0, Z, -X}, {0.0, -Z, X}, {0.0, -Z, -X},
    {Z, X, 0.0}, {-Z, X, 0.0}, {Z, -X, 0.0}, {-Z, -X, 0.0}
};

static int tindices[20][3] = {
    {0,4,1}, {0,9,4}, {9,5,4}, {4,5,8}, {4,8,1},
    {8,10,1}, {8,3,10}, {5,3,8}, {5,2,3}, {2,7,3},
    {7,10,3}, {7,6,10}, {7,11,6}, {11,0,6}, {0,1,6},
    {6,1,10}, {9,0,11}, {9,11,2}, {9,2,5}, {7,2,11} };
//--------------------------------------------------------------------------------

Sphere::Sphere(const Eigen::Vector3f& c, float r)
    : mCenter(c), mRadius(r)
{
    mpMesh = new Mesh;
    Eigen::Matrix<float,3,12> vertices((float*)vdata);
    vertices = (vertices*r).colwise() + c;
    mpMesh->loadRawData(vertices.data(), 12, (int*)tindices, 20);
}

Sphere::Sphere(const QDomElement& e)
    :mCenter(Eigen::Vector3f(0.0,0.0,0.0))
{
    if(e.hasAttribute("radius"))
        mRadius = e.attribute("radius").toFloat();
    else{
        QMessageBox::warning(NULL, "Object XML error", "Error while parsing Object XML document: radius attribute missing");
        return;
    }
    mpMesh = new Mesh;
    Eigen::Matrix<float,3,12> vertices((float*)vdata);
    vertices = (vertices*radius()).colwise() + center();
    mpMesh->loadRawData(vertices.data(), 12, (int*)tindices, 20);
}

Sphere::~Sphere()
{
    delete mpMesh;
}

void Sphere::drawGeometry(int prg_id) const
{
    mpMesh->drawGeometry(prg_id);
}

bool Sphere::intersect(const Ray& ray, Hit& hit) const
{
  float a = (ray.direction.normalized()).dot(ray.direction.normalized());
  float b = 2.0f * (ray.direction.normalized()).dot(ray.origin - mCenter);
  float c = (ray.origin - mCenter).dot(ray.origin - mCenter) - (mRadius * mRadius);

  float discriminant = b * b - 4 * a * c;

  if (discriminant > 0) {

    float s1 = (- b + sqrtf(discriminant)) / 2 * a;
    float s2 = (- b - sqrtf(discriminant)) / 2 * a;

    // Tester le chemin le plus court ?
    if (s1 > 0) {
      if (s1 < s2 && hit.t() > s1) {
        hit.setT(s1);
	hit.setIntersection(ray.direction * s1);
      }
    }

    if (s2 > 0) {
      if (s2 < s1 && hit.t() > s2) {
        hit.setT(s2);
	hit.setIntersection(ray.direction * s2);
      }
    }
  }
  else if (discriminant = 0) {
    float s = - b / 2 * a;

    if (hit.t() > s) {
      hit.setT(s);
      hit.setIntersection(ray.direction * s);
    }
  }
  else {
    return false;
  }

  return true;
}
