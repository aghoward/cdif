#pragma once

#include <any>
#include <functional>
#include <type_traits>

#include "cdif.h"

namespace cdif {
    class Registration {
        private:
            std::function<std::any(const cdif::Container &)> m_resolver;

        public:
            Registration(std::function<std::any (const cdif::Container &)> resolver)
                : m_resolver(resolver) {};

            template <typename T>
            Registration(std::function<T (const cdif::Container &)> resolver)
                requires !std::is_copy_constructible<T>::value && std::is_move_constructible<T>::value
            {
                m_resolver = [resolver] (const cdif::Container &) { return resolver; };
            }

            virtual ~Registration() = default;

            Registration(const Registration &) = default;
            Registration & operator=(const Registration &) = default;
            Registration(Registration &&) = default;
            Registration & operator=(Registration &&) = default;


            template <typename T>
            T Resolve(const cdif::Container & ctx) const {
                return std::any_cast<T>(m_resolver(ctx));
            }

            template <typename T>
            T Resolve(const cdif::Container & ctx) const
                requires !std::is_copy_constructible<T>::value && std::is_move_constructible<T>::value
            {
                auto resolver = std::any_cast<std::function<T (const cdif::Container &)>>(m_resolver(ctx));
                return std::move(resolver(ctx));
            }
    };
}
