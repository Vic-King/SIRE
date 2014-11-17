
#ifndef SIRE_RAYTRACING_H
#define SIRE_RAYTRACING_H

#include "Scene.h"
#include <QImage>

class Raytracing
{
public:
    static QImage raytraceImage(const Scene& scene);
};

#endif // SIRE_RAYTRACING_H
