#pragma once

#include <any>
#include <functional>
#include <type_traits>

#include "cdif.h"

namespace cdif {
    class Registration
    {
        private:
            std::function<std::any(const Container&)> m_resolver;

        public:
            template <typename T>
            Registration(const std::function<T (const Container&)>& resolver)
            {
                m_resolver = [resolver] (const Container&) { return resolver; };
            }

            virtual ~Registration() = default;

            Registration(const Registration&) = default;
            Registration& operator=(const Registration&) = default;
            Registration(Registration&&) = default;
            Registration& operator=(Registration&&) = default;

            template <typename T>
            T resolve(const cdif::Container& ctx) const
            {
                auto resolver = std::any_cast<std::function<T (const Container&)>>(m_resolver(ctx));
                return resolver(ctx);
            }
    };
}
