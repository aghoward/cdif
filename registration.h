#pragma once

#include <any>
#include <functional>

#include "cdif.h"

namespace cdif {
    class Registration {
        private:
            std::function<std::any(const cdif::Container &)> m_resolver;

        public:
            Registration(std::function<std::any (const cdif::Container&)> resolver)
                : m_resolver(resolver) {};

            virtual ~Registration() = default;

            Registration(const Registration &) = default;
            Registration & operator=(const Registration &) = default;
            Registration(Registration &&) = default;
            Registration & operator=(Registration &&) = default;


            template <typename T>
            T Resolve(const cdif::Container & ctx) const {
                return std::any_cast<T>(m_resolver(ctx));
            }
    };
}
