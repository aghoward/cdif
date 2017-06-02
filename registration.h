#pragma once

#include <any>
#include <functional>

#include "context.h"

class Registration {
    private:
        std::function<std::any(const Context &)> _resolver;

    public:
        Registration(std::function<std::any (const Context&)> resolver)
            : _resolver(resolver) {};

        template <typename T>
        T Resolve(const Context & ctx) const {
            return std::any_cast<T>(_resolver(ctx));
        }
};


