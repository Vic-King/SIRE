
#ifndef SIRE_LIGHT_H
#define SIRE_LIGHT_H

#include <Eigen/Core>
#include <QMessageBox>
#include <QDomElement>

class Light
{
public:
    /** returns the direction (unit vector) from x to the light,
    * optionnaly, returns the distance to the light in *dist
    */
    virtual Eigen::Vector3f direction(const Eigen::Vector3f& x, float* dist = 0) const = 0;
    /// \returns the intensity emitted at x
    virtual Eigen::Array3f intensity(const Eigen::Vector3f& x) const = 0;

    Light(const Eigen::Array3f& a_intensity) : m_intensity(a_intensity) {}

    Light(const QDomElement& e);

protected:
    Eigen::Array3f m_intensity;
};

class DirectionalLight : public Light
{
public:
    DirectionalLight(const QDomElement& e);

    DirectionalLight(const Eigen::Vector3f& d, const Eigen::Array3f& a_intensity)
        : Light(a_intensity), m_direction(d) {}

    virtual Eigen::Vector3f direction(const Eigen::Vector3f& /*x*/, float* dist = 0) const
    {
        if(dist)
            *dist = std::numeric_limits<float>::max();
        return -m_direction;
    }
    virtual Eigen::Array3f intensity(const Eigen::Vector3f& x) const { return m_intensity; }
protected:
    Eigen::Vector3f m_direction;
};

class PointLight : public Light
{
public:
    PointLight(const QDomElement& e);

    /// \a radius is the influence radius
    PointLight(const Eigen::Vector3f& p, const Eigen::Array3f& a_intensity, float radius)
        : Light(a_intensity), m_position(p), m_radius(radius) {}

    virtual Eigen::Vector3f direction(const Eigen::Vector3f& x, float* dist = 0) const
    {
        // TODO
        return Eigen::Vector3f();
    }

    virtual Eigen::Array3f intensity(const Eigen::Vector3f& x) const
    {
        // TODO
        return Eigen::Array3f();
    }

protected:
    Eigen::Vector3f m_position;
    float m_radius;
};

typedef std::vector<Light*> LightList;

#endif // SIRE_LIGHT_H
