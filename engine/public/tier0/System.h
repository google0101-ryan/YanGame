#pragma once

#include <tier0/types.h>

abstract_class ISystem
{
public:
    virtual bool Init() = 0;
    virtual void Shutdown() = 0;
};