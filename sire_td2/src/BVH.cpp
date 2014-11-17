#include "BVH.h"
#include "Mesh.h"
#include "GLPrimitives.h"
#include <iostream>

/** Build the BVH from the mesh \a pMesh.
 *  \param targetCellSize maximal number of faces per leaf
 *  \param maxDepth maximal depth of the tree
 */
void BVH::build(const Mesh* pMesh, int targetCellSize, int maxDepth)
{
    // store a pointer to the mesh
    mpMesh = pMesh;

    // allocate the root node
    mNodes.resize(1);

    // reserve space for other nodes to avoid multiple memory reallocations
    mNodes.reserve( std::min<int>(2 << maxDepth, std::log(mpMesh->nbFaces() / targetCellSize)));

    // compute face centroids and initialize the face list
    mCentroids.resize(mpMesh->nbFaces());

    mFaces.resize(mpMesh->nbFaces());

    for(int i = 0; i < mpMesh->nbFaces(); ++i)
    {
        mCentroids[i] = (mpMesh->vertexOfFace(i, 0).position + mpMesh->vertexOfFace(i, 1).position + mpMesh->vertexOfFace(i, 2).position)/3.f;
        mFaces[i] = i;
    }

    // recursively build the BVH, starting from the root node and the entire list of faces
    buildNode(0, 0, mpMesh->nbFaces(), 0, targetCellSize, maxDepth);
}

/** Computes the intersection with the underlying mesh using the BHV.
 * \returns true is a nearest intersection has been found
 */
bool BVH::intersect(const Ray& ray, Hit& hit) const
{
    // compute the intersection with the root node
    // vérifier si on a bien une intersection (en fonction de tMin, tMax, et hit.t()), et si oui appeler intersecNode...

    float tMin, tMax;
     if (::intersect(ray, mNodes[0].box, tMin, tMax)) {
         intersectNode(0, ray, hit);

         return true;
     }

    return false;
}

/** Computes the intersection with the \a nodeId -th node.
 */
bool BVH::intersectNode(int nodeId, const Ray& ray, Hit& hit) const
{
    Node node = mNodes[nodeId];

    // TODO, deux cas: soit mNodes[nodeId] est une feuille (il faut alors intersecter les triangles du noeud),
    if (node.is_leaf == 1) {
        for (int i = node.first_face_id; i < node.first_face_id + node.nb_faces; ++i) {
            if (mpMesh->intersectFace(ray, hit, i)) {
                std::cout << "Intersection with " << nodeId << std::endl;
                return true;
            }
        }
    }
    // soit c'est un noeud interne (il faut visiter les fils (ou pas))
    else {
        return (intersectNode(node.first_child_id, ray, hit) || intersectNode(node.first_child_id + 1, ray, hit));
    }

    return false;
}

/** Sorts the faces with respect to their centroid along the dimension \a dim and spliting value \a split_value.
  * \returns the middle index called mid_id
  * When this function returns, the centroid of faces in the range start
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

/** Recursively builds the BVH
 * \param nodeId the index in mNodes of the node which has to be built
 * \param start the first index of the faces that have to be considered
 * \param end the last+1 index of the faces that have to be considered
 * \param level the current recursion level
 * \param targetCellSize stopping criteria, see build()
 * \param maxDepth stopping criteria, see build()
 * This function must do the following:
 * - compute the bounding box of the \a nodeId -th node
 * - check whether it's a leaf or an inner node
 * - if it's a leaf, find a splitting plane and sort the faces accordingly
 * - allocate the children and recursively build them by calling this function.
 */
void BVH::buildNode(int nodeId, int start, int end, int level, int targetCellSize, int maxDepth)
{
    Node& node = mNodes[nodeId];
    node.box.setNull();


    // étape 1 : calculer la boite englobante des faces indexées de mFaces[start] à mFaces[end]
    // (Utiliser la fonction extend de Eigen::AlignedBox3f et la fonction mpMesh->vertexOfFace(int) pour obtenir les coordonnées des sommets des faces)

    for (int i = start; i < end; ++i) {

        Eigen::Vector3f vx = mpMesh->vertexOfFace(mFaces[i], 0).position;
        Eigen::Vector3f vy = mpMesh->vertexOfFace(mFaces[i], 1).position;
        Eigen::Vector3f vz = mpMesh->vertexOfFace(mFaces[i], 2).position;

        node.box.extend(vx);
        node.box.extend(vy);
        node.box.extend(vz);
    }

    // étape 2 : déterminer si il s'agit d'une feuille (appliquer les critères d'arrêts)

    // Si c'est une feuille, finaliser le noeud et stop3

    if (end - start <= targetCellSize || level == maxDepth) {
        node.is_leaf = 1;
        node.first_face_id = start;
        node.nb_faces = end - start;
    }

    else {
        // Si c'est un noeud interne:

        node.is_leaf = 0;
        node.first_child_id = mNodes.size();
        node.nb_faces = 0;

        // étape 3 : calculer la dimension (0, 1, ou 2) et valeur du plan de coupe
        // (on découpe au milieu de la boite selon la plus grande dimension)

        Eigen::Vector3f dim3D = node.box.sizes();

        int dim = dim3D[0] > dim3D[1] && dim3D[0] > dim3D[2] ? 0 : dim3D[1] > dim3D[2] ? 1 : 2;
        float split_value = dim == 0 ? dim3D[0] / 2.0f + node.box.min()[0] : dim == 1 ? dim3D[1] / 2.0f + node.box.min()[1] : dim3D[2] / 2.0f + node.box.min()[2];

        // étape 4 : appeler la fonction split pour trier (partiellement) les faces

        int ind = split(start, end, dim, split_value);

        // étape 5 : allouer les fils, et les construire en appelant buildNode...

        mNodes.resize(mNodes.size() + 2);

        buildNode(node.first_child_id, start, ind, level + 1, targetCellSize, maxDepth);
        buildNode(node.first_child_id+1, ind, end, level + 1, targetCellSize, maxDepth);
    }
}
