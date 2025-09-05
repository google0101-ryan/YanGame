#pragma once

#include <tier0/types.h>

abstract_class IApplication
{
public:
    virtual bool Init() = 0;
    virtual void Tick() = 0;
    virtual void Shutdown() = 0;

    virtual bool IsRunning() = 0;
};