#ifndef DOMUTILS_H
#define DOMUTILS_H

#include <QDomElement>
#include <Eigen/Core>

class DomUtils
{
public:
    static Eigen::Array3f initColorFromDOMElement(const QDomElement& e)
    {
        float r = e.attribute("red",   "0.0").toFloat();
        float g = e.attribute("green", "0.0").toFloat();
        float b = e.attribute("blue",  "0.0").toFloat();
        return Eigen::Array3f(r,g,b);
    }

    static Eigen::Array3f initArrayFromDOMElement(const QDomElement& e)
    {
        float x = e.attribute("x", "0.0").toFloat();
        float y = e.attribute("y", "0.0").toFloat();
        float z = e.attribute("z", "0.0").toFloat();
        return Eigen::Array3f(x,y,z);
    }
};

#endif // DOMUTILS_H
