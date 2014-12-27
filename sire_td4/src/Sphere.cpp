
#include "Sphere.h"
#include <QMessageBox>
#include <deque>

//--------------------------------------------------------------------------------
//  data
//--------------------------------------------------------------------------------
#define X .525731112119133606
#define Z .850650808352039932

static float vdata[12][3] = {
    {-X, 0.0, Z},{X, 0.0, Z}, {-X, 0.0, -Z}, {X, 0.0, -Z},
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

    /*int nU = 10;
    int nV = 10;
    int nVertices  = (nU + 1) * (nV + 1);
    int nTriangles =  nU * nV * 2;
    std::vector<Eigen::Vector3f> vertices;
    vertices.resize(nVertices);
    for(int v=0;v<=nV;++v) {
        for(int u=0;u<=nU;++u) {
            float theta = u / float(nU) * M_PI;
            float phi 	= v / float(nV) * M_PI * 2;
            int index 	= u +(nU+1)*v;
            // position
            Eigen::Vector3f vertex = Eigen::Vector3f(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta)).normalized() * mRadius + mCenter;
            vertices[index] = vertex;
        }
    }*/


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

const Eigen::AlignedBox3f& Sphere::AABB() const
{
    return mpMesh->AABB();
}

bool Sphere::intersect(const Ray& ray, Hit& hit) const
{
    Eigen::Vector3f diff = ray.origin - mCenter;
    float b = 2.*ray.direction.dot(diff);
    float c = (diff).squaredNorm() - mRadius*mRadius;
    float discr = b*b - 4.*c;
    if(discr>=0)
    {
        discr = std::sqrt(discr);
        float t = 0.5*(-b - discr);
        if(t<1e-4)
            t = 0.5*(-b + discr);
        if(t<1e-4 || t>hit.t())
            return false;

        hit.setT(t);
        hit.setNormal((ray.at(t) - mCenter).normalized());

        return true;
    }
    return false;
}
