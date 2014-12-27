#include "Light.h"

class AreaLight : public PointLight
{
public:
    AreaLight(const Eigen::Vector3f& pos, const Eigen::Vector3f& dir, float a_size, const Eigen::Array3f& a_intensity, float radius)
        : PointLight(pos, a_intensity, radius), m_size(a_size)
    {
        m_frame.col(2) = -dir;
        m_frame.col(0) = m_frame.col(2).unitOrthogonal();
        m_frame.col(1) = m_frame.col(2).cross(m_frame.col(0));
    }

    // average intensity (to consider it as a point light)
    virtual Eigen::Array3f intensity(const Eigen::Vector3f& x) const
    {
        return std::max(0.f,(x-m_position).normalized().dot(direction())) * PointLight::intensity(x);
    }

    Eigen::Array3f intensity(const Eigen::Vector3f& hit, const Eigen::Vector3f& pos) const {

        return std::max(0.f,(hit - pos).normalized().dot(direction())) * PointLight::intensity(hit);
    }

    void loadTexture(QString filename) {
        m_texture.load(filename);
    }

    //------------------------------------------------------------
    // Frame setters and getters
    /// sets the position of the camera
    void setPosition(const Eigen::Vector3f& pos) { m_position = pos; }
    /// \returns the position of the camera
    inline const Eigen::Vector3f& position() const { return m_position; }
    /// sets the orientation of the light
    void setOrientation(const Eigen::Quaternionf& q) { m_frame = q.toRotationMatrix();  }
    /// \returns the light direction, i.e., the -z axis of the frame
    Eigen::Vector3f direction() const { return -m_frame.col(2); }
    /// \returns the first tangent axis of the light plane
    Eigen::Vector3f uVec() const { return m_frame.col(1); }
    /// \returns the second tangent axis of the light plane
    Eigen::Vector3f vVec() const { return m_frame.col(0); }
    //------------------------------------------------------------

    float size() const { return m_size; }

protected:
    Eigen::Matrix3f m_frame;
    float m_size;
    QImage m_texture;
};
