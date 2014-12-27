#include "CubeMap.h"

bool CubeMap::load(const QString& filename)
{
    if (filename.endsWith(".hdr"))
    {
        FILE* f = fopen(filename.toStdString().c_str(), "rb");

        // Read image header
        if(RGBE_ReadHeader(f, &m_sizeX, &m_sizeY, 0)!=0){
            return false;
        }

        m_image = new float[sizeof(float)*3*m_sizeX*m_sizeY];
        // Read image data
        if(RGBE_ReadPixels_RLE(f, reinterpret_cast<float*>(m_image), m_sizeX, m_sizeY)!=0){
            return false;
        }
        return true;
    }

    QImage image;
    if (image.load(filename)){
        for(int x = 0; x < m_sizeX; ++x)
            for(int y = 0; y < m_sizeY; ++y){
                QRgb c = image.pixel(x,y);
                m_image[ 3 * ( x + m_sizeX * y )    ] = qRed(c)/255.f;
                m_image[ 3 * ( x + m_sizeX * y ) + 1] = qGreen(c)/255.f;
                m_image[ 3 * ( x + m_sizeX * y ) + 2] = qBlue(c)/255.f;
            }
        return true;
    }
    qWarning("Could not open: %s", qPrintable(filename));
    return false;
}

float* CubeMap::getPixel(int x, int y) const
{
    return &m_image[(x + y*m_sizeX) * 3];
}

Eigen::Array3f CubeMap::intensity(const Eigen::Vector3f& dir) const
{
    assert(m_image.isInitialized());

    // cross is 3 faces wide, 4 faces high
    int face_width  = m_sizeX / 3;
    int face_height = m_sizeY / 4;

    Eigen::Array3f outputColor;

    Eigen::Array3f tdir = Eigen::Affine3f(Eigen::AngleAxisf(M_PI/2.,Eigen::Vector3f::UnitX())) * dir;

    if ((fabsf(tdir[0]) >= fabsf(tdir[1]))
            && (fabsf(tdir[0]) >= fabsf(tdir[2])))
    {
        if (tdir[0] > 0.0f)
        {
            // right
            outputColor = readTexture(2*face_width, face_height,
                                      1.0f - (tdir[2] / tdir[0]+ 1.0f) * 0.5f,
                    (tdir[1] / tdir[0]+ 1.0f) * 0.5f, face_width, face_height);
        }
        else if (tdir[0] < 0.0f)
        {
            // left
            outputColor = readTexture(0, face_height,
                                      1.0f - (tdir[2] / tdir[0]+ 1.0f) * 0.5f,
                    1.0f - ( tdir[1] / tdir[0] + 1.0f) * 0.5f,
                    face_width, face_height);
        }
    }
    else if ((fabsf(tdir[1]) >= fabsf(tdir[0])) && (fabsf(tdir[1]) >= fabsf(tdir[2])))
    {
        if (tdir[1] > 0.0f)
        {
            // bottom
            outputColor = readTexture(face_width, 2*face_height,
                                      (tdir[0] / tdir[1] + 1.0f) * 0.5f,
                    1.0f - (tdir[2]/ tdir[1] + 1.0f) * 0.5f, face_width, face_height);
        }
        else if (tdir[1] < 0.0f)
        {
            // top
            outputColor = readTexture(face_width, 0,
                                      1.0f - (tdir[0] / tdir[1] + 1.0f) * 0.5f,
                    1.0f - (tdir[2]/tdir[1] + 1.0f) * 0.5f, face_width, face_height);
        }
    }
    else if ((fabsf(tdir[2]) >= fabsf(tdir[0]))
             && (fabsf(tdir[2]) >= fabsf(tdir[1])))
    {
        if (tdir[2] > 0.0f)
        {
            // Front
            outputColor = readTexture(face_width, face_height,
                                      (tdir[0] / tdir[2] + 1.0f) * 0.5f,
                    (tdir[1]/tdir[2] + 1.0f) * 0.5f, face_width, face_height);
        }
        else if (tdir[2] < 0.0f)
        {
            // Back
            outputColor = readTexture(face_width, 3*face_height,
                                      1.0f - (tdir[0] / tdir[2] + 1.0f) * 0.5f,
                    (tdir[1] /tdir[2]+1) * 0.5f, face_width, face_height);
        }
    }
    return outputColor;
}

Eigen::Array3f CubeMap::readTexture(int startX, int startY, float u, float v, int sizeU, int sizeV) const
{
    u = fabsf(u);
    v = fabsf(v);
    int umin = int(sizeU * u);
    int vmin = int(sizeV * v);
    int umax = int(sizeU * u) + 1;
    int vmax = int(sizeV * v) + 1;
    float ucoef = fabsf(sizeU * u - umin);
    float vcoef = fabsf(sizeV * v - vmin);

    // Clamping
    umin = fmin(fmax(umin, 0), sizeU - 1);
    umax = fmin(fmax(umax, 0), sizeU - 1);
    vmin = fmin(fmax(vmin, 0), sizeV - 1);
    vmax = fmin(fmax(vmax, 0), sizeV - 1);

    // Bilinear interpolation along u and v
    const float* t1 = getPixel(startX+umin,startY+vmin);
    const float* t2 = getPixel(startX+umax,startY+vmin);
    const float* t3 = getPixel(startX+umin,startY+vmax);
    const float* t4 = getPixel(startX+umax,startY+vmax);
    Eigen::Array3f output =
            (1.0f - vcoef) * ((1.0f - ucoef) * Eigen::Array3f(*t1,*(t1+1),*(t1+2))
                              + ucoef * Eigen::Array3f(*t2,*(t2+1),*(t2+2)))
            + vcoef * ((1.0f - ucoef) * Eigen::Array3f(*t3,*(t3+1),*(t3+2))
                       + ucoef * Eigen::Array3f(*t4,*(t4+1),*(t4+2)));
    return output;
}
