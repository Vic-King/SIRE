
#ifndef SIRE_BVH_H
#define SIRE_BVH_H

#include <Eigen/Geometry>
#include <vector>
#include "Ray.h"

class Mesh;

class BVH
{

    struct Node {
        Eigen::AlignedBox3f box;  ///< the axis aligned bounding box of the node
        union {
            int first_child_id; ///< for inner nodes only, index of the first child in mNodes, the second node is at mNodes[first_child_id+1]
            int first_face_id;  /// for leaves only, index of the first face in the face index list mFaces
        };
        unsigned short nb_faces;  /// for leaves only, number of faces in the leaf
        short is_leaf;            /// 1 or 0
    };

    typedef std::vector<Node> NodeList;

public:

    void build(const Mesh* pMesh, int targetCellSize, int maxDepth);

    bool intersect(const Ray& ray, Hit& hit) const;

protected:

    bool intersectNode(int nodeId, const Ray& ray, Hit& hit) const;

    int split(int start, int end, int dim, float split_value);

    void buildNode(int nodeId, int start, int end, int level, int targetCellSize, int maxDepth);

    /// Pointer to the original Mesh object
    const Mesh* mpMesh;

    /// The list of nodes
    NodeList mNodes;

    /// The sorted list of faces
    std::vector<int> mFaces;

    /// The centroids of the fase (use during the construction only)
    std::vector<Eigen::Vector3f> mCentroids;
};

#endif
