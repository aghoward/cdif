#pragma once

#include <any>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <typeinfo>
#include <type_traits>

class Container;

#include "imodule.h"
#include "registrar.h"
#include "registration.h"

class Container {
    private:
        std::unique_ptr<Registrar> _registrar;

        template <typename TService>
        const std::string GetServiceName(const std::string & name) const {
            if (!name.empty())
                return name;

            return typeid(TService).name();
        }

    public:
        Container(): _registrar(std::move(std::make_unique<Registrar>())) {};

        template <typename TService, typename TImpl>
        void Register(const std::function<std::shared_ptr<TImpl> (const Container &)> & resolver, const std::string & name = "") {
            static_assert(std::is_base_of<TService, TImpl>::value, "Implementation must be derived from Service");
            auto serviceResolver = [resolver] (const Container & ctx) { return static_cast<std::shared_ptr<TService>>(resolver(ctx)); };

            Register<std::shared_ptr<TService>>(serviceResolver, name);
        }

        template <typename TService, typename TImpl, typename ... TDeps>
        void RegisterShared(const std::string & name = "") {
            auto resolver = [] (const Container & ctx) { return std::make_shared<TImpl>(ctx.Resolve<TDeps>()...); };
            Register<TService, TImpl>(resolver, name);
        }

        template <typename TService>
        void RegisterInstance(const std::shared_ptr<TService> & instance, const std::string & name = "") {
            auto resolver = [instance] (const Container & ctx) { return instance; };
            Register<std::shared_ptr<TService>>(resolver, name);
        }

        template <typename TService, typename ... TDeps>
        void RegisterInstance(TDeps ... args, const std::string & name = "") {
            auto instance = std::make_shared<TService>(args...);
            RegisterInstance<TService>(instance, name);
        }

        template <typename TService, typename ... TArgs>
        void RegisterFactory(const std::function<TService(TArgs...)> & factory, const std::string & name = "") {
            auto serviceResolver = [factory] (const Container & ctx) { return factory; };
            Register<std::function<TService(TArgs...)>>(serviceResolver, name);
        }

        template <typename TService>
        void RegisterFactory(const std::function<TService()> & factory, const std::string & name = "") {
            auto serviceResolver = [factory] (const Container & ctx) { return factory; };
            Register<std::function<TService()>>(serviceResolver, name);
        }

        template <typename TModule>
        void RegisterModule() {
            static_assert(std::is_base_of<IModule, TModule>::value, "Registered modules must derive from IModule.");
            static_assert(std::is_default_constructible<TModule>::value, "Module must have default constructor.");

            auto module = TModule();
            module.Load(*this);
        }

        template <typename TService, typename ... TDeps>
        void Register(const std::string & name = "") {
            auto resolver = [] (const Container & ctx) { return TService(ctx.Resolve<TDeps>()...); };
            Register<TService>(resolver, name);
        }

        template <typename TService>
        void Register(const std::function<TService (const Container &)> & serviceResolver, const std::string & name = "") {
            _registrar->Register<TService>(serviceResolver, GetServiceName<TService>(name));
        }

        template <typename TService>
        TService Resolve(const std::string & name = "") const {
            auto serviceName = GetServiceName<TService>(name);
            const std::unique_ptr<Registration> & registration = _registrar->GetRegistration<TService>(serviceName);
            return registration->Resolve<TService>(*this);
        }
};
