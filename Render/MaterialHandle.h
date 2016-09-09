// -*- mode: c++; tab-width: 4; c-basic-offset: 4; -*-

#pragma once

#include "Render/Asset.h"
#include <stdint.h>

struct Material;

struct MaterialHandle : AssetHandle
{
    static MaterialHandle Invalid();

    // resolve
    Material* Resolve();
    const Material* Resolve() const;
    
    MaterialHandle IncrementRefCount();
    void DecrementRefCount();
};

// crc
uint32_t Djb(MaterialHandle materialHandle);
