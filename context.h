#pragma once

#include <any>
#include <functional>
#include <map>
#include <memory>
#include <typeinfo>
#include <type_traits>

class Context;

#include "imodule.h"
#include "registrar.h"
#include "registration.h"

class Context {
    private:
        std::unique_ptr<Registrar> _registrar;

    public:
        Context(): _registrar(std::move(std::make_unique<Registrar>())) {};

        template <typename TService, typename TImpl>
        void Register(const std::function<std::shared_ptr<TImpl> (const Context &)> & resolver) {
            static_assert(std::is_base_of<TService, TImpl>::value, "Implementation must be derived from Service");
            auto serviceResolver = [resolver] (const Context & ctx) { return static_cast<std::shared_ptr<TService>>(resolver(ctx)); };

            Register<std::shared_ptr<TService>>(serviceResolver);
        }

        template <typename TService, typename TImpl, typename ... TDeps>
        void RegisterShared() {
            std::function<std::shared_ptr<TImpl> (const Context &)> resolver = nullptr;

            if constexpr (sizeof...(TDeps) > 0)
                resolver = [] (const Context & ctx) { return std::make_shared<TImpl>(ctx.Resolve<TDeps>()...); };
            else
                resolver = [] (const Context & ctx) { return std::make_shared<TImpl>(); };

            Register<TService, TImpl>(resolver);
        }

        template <typename TService>
        void RegisterInstance(const std::shared_ptr<TService> & instance) {
            auto resolver = [instance] (const Context & ctx) { return instance; };
            Register<std::shared_ptr<TService>>(resolver);
        }

        template <typename TService, typename ... TDeps>
        void RegisterInstance(TDeps ... args) {
            std::shared_ptr<TService> instance = nullptr;

            if constexpr (sizeof...(TDeps) > 0)
                instance = std::make_shared<TService>(args...);
            else
                instance = std::make_shared<TService>();

            RegisterInstance<TService>(instance);
        }

        template <typename TService, typename ... TArgs>
        void RegisterFactory(const std::function<TService(TArgs...)> & factory) {
            auto serviceResolver = [factory] (const Context & ctx) { return factory; };
            Register<std::function<TService(TArgs...)>>(serviceResolver);
        }

        template <typename TService>
        void RegisterFactory(const std::function<TService()> & factory) {
            auto serviceResolver = [factory] (const Context & ctx) { return factory; };
            Register<std::function<TService()>>(serviceResolver);
        }

        template <typename TModule>
        void RegisterModule() {
            static_assert(std::is_base_of<IModule, TModule>::value, "Registered modules must derive from IModule.");
            static_assert(std::is_default_constructible<TModule>::value, "Module must have default constructor.");

            auto module = TModule();
            module.Load(*this);
        }

        template <typename TService, typename ... TDeps>
        void Register() {
            std::function<TService (const Context &)> resolver;

            if constexpr (sizeof...(TDeps) > 0)
                resolver = [] (const Context & ctx) { return TService(ctx.Resolve<TDeps>()...); };
            else
                resolver = [] (const Context & ctx) { return TService(); };

            Register<TService>(resolver);
        }

        template <typename TService>
        void Register(const std::function<TService (const Context &)> & serviceResolver) {
            _registrar->Register<TService>(serviceResolver);
        }

        template <typename TService>
        TService Resolve() const {
            const std::unique_ptr<Registration> & registration = _registrar->GetRegistration<TService>();
            return registration->Resolve<TService>(*this);
        }
};
