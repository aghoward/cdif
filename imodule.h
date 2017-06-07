#pragma once

#include "container.h"

class IModule {
    public:
        virtual void Load(Container & ctx) = 0;
};

