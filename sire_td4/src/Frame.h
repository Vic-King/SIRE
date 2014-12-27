#ifndef FRAME_H
#define FRAME_H

#include <Eigen/Geometry>
#include <QDomElement>

/// Represents a 3D frame, i.e. an orthogonal basis with the position of the origin.
class Frame
{
  public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    Frame(const Eigen::Vector3f& pos = Eigen::Vector3f::Zero(),
                 const Eigen::Quaternionf& o = Eigen::Quaternionf())
      : orientation(o), position(pos) {}

    Frame(const QDomElement& e);

    Frame lerp(float alpha, const Frame& other) const;

    Eigen::Matrix4f getMatrix() const;

    Eigen::Quaternionf orientation;
    Eigen::Vector3f position;
};

#endif // FRAME_H
