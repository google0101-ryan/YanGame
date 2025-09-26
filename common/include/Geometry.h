#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <tier0/types.h>
#include <tier2/Texture.h>

class IGameObject;

typedef u16 Index_t;

struct DrawVert
{
    glm::vec3 pos;
    glm::vec2 uv;
    glm::vec3 normal;
    glm::vec3 color;
};

struct Geometry 
{
    int iNumVerts;
    DrawVert* pVerts;
    int iNumIndices;
    Index_t* pIndices;

    TexHandle_t albedoHandle;

    IGameObject* pParent;
};