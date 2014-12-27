// Copyright (C) 2008-2011 Gael Guennebaud <gael.guennebaud@inria.fr>


#include "Mesh.h"
#include "Shader.h"

#include <iostream>
#include <fstream>
#include <limits>

#include <QCoreApplication>
#include <Eigen/Geometry>
#include <ObjFormat/ObjFormat.h>
#include <lib3ds/mesh.h>
#include <lib3ds/file.h>
#include "BVH.h"

using namespace Eigen;

Mesh::Mesh(const std::string& filename)
    : mIsInitialized(false), mBVH(0)
{
    std::string ext = filename.substr(filename.size()-3,3);
    if(ext=="off" || ext=="OFF")
        loadOFF(filename);
    else if(ext=="obj" || ext=="OBJ")
        loadOBJ(filename);
    else if(ext=="3ds" || ext=="3DS")
        load3DS(filename);
    else
        std::cerr << "Mesh: extension \'" << ext << "\' not supported." << std::endl;
}

void Mesh::loadOFF(const std::string& filename)
{
    std::ifstream in(filename.c_str(),std::ios::in);
    if(!in)
    {
        std::cerr << "File not found " << filename << std::endl;
        return;
    }

    std::string header;
    in >> header;

    // check the header file
    if(header != "OFF")
    {
        std::cerr << "Wrong header = " << header << std::endl;
        return;
    }

    int nofVertices, nofFaces, inull;
    int nb, id0, id1, id2;
    Vector3 v;

    in >> nofVertices >> nofFaces >> inull;

    for(int i=0 ; i<nofVertices ; ++i)
    {
        in >> v.x() >> v.y() >> v.z();
        mVertices.push_back(v);
    }

    for(int i=0 ; i<nofFaces ; ++i)
    {
        in >> nb >> id0 >> id1 >> id2;
        assert(nb==3);
        mFaces.push_back(FaceIndex(id0, id1, id2));
    }

    in.close();

    computeNormals();
    computeAABB();
}

void Mesh::loadOBJ(const std::string& filename)
{
    ObjMesh* pRawObjMesh = ObjMesh::LoadFromFile(filename);

    if (!pRawObjMesh)
    {
        std::cerr << "Mesh::loadObj: error loading file " << filename << "." << std::endl;
        return;
    }

    // Makes sure we have an indexed face set
    ObjMesh* pObjMesh = pRawObjMesh->createIndexedFaceSet(Obj::Options(Obj::AllAttribs|Obj::Triangulate));
    delete pRawObjMesh;
    pRawObjMesh = 0;

    // copy vertices
    mVertices.resize(pObjMesh->positions.size());

    for (int i=0 ; i<pObjMesh->positions.size() ; ++i)
    {
        mVertices[i] = Vertex(Vector3f(pObjMesh->positions[i]));

        if(!pObjMesh->texcoords.empty())
            mVertices[i].texcoord = Vector2f(pObjMesh->texcoords[i]);

        if(!pObjMesh->normals.empty())
            mVertices[i].normal = Vector3f(pObjMesh->normals[i]);
    }

    // copy faces
    for (int smi=0 ; smi<pObjMesh->getNofSubMeshes() ; ++smi)
    {
        const ObjSubMesh* pSrcSubMesh = pObjMesh->getSubMesh(smi);

        mFaces.reserve(pSrcSubMesh->getNofFaces());

        for (uint fid = 0 ; fid<pSrcSubMesh->getNofFaces() ; ++fid)
        {
            ObjConstFaceHandle srcFace = pSrcSubMesh->getFace(fid);
            mFaces.push_back(Vector3i(srcFace.vPositionId(0), srcFace.vPositionId(1), srcFace.vPositionId(2)));
        }
    }

    if(pObjMesh->normals.empty())
    {
        std::cout << "compute normals\n";
        computeNormals();
    }
}

void Mesh::load3DS(const std::string& filename)
{
    Lib3dsFile* pFile = lib3ds_file_load(filename.c_str());

    if(pFile==0)
    {
        std::cerr << "file not find !" << std::endl;
        return;
    }

    lib3ds_file_eval(pFile,0);

    /*
      1 objet 3DS = ensemble de sous-objets
      1 sous-objet = ensemble de sommets et de faces (triangles)
      1 triangle = liste de 3 indices
  */

    // Parcours de tous les sous-objets

    /* pointeur sur le sous-objet courrant */
    Lib3dsMesh* pMesh = NULL;

    /* pFile->meshes == pointeur sur le premier sous-objet */
    for(pMesh = pFile->meshes ; pMesh!=NULL ; pMesh = pMesh->next)
    {
        // pMesh->name == nom du sous-objet courrant
        uint i;
        uint offset_id = mVertices.size();

        // Parcours de tous les points du sous-objet
        for (i = 0; i < pMesh->points; i++)
        {
            /* position du sommet i */
            mVertices.push_back(Vector3(pMesh->pointL[i].pos));

            // si il y a autant de coordonées de texture que de sommets ...
            if (pMesh->texels == pMesh->points)
            {
                // ... alors les coordonnées de texture sont disponibles :
                mVertices.back().texcoord[0] = pMesh->texelL[i][0]; // i = numéro du sommet
                mVertices.back().texcoord[1] = pMesh->texelL[i][1];
            }
        }

        // Parcours de toutes les faces du sous-objet
        for (i = 0; i < pMesh->faces; i++)
        {
            mFaces.push_back(FaceIndex(
                                 offset_id + pMesh->faceL[i].points[0],
                             offset_id + pMesh->faceL[i].points[1],
                    offset_id + pMesh->faceL[i].points[2]));
        }
    }

    computeNormals();
    computeAABB();
}

void Mesh::loadRawData(float* positions, int nbVertices, int* indices, int nbTriangles)
{
    mVertices.resize(nbVertices);
    for(int i=0; i<nbVertices; ++i)
        mVertices[i].position = Eigen::Vector3f::Map(positions+3*i);
    mFaces.resize(nbTriangles);
    for(int i=0; i<nbTriangles; ++i)
        mFaces[i] = Eigen::Vector3i::Map(indices+3*i);

    computeNormals();
    computeAABB();
}

Mesh::~Mesh()
{
    if(mIsInitialized)
    {
        glDeleteBuffers(1,&mVertexBufferId);
        glDeleteBuffers(1,&mIndexBufferId);
    }
    delete mBVH;
}

void Mesh::makeUnitary()
{
    // computes the lowest and highest coordinates of the axis aligned bounding box,
    // which are equal to the lowest and highest coordinates of the vertex positions.
    Eigen::Vector3f lowest, highest;
    lowest.fill(std::numeric_limits<float>::max());   // "fill" sets all the coefficients of the vector to the same value
    highest.fill(-std::numeric_limits<float>::max());

    for(VertexArray::iterator v_iter = mVertices.begin() ; v_iter!=mVertices.end() ; ++v_iter)
    {
        // - v_iter is an iterator over the elements of mVertices,
        //   an iterator behaves likes a pointer, it has to be dereferenced (*v_iter, or v_iter->) to access the referenced element.
        // - Here the .aray().min(_) and .array().max(_) operators work per component.
        //
        lowest  = lowest.array().min(v_iter->position.array());
        highest = highest.array().max(v_iter->position.array());
    }

    // TODO: appliquer une transformation à tous les sommets de mVertices de telle sorte
    // que la boite englobante de l'objet soit centrée en (0,0,0)  et que sa plus grande dimension soit de 1
    Eigen::Vector3f center = (lowest+highest)/2.0;
    float m = (highest-lowest).maxCoeff();
    for(VertexArray::iterator v_iter = mVertices.begin() ; v_iter!=mVertices.end() ; ++v_iter)
        v_iter->position = (v_iter->position - center) / m;

    computeAABB();
}

void Mesh::computeNormals()
{
    // pass 1: set the normal to 0
    for(VertexArray::iterator v_iter = mVertices.begin() ; v_iter!=mVertices.end() ; ++v_iter)
        v_iter->normal.setZero();

    // pass 2: compute face normals and accumulate
    for(FaceIndexArray::iterator f_iter = mFaces.begin() ; f_iter!=mFaces.end() ; ++f_iter)
    {
        Vector3f v0 = mVertices[(*f_iter)(0)].position;
        Vector3f v1 = mVertices[(*f_iter)(1)].position;
        Vector3f v2 = mVertices[(*f_iter)(2)].position;

        Vector3f n = (v1-v0).cross(v2-v0).normalized();

        mVertices[(*f_iter)(0)].normal += n;
        mVertices[(*f_iter)(1)].normal += n;
        mVertices[(*f_iter)(2)].normal += n;
    }

    // pass 3: normalize
    for(VertexArray::iterator v_iter = mVertices.begin() ; v_iter!=mVertices.end() ; ++v_iter)
        v_iter->normal.normalize();
}

void Mesh::computeAABB()
{
    mAABB.setNull();
    for(VertexArray::iterator v_iter = mVertices.begin() ; v_iter!=mVertices.end() ; ++v_iter)
        mAABB.extend(v_iter->position);
}

void Mesh::buildBVH()
{
    delete mBVH;
    mBVH = new BVH;
    mBVH->build(this, 10, 100);
}

void Mesh::drawGeometry(int prg_id) const
{
    if(!mIsInitialized)
    {
        mIsInitialized = true;
        // this is the first call to drawGeometry
        // => create the BufferObjects and copy the related data into them.
        glGenBuffers(1,&mVertexBufferId);
        glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferId);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*mVertices.size(), mVertices[0].position.data(), GL_STATIC_DRAW);

        glGenBuffers(1,&mIndexBufferId);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBufferId);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(FaceIndex)*mFaces.size(), mFaces[0].data(), GL_STATIC_DRAW);

        glGenVertexArrays(1,&mVertexArrayId);GL_TEST_ERR;
    }

    // bind the vertex array
    glBindVertexArray(mVertexArrayId);

    glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBufferId);

    int vertex_loc   = glGetAttribLocation(prg_id, "vtx_position");
    int normal_loc   = glGetAttribLocation(prg_id, "vtx_normal");
    int texcoord_loc = glGetAttribLocation(prg_id, "vtx_texcoord");

    // specify the vertex data
    if(vertex_loc>=0)
    {
        glVertexAttribPointer(vertex_loc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(vertex_loc);
    }

    if(normal_loc>=0)
    {
        glVertexAttribPointer(normal_loc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(Vector3f));
        glEnableVertexAttribArray(normal_loc);
    }

    if(texcoord_loc>=0)
    {
        glVertexAttribPointer(texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(2*sizeof(Vector3f)));GL_TEST_ERR;
        glEnableVertexAttribArray(texcoord_loc);GL_TEST_ERR;
    }

    // send the geometry
    glDrawElements(GL_TRIANGLES, 3*mFaces.size(), GL_UNSIGNED_INT, (void*)0);GL_TEST_ERR;

    if(texcoord_loc>=0) glDisableVertexAttribArray(texcoord_loc);
    if(normal_loc>=0)   glDisableVertexAttribArray(normal_loc);
    if(vertex_loc>=0)   glDisableVertexAttribArray(vertex_loc);

    // release the vertex array
    glBindVertexArray(0);GL_TEST_ERR;
}

long int Mesh::ms_itersection_count = 0;

bool Mesh::intersectFace(const Ray& ray, Hit& hit, int faceId) const
{
    ms_itersection_count++;
    Vector3f v0 = mVertices[mFaces[faceId](0)].position;
    Vector3f v1 = mVertices[mFaces[faceId](1)].position;
    Vector3f v2 = mVertices[mFaces[faceId](2)].position;
    Vector3f e1 = v1 - v0;
    Vector3f e2 = v2 - v0;
    Matrix3f M;
    M << -ray.direction, e1, e2;
    Vector3f tuv = M.inverse() * (ray.origin - v0);
    float t = tuv(0), u = tuv(1), v = tuv(2);
    if(t>0 && u>=0 && v>=0 && (u+v)<=1 && t<hit.t())
    {
        hit.setT(t);

        if(ray.shadowRay)
            return true;

        Vector3f n0 = mVertices[mFaces[faceId](0)].normal;
        Vector3f n1 = mVertices[mFaces[faceId](1)].normal;
        Vector3f n2 = mVertices[mFaces[faceId](2)].normal;
        hit.setNormal(u*n1 + v*n2 + (1.-u-v)*n0);

        Vector2f tc0 = mVertices[mFaces[faceId](0)].texcoord;
        Vector2f tc1 = mVertices[mFaces[faceId](1)].texcoord;
        Vector2f tc2 = mVertices[mFaces[faceId](2)].texcoord;
        hit.setTexcoord(u*tc1 + v*tc2 + (1.-u-v)*tc0);

        return true;
    }
    return false;
}

bool Mesh::intersect(const Ray& ray, Hit& hit) const
{
    if(mBVH)
    {
        // use the BVH !!
        return mBVH->intersect(ray, hit);
    }
    else
    {
        // brute force !!
        bool ret = false;
        float tMin, tMax;
        if( (!::intersect(ray, mAABB, tMin, tMax)) || tMin>hit.t())
            return false;
        for(int i=0; i<mFaces.size(); ++i)
        {
            ret = ret | intersectFace(ray, hit, i);
        }
        return ret;
    }
}


