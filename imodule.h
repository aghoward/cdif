#pragma once

#include "context.h"

class IModule {
    public:
        virtual void Load(Context & ctx) = 0;
};

