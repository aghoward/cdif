#pragma once

#include "cdif/cdif.h"

#include <functional>
#include <memory>
#include <utility>
#include <type_traits>

namespace cdif {
    template <typename TService, typename ... TArgs>
    static const std::function<TService (TArgs&&...)> defaultFactory()
    {
        return [] (TArgs&& ... args) { return TService(std::forward<TArgs>(args)...); };
    }

    template <typename TService, typename ... TArgs>
    static const std::function<TService* (TArgs&&...)> defaultPtrFactory()
    {
        return [] (TArgs&& ... args) { return new TService(std::forward<TArgs>(args)...); };
    }

    template <typename TService, typename TInterface, typename ... TArgs>
    static const std::function<TInterface* (TArgs&&...)> defaultInterfacePtrFactory()
    {
        return [] (TArgs&& ... args) { return static_cast<TInterface*>(new TService(std::forward<TArgs>(args)...)); };
    }

    template <typename TService, typename ... TArgs>
    static const std::function<std::shared_ptr<TService> (TArgs&&...)> defaultSharedPtrFactory()
    {
        return [] (TArgs&& ... args) { return std::make_shared<TService>(std::forward<TArgs>(args)...); };
    }

    template <typename TService, typename TInterface, typename ... TArgs>
    static const std::function<std::shared_ptr<TInterface> (TArgs&&...)> defaultInterfaceSharedPtrFactory()
    {
        return [] (TArgs&& ... args)
            {
                return static_cast<std::shared_ptr<TInterface>>(std::make_shared<TService>(std::forward<TArgs>(args)...));
            };
    }

    template <typename TService, typename ... TArgs>
    static const std::function<std::unique_ptr<TService> (TArgs&&...)> defaultUniquePtrFactory()
    {
        return [] (TArgs&& ... args) { return std::make_unique<TService>(std::forward<TArgs>(args)...); };
    }

    template <typename TService, typename TInterface, typename ... TArgs>
    static const std::function<std::unique_ptr<TInterface> (TArgs&&...)> defaultInterfaceUniquePtrFactory()
    {
        return [] (TArgs&& ... args)
            {
                return static_cast<std::unique_ptr<TInterface>>(std::make_unique<TService>(std::forward<TArgs>(args)...));
            };
    }

    template <typename TList, typename TBase, typename ... Ts, size_t ... Indices, size_t TListSize = sizeof...(Ts)>
    static const TList buildInitializerListFrom(
        const std::array<std::function<std::any ()>, sizeof...(Ts)>& resolvers,
        const Container& ctx,
        std::index_sequence<Indices...>)
    {
        return { static_cast<TBase>(getResolverAtIndex<Ts, Indices, TListSize>(resolvers)(ctx))... };
    }

    template <typename TList, typename TBase, typename ... Ts, size_t ... Indices, size_t TListSize = sizeof...(Ts)>
    static constexpr TList buildListFrom(
        const std::array<std::function<std::any ()>, sizeof...(Ts)>& resolvers,
        const Container& ctx,
        std::index_sequence<Indices...>)
    {
        auto list = TList();
        ( list.push_back(static_cast<TBase>(getResolverAtIndex<Ts, Indices, TListSize>(resolvers)(ctx))), ... );
        return list;
    }

    template <typename TList, typename TBase, typename ... Ts, typename Indices = std::make_index_sequence<sizeof...(Ts)>>
    static const std::function<TList (const Container&)> buildArrayFrom(
        const std::array<std::function<std::any ()>, sizeof...(Ts)>& resolvers)
    {
        return [resolvers] (const Container& ctx) -> TList
        {
            return buildInitializerListFrom<TList, TBase, Ts...>(resolvers, ctx, Indices{});
        };
    }

    template <typename TList, typename TBase, typename ... Ts, typename Indices = std::make_index_sequence<sizeof...(Ts)>>
    static const std::function<TList (const Container&)> buildListFrom(
        const std::array<std::function<std::any ()>, sizeof...(Ts)>& resolvers)
    {
        return [resolvers] (const Container& ctx) -> TList
        {
            return buildListFrom<TList, TBase, Ts...>(resolvers, ctx, Indices{});
        };
    }
}
