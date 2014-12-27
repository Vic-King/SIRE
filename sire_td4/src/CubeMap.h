
#ifndef SIRE_CUBEMAP_H
#define SIRE_CUBEMAP_H

#include "rgbe.h"

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <QMessageBox>
#include <QDomElement>
#include <iostream>

class CubeMap
{
public:
    bool load(const QString& filename);

    Eigen::Array3f intensity(const Eigen::Vector3f& dir) const;

protected:
    Eigen::Array3f readTexture(int startX, int startY, float u, float v, int sizeU, int sizeV) const;
    float* getPixel(int x, int y) const;

private:
    float* m_image;
    int m_sizeX, m_sizeY;
};

#endif // SIRE_CUBEMAP_H
