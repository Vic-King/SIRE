#include "Plane.h"
#include <QMessageBox>

//--------------------------------------------------------------------------------
// icosahedron data
//--------------------------------------------------------------------------------
#define X 2.0

static float vdata[4][3] = {
    {X, X, 0.0}, {-X, X, 0.0}, {-X, -X, 0.0}, {X, -X, 0.0}
};

static int tindices[2][3] = {
    {0,1,3}, {3,1,2}
};
//--------------------------------------------------------------------------------

Plane::Plane()
{
    mpMesh = new Mesh;
    Eigen::Matrix<float,3,4> vertices((float*)vdata);
    mpMesh->loadRawData(vertices.data(), 4, (int*)tindices, 2);
}

Plane::~Plane()
{
    delete mpMesh;
}

void Plane::drawGeometry(int prg_id) const
{
    mpMesh->drawGeometry(prg_id);
}

const Eigen::AlignedBox3f& Plane::AABB() const
{
    return mpMesh->AABB();
}

bool Plane::intersect(const Ray& ray, Hit& hit) const
{
    return false;
}
