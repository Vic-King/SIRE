
#ifndef SIRE_BVH_H
#define SIRE_BVH_H

#include <Eigen/Geometry>
#include <vector>
#include "Ray.h"
class Mesh;

class BVH
{
  
  struct Node {
    Eigen::AlignedBox3f box;
    union {
      int first_child_id; // for inner nodes
      int first_face_id;  // for leaves
    };
    unsigned short nb_faces;
    short is_leaf;
  };
  
  typedef std::vector<Node> NodeList;
  
public:
  
  void build(const Mesh* pMesh, int targetCellSize, int maxDepth);
  bool intersect(const Ray& ray, Hit& hit) const;
  
  
  
protected:
  
  bool intersectNode(int nodeId, float tMin, float tMax, const Ray& ray, Hit& hit) const;
  
  int split(int start, int end, int dim, float split_value);
  
  void buildNode(int nodeId, int start, int end, int level, int targetCellSize, int maxDepth);
  
  const Mesh* mpMesh;
  NodeList mNodes;
  std::vector<int> mFaces;
  std::vector<Eigen::Vector3f> mCentroids;
  
};

#endif
