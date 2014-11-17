#ifndef SIRE_RAY
#define SIRE_RAY

#include <Eigen/Geometry>

class Object;

class Ray
{
public:
    Ray(const Eigen::Vector3f& o, const Eigen::Vector3f& d)
        : origin(o), direction(d) {}

    Ray() {}

    Eigen::Vector3f origin;
    Eigen::Vector3f direction;

    Eigen::Vector3f at(float t) const { return origin + t*direction; }
};

class Hit
{
private:
    Eigen::Vector3f m_intersection;
    const Object* mp_object;
    float m_t;

public:
    Hit() : mp_object(0), m_t(std::numeric_limits<float>::max()) {}

    bool foundIntersection() const { return m_t < std::numeric_limits<float>::max(); }

    void setT(float t) { m_t = t; }
    float t() const { return m_t; }
    void reset() { m_t = std::numeric_limits<float>::max(); }
    void setIntersection(const Eigen::Vector3f& i) { m_intersection = i; }
    const Eigen::Vector3f& intersection() { return m_intersection; }

    void setObject(const Object* obj) { mp_object = obj; }
    const Object* object() const { return mp_object; }
};

/** Compute the intersection between a ray and an aligned box
  * \returns true if an intersection is found
  * The ranges are returned in tMin,tMax
  */
static inline bool intersect(const Ray& ray, const Eigen::AlignedBox3f& box, float& tMin, float& tMax)
{
    // TODO
    return false;
}

#endif
