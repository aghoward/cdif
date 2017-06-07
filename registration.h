#pragma once

#include <any>
#include <functional>

#include "container.h"

class Registration {
    private:
        std::function<std::any(const Container &)> _resolver;

    public:
        Registration(std::function<std::any (const Container&)> resolver)
            : _resolver(resolver) {};

        template <typename T>
        T Resolve(const Container & ctx) const {
            return std::any_cast<T>(_resolver(ctx));
        }
};


