#ifndef LINE_H
#define LINE_H

#include "OpenGL.h"

#include <Eigen/Core>
#include <iostream>

class Point
{
public:
    static void draw(int prg_id, const Eigen::Vector3f& p)
    {
        unsigned int vertexBufferId;
        glGenBuffers(1,&vertexBufferId);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Eigen::Vector3f), p.data(), GL_STATIC_DRAW);
        GL_TEST_ERR;

        unsigned int vertexArrayId;
        glGenVertexArrays(1,&vertexArrayId);GL_TEST_ERR;

        // bind the vertex array
        glBindVertexArray(vertexArrayId);

        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);

        int vertex_loc = glGetAttribLocation(prg_id, "vtx_position");
        glVertexAttribPointer(vertex_loc, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(vertex_loc);GL_TEST_ERR;

        glPointSize(10.f);
        glDrawArrays(GL_POINTS,0,1);GL_TEST_ERR;

        glDisableVertexAttribArray(vertex_loc);
        glBindVertexArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDeleteBuffers(1, &vertexBufferId);
        glBindVertexArray(0);
        glDeleteVertexArrays(1, &vertexArrayId);
    }
};

class GLLine
{
public:
    static void draw(int prg_id, const Eigen::Vector3f& p1, const Eigen::Vector3f& p2)
    {
        Eigen::Vector3f mPoints[2] = {p1, p2};

        unsigned int vertexBufferId;
        glGenBuffers(1,&vertexBufferId);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Eigen::Vector3f)*2, mPoints[0].data(), GL_STATIC_DRAW);
        GL_TEST_ERR;

        unsigned int vertexArrayId;
        glGenVertexArrays(1,&vertexArrayId);GL_TEST_ERR;

        // bind the vertex array
        glBindVertexArray(vertexArrayId);

        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);

        int vertex_loc = glGetAttribLocation(prg_id, "vtx_position");
        glVertexAttribPointer(vertex_loc, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(vertex_loc);GL_TEST_ERR;

        glDrawArrays(GL_LINES,0,2);GL_TEST_ERR;

        glDisableVertexAttribArray(vertex_loc);
        glBindVertexArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDeleteBuffers(1, &vertexBufferId);
        glBindVertexArray(0);
        glDeleteVertexArrays(1, &vertexArrayId);
    }
};

#endif // LINE_H
