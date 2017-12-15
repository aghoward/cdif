#pragma once

#include "cdif.h"

#include <functional>
#include <memory>
#include <utility>

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
}
