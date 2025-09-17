#pragma once

#include <glm/glm.hpp>
#include <tier0/types.h>

typedef u16 Index_t;

struct DrawVert
{
    glm::vec3 pos;
    glm::vec3 color;
};

struct Geometry
{
    int iNumVerts;
    DrawVert* pVerts;
    int iNumIndices;
    Index_t* pIndices;
};