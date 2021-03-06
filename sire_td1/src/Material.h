#ifndef SIRE_MATERIAL_H
#define SIRE_MATERIAL_H

#include <Eigen/Core>
#include <QDomElement>
#include <QImage>

class Material
{
public:

    virtual Eigen::Array3f ambientColor() const = 0;

    /// evaluate the BRDF
    virtual Eigen::Array3f brdf(const Eigen::Vector3f& viewDir, const Eigen::Vector3f& lightDir, const Eigen::Vector3f& normal) const = 0;

    /// evaluate the BRDF in the reflected direction
    virtual Eigen::Array3f brdfReflect(const Eigen::Vector3f& viewDir, const Eigen::Vector3f& normal) const = 0;

    /// texture
    enum TextureMode { MODULATE, BLEND, REPLACE };

    float textureScaleU() const { return m_textureScaleU; }
    float textureScaleV() const { return m_textureScaleV; }
    TextureMode textureMode() const { return m_textureMode; }
    const QImage& texture() const { return m_texture; }

    void setTexture(const QImage& texture) { m_texture = texture; }
    void loadTextureFromFile(const QString& fileName);
    void setTextureScale(float textureScale) { setTextureScaleU(textureScale); setTextureScaleV(textureScale); }
    void setTextureScaleU(float textureScaleU) { if (fabs(textureScaleU) > 1e-3) m_textureScaleU = textureScaleU; }
    void setTextureScaleV(float textureScaleV) { if (fabs(textureScaleV) > 1e-3) m_textureScaleV = textureScaleV; }
    void setTextureMode(TextureMode textureMode) { m_textureMode = textureMode; }

private:
    TextureMode m_textureMode;
    QImage m_texture;
    float m_textureScaleU, m_textureScaleV;
};


class BlinnPhong : public Material
{
public:

    BlinnPhong(const Eigen::Array3f& diffuseColor, const Eigen::Array3f& specularColor, float exponent)
        : m_diffuseColor(diffuseColor), m_specularColor(specularColor), m_exponent(exponent)
    {}

    BlinnPhong(const QDomElement &e);

    Eigen::Array3f ambientColor() const
    {
        return m_diffuseColor;
    }

    Eigen::Array3f brdf(const Eigen::Vector3f& viewDir, const Eigen::Vector3f& lightDir, const Eigen::Vector3f& normal) const
    {
        // TODO
        return Eigen::Array3f();
    }

    Eigen::Array3f brdfReflect(const Eigen::Vector3f& /*viewDir*/, const Eigen::Vector3f& /*normal*/) const
    {
        // TODO
        return Eigen::Array3f();
    }

protected:
    Eigen::Array3f m_diffuseColor;
    Eigen::Array3f m_specularColor;
    float m_exponent;
};

#endif // SIRE_MATERIAL_H
