#pragma once

#include <functional>
#include <memory>
#include <type_traits>
#include <utility>

#include "cdif/type_traits.h"

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
            typename ... TArgs,
            typename TBase = typename get_base_type<T>::type,
            typename TFactory = std::function<TBase (TArgs...)>>
        TBase& createSingletonFactory(const TFactory& factory, TArgs&&... args)
        {
            static TBase instance = factory(std::forward<TArgs>(args)...);
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

        template <
            typename T,
            typename ... TArgs,
            typename TBase = typename get_base_type<T>::type,
            typename TFactory = std::function<TBase (TArgs...)>>
        TBase& createThreadLocalFactory(const TFactory& factory, TArgs&&... args)
        {
            thread_local TBase instance = factory(std::forward<TArgs>(args)...);
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

        template <typename T,
            typename ... TArgs,
            typename TBase = typename get_base_type<T>::type,
            typename TFactory = std::function<TBase (TArgs...)>,
            typename TScopedFactory = std::function<TBase& (const TFactory&, TArgs...)>>
        T createScopedFactory(const TFactory& factory, const TScopedFactory& scopedFactory, TArgs&&... args)
        {
            static_assert(is_singleton_type<T>,
                "Requested type is not compatible with scope (must be one of T*, T&, T&&, std::shared_ptr<T>)");
            auto& instance = scopedFactory(factory, std::forward<TArgs>(args)...);

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

    template <typename T,
        typename ... TArgs,
        typename TBase = typename get_base_type<T>::type,
        typename TFactory = std::function<TBase (TArgs...)>>
    T createSingletonFactory(const TFactory& factory, TArgs&&... args)
    {
        return detail::createScopedFactory<T>(factory, detail::createSingletonFactory<T, TArgs...>, std::forward<TArgs>(args)...);
    }

    template <typename T,
        typename ... TArgs,
        typename TBase = typename get_base_type<T>::type,
        typename TFactory = std::function<TBase (TArgs...)>>
    T createThreadLocalFactory(const TFactory& factory, TArgs&&... args)
    {
        return detail::createScopedFactory<T>(factory, detail::createThreadLocalFactory<T, TArgs...>, std::forward<TArgs>(args)...);
    }
}
