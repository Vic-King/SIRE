#include "Light.h"
#include "DomUtils.h"

Light::Light(const QDomElement& e)
{
    QDomNode n = e.firstChild();
    while (!n.isNull())
    {
        QDomElement e = n.toElement();
        if (!e.isNull())
        {
            if (e.tagName() == "Color")
                m_intensity = DomUtils::initColorFromDOMElement(e);
        }
        else
            QMessageBox::warning(NULL, "Light XML error", "Error while parsing Light XML document");
        n = n.nextSibling();
    }
}

DirectionalLight::DirectionalLight(const QDomElement& e)
    : Light(e)
{
    QDomNode n = e.firstChild();
    while (!n.isNull())
    {
        QDomElement e = n.toElement();
        if (!e.isNull())
        {
            if (e.tagName() == "Direction")
            {
                m_direction = DomUtils::initArrayFromDOMElement(e);
                m_direction.normalize();
            }
        }
        else
            QMessageBox::warning(NULL, "DirectionalLight XML error", "Error while parsing DirectionalLight XML document");
        n = n.nextSibling();
    }
}

PointLight::PointLight(const QDomElement& e)
    : Light(e)
{
    QDomNode n = e.firstChild();
    while (!n.isNull())
    {
        QDomElement e = n.toElement();
        if (!e.isNull())
        {
            if (e.tagName() == "Position"){
                m_position = DomUtils::initArrayFromDOMElement(e);
            }else if (e.tagName() == "Parameters"){
                m_radius = e.attribute("size", "0.0").toFloat();
            }
        }
        else
            QMessageBox::warning(NULL, "PointLight XML error", "Error while parsing PointLight XML document");
        n = n.nextSibling();
    }
}

