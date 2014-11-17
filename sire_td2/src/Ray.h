#ifndef SIRE_RAY
#define SIRE_RAY

#include <Eigen/Geometry>

class Object;

class Ray
{
public:
    Ray(const Eigen::Vector3f& o, const Eigen::Vector3f& d)
        : origin(o), direction(d), beta(1.), recursionLevel(0), shadowRay(false)
    {}
    Ray() : beta(1.), recursionLevel(0), shadowRay(false) {}

    Eigen::Vector3f origin;
    Eigen::Vector3f direction;

    Eigen::Vector3f at(float t) const { return origin + t*direction; }

    float beta;           ///< contribution percentage to the final value (used as a stoping critera)
    int recursionLevel;   ///< recursion level (used as a stoping critera)
    bool shadowRay;       ///< tag for shadow rays
};

class Hit
{
private:
    Eigen::Vector3f m_intersection;
    Eigen::Vector3f m_normal;
    Eigen::Vector2f m_texcoord;
    const Object* mp_object;
    float m_t;

public:
    Hit()
        : m_texcoord(0,0), mp_object(0), m_t(std::numeric_limits<float>::max())
    {}
    bool foundIntersection() const { return m_t < std::numeric_limits<float>::max(); }

    void setT(float t) { m_t = t; }
    float t() const { return m_t; }

    void setIntersection(const Eigen::Vector3f& i) { m_intersection = i; }
    const Eigen::Vector3f& intersection() { return m_intersection; }

    void setObject(const Object* obj) { mp_object = obj; }
    const Object* object() const { return mp_object; }

    void setNormal(const Eigen::Vector3f& n) { m_normal = n; }
    const Eigen::Vector3f& normal() const { return m_normal; }

    void setTexcoord(const Eigen::Vector2f& uv) { m_texcoord = uv; }
    const Eigen::Vector2f& texcoord() const { return m_texcoord; }
};

/** Compute the intersection between a ray and an aligned box
  * \returns true if an intersection is found
  * The ranges are returned in tMin,tMax
  */
static inline bool intersect(const Ray& ray, const Eigen::AlignedBox3f& box, float& tMin, float& tMax)
{
    Eigen::Array3f t1, t2;
    t1 = (box.min()-ray.origin).cwiseQuotient(ray.direction);
    t2 = (box.max()-ray.origin).cwiseQuotient(ray.direction);
    tMin = t1.min(t2).maxCoeff();
    tMax = t1.max(t2).minCoeff();
    return tMax>0 && tMin<=tMax;
}

#endif
