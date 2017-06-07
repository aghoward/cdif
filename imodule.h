#pragma once

#include "container.h"

namespace cdif {
    class IModule {
        public:
            virtual void Load(cdif::Container & ctx) = 0;
    };
}
