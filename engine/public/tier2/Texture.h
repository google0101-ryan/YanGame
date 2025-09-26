#pragma once

#include <tier0/types.h>

// Texture handles are actually indices into the bindless descriptor array
typedef u32 TexHandle_t;

namespace Texture
{

void InitTextures();
TexHandle_t CreateTexture(const char* path);

}