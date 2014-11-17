#include "Material.h"
#include "DomUtils.h"

#include <QMessageBox>

BlinnPhong::BlinnPhong(const QDomElement &e)
    : m_diffuseColor(0.f,0.f,0.f), m_specularColor(0.f,0.f,0.f), m_exponent(1.)
{
    if (e.tagName() != "Material")
    {
        QMessageBox::critical(NULL, "Material init error", "Material::initFromDOMElement, bad DOM tagName.\nExpecting 'Material', got "+e.tagName());
        return;
    }

    QDomNode n = e.firstChild();
    while (!n.isNull())
    {
        QDomElement e = n.toElement();
        if (!e.isNull())
        {
            if (e.tagName() == "DiffuseColor")
                m_diffuseColor = DomUtils::initColorFromDOMElement(e);
            else if (e.tagName() == "SpecularColor")
            {
                m_specularColor = DomUtils::initColorFromDOMElement(e);
                if (!e.hasAttribute("exponent"))
                    QMessageBox::warning(NULL, "Material error", "No Material specular coefficient provided. Using 0.0 instead");
                m_exponent = e.attribute("exponent", "0.0").toFloat();
            }
            else if (e.tagName() == "Texture")
            {
                setTextureScale(e.attribute("scale", "1.0").toFloat());
                if (e.hasAttribute("scaleU"))
                    setTextureScaleU(e.attribute("scaleU", "1.0").toFloat());
                if (e.hasAttribute("scaleV"))
                    setTextureScaleV(e.attribute("scaleV", "1.0").toFloat());
                loadTextureFromFile(e.attribute("file"));
                if (e.attribute("mode") == "MODULATE") setTextureMode(MODULATE);
                if (e.attribute("mode") == "BLEND")    setTextureMode(BLEND);
                if (e.attribute("mode") == "REPLACE")  setTextureMode(REPLACE);
            }
            else
                qWarning("Material child error -- Unsupported Material child : %s",qPrintable(e.tagName()));
        }
        else
            QMessageBox::warning(NULL, "Material XML error", "Error while parsing Material XML document");

        n = n.nextSibling();
    }
}

void Material::loadTextureFromFile(const QString& fileName)
{
    if (fileName.isNull())
        QMessageBox::warning(NULL, "Material texture error", "Material error : no texture file name provided");
    else
        if (!m_texture.load(SIRE_DIR"/data/" + fileName))
            QMessageBox::warning(NULL, "Material texture error", "Unable to load Material texture from "+fileName);
}
