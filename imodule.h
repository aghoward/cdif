#pragma once

#include "cdif.h"

namespace cdif {
    class IModule {
        public:
            virtual void load(cdif::Container & ctx) = 0;
            virtual ~IModule() = default;
    };
}
