
#include "BVH.h"
#include "Mesh.h"
#include <iostream>

void BVH::build(const Mesh* pMesh, int targetCellSize, int maxDepth)
{
    mpMesh = pMesh;
    mNodes.resize(1);
    mNodes.reserve( std::min<int>(2<<maxDepth, std::log(mpMesh->nbFaces()/targetCellSize) ) );
    // compute centroids and initialize the face list
    mCentroids.resize(mpMesh->nbFaces());
    mFaces.resize(mpMesh->nbFaces());
    for(int i=0; i<mpMesh->nbFaces(); ++i)
    {
        mCentroids[i] = (mpMesh->vertexOfFace(i, 0).position + mpMesh->vertexOfFace(i, 1).position + mpMesh->vertexOfFace(i, 2).position)/3.f;
        mFaces[i] = i;
    }

    buildNode(0, 0, mpMesh->nbFaces(), 0, targetCellSize, maxDepth);
}

bool BVH::intersect(const Ray& ray, Hit& hit) const
{
    float tMin, tMax;
    ::intersect(ray, mNodes[0].box, tMin, tMax);
    if(tMax>0 && tMax>=tMin && tMin<hit.t())
        return intersectNode(0, tMin, tMax, ray, hit);
    return false;
}

bool BVH::intersectNode(int nodeId, float tMin, float tMax, const Ray& ray, Hit& hit) const
{
    if(std::isinf(tMin) || std::isinf(tMax))
        return false;

    const Node& node = mNodes[nodeId];

    if(node.is_leaf)
    {
        int end = node.first_child_id+node.nb_faces;
        for(int i=node.first_child_id; i<end; ++i)
        {
            mpMesh->intersectFace(ray, hit, mFaces[i]);
        }
    }
    else
    {
        float tMin1, tMax1, tMin2, tMax2;
        int child_id1 = node.first_child_id;
        int child_id2 = node.first_child_id+1;
        ::intersect(ray, mNodes[child_id1].box, tMin1, tMax1);
        ::intersect(ray, mNodes[child_id2].box, tMin2, tMax2);
        if(tMin1 > tMin2)
        {
            std::swap(tMin1, tMin2);
            std::swap(tMax1, tMax2);
            std::swap(child_id1, child_id2);
        }   

        if(tMin1 < hit.t() && tMin1<=tMax1 && tMax1>0)
        {
            intersectNode(child_id1, tMin1, tMax1, ray, hit);
        }
        if(tMin2 < hit.t() && tMin2<=tMax2 && tMax2>0)
        {
            intersectNode(child_id2, tMin2, tMax2, ray, hit);
        }
    }
}

/** Sorts the faces with respect to their centroid along the dimension \a dim and spliting value \a split_value.
  * \returns the middle index
  */
int BVH::split(int start, int end, int dim, float split_value)
{
    int l(start), r(end-1);
    while(l<r)
    {
        // find the first on the left
        while(l<end && mCentroids[l](dim) < split_value) ++l;
        while(r>=start && mCentroids[r](dim) >= split_value) --r;
        if(l>r) break;
        std::swap(mCentroids[l], mCentroids[r]);
        std::swap(mFaces[l], mFaces[r]);
        ++l;
        --r;
    }
    return mCentroids[l][dim]<split_value ? l+1 : l;
}

void BVH::buildNode(int nodeId, int start, int end, int level, int targetCellSize, int maxDepth)
{
    Node& node = mNodes[nodeId];

    // compute bounding box
    Eigen::AlignedBox3f aabb;
    aabb.setNull();
    for(int i=start; i<end; ++i)
    {
        aabb.extend(mpMesh->vertexOfFace(mFaces[i], 0).position);
        aabb.extend(mpMesh->vertexOfFace(mFaces[i], 1).position);
        aabb.extend(mpMesh->vertexOfFace(mFaces[i], 2).position);
    }
    node.box = aabb;

    // stopping criteria
    if(end-start <= targetCellSize || level>=maxDepth)
    {
        // we got a leaf !
        node.is_leaf = true;
        node.first_face_id = start;
        node.nb_faces = end-start;
        return;
    }
    node.is_leaf = false;

    // Split along the largest dimension
    Eigen::Vector3f diag = aabb.max() - aabb.min();
    int dim;
    diag.maxCoeff(&dim);
    // Split at the middle
    float split_value = 0.5 * (aabb.max()[dim] + aabb.min()[dim]);

    // Sort the faces according to the split plane
    int mid_id = split(start, end, dim, split_value);

    // second stopping criteria
    if(mid_id==start || mid_id==end)
    {
        // no improvement
        node.is_leaf = true;
        node.first_face_id = start;
        node.nb_faces = end-start;
        return;
    }

    // create the children
    int child_id = node.first_child_id = mNodes.size();
    mNodes.resize(mNodes.size()+2);
    // node is not a valid reference anymore !

    buildNode(child_id  , start, mid_id, level+1, targetCellSize, maxDepth);
    buildNode(child_id+1, mid_id, end, level+1, targetCellSize, maxDepth);
}
