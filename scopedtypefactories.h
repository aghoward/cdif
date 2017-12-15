#pragma once

#include <functional>
#include <memory>
#include <type_traits>
#include <utility>

#include "type_traits.h"

namespace cdif
{
    namespace detail {
        template <
            typename T,
            typename TBase = typename get_base_type<T>::type,
            typename TFactory = std::function<TBase (const Container&)>>
        TBase& createSingleton(const TFactory& factory, const Container& ctx) 
        {
            static TBase instance = factory(ctx);
            return instance;
        }

        template <
            typename T,
            typename TBase = typename get_base_type<T>::type,
            typename TFactory = std::function<TBase (const Container&)>>
        TBase& createThreadLocal(const TFactory& factory, const Container& ctx) 
        {
            thread_local TBase instance = factory(ctx);
            return instance;
        }

        template <typename T,
            typename TBase = typename get_base_type<T>::type,
            typename TFactory = std::function<TBase (const Container&)>,
            typename TScopedFactory = std::function<TBase& (const TFactory&, const Container&)>>
        T createScoped(const TFactory& factory, const Container& ctx, const TScopedFactory& scopedFactory)
        {
            static_assert(is_singleton_type<T>,
                "Requested type is not compatible with scope (must be one of T*, T&, T&&, std::shared_ptr<T>)");
            auto& instance = scopedFactory(factory, ctx);

            if constexpr (std::is_pointer_v<T>)
                return &instance;
            else if constexpr (cdif::is_shared_ptr<T>)
                return std::shared_ptr<TBase>(&instance, [] (auto*) {});
            else
                return instance;
        }
    }

    template <typename T,
        typename TBase = typename get_base_type<T>::type,
        typename TFactory = std::function<TBase (const Container&)>>
    T createSingleton(const TFactory& factory, const Container& ctx) 
    {
        return detail::createScoped<T>(factory, ctx, detail::createSingleton<T>); 
    }

    template <typename T,
        typename TBase = typename get_base_type<T>::type,
        typename TFactory = std::function<TBase (const Container&)>>
    T createThreadLocal(const TFactory& factory, const Container& ctx)
    {
        return detail::createScoped<T>(factory, ctx, detail::createThreadLocal<T>);
    }
}
