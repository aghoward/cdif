#pragma once

#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <type_traits>

#include "cdif.h"


namespace cdif {
    class Container {
        private:
            std::unique_ptr<cdif::Registrar> m_registrar;
            std::unique_ptr<cdif::ServiceNameFactory> m_serviceNameFactory;
            std::unique_ptr<cdif::PerThreadDependencyChainTracker> m_dependencyChain;

            void CheckCircularDependencyResolution(const std::string & name) const {
                auto count = m_dependencyChain->Increment(name);
                if (count > 1)
                    throw std::runtime_error("Circular dependecy detected while resolving: " + name);
            }

            template <typename TService>
            TService UnguardedResolve(const std::string & serviceName) const {
                const std::unique_ptr<Registration> & registration = m_registrar->GetRegistration<TService>(serviceName);
                return registration->Resolve<TService>(*this);
            }

            template <typename TService, typename TImpl, typename ... TDeps>
            void RegisterInterface(const std::string & name = "") {
                static_assert(std::is_base_of<TService, TImpl>::value, "Implementation must be derived from Service");

                RegisterShared<TService, TImpl, TDeps...>(name);
                RegisterUnique<TService, TImpl, TDeps...>(name);
                RegisterType<TImpl, TDeps...>(name);
            }

            template <typename TService, typename ... TDeps>
            void RegisterConcrete(const std::string & name = "") {
                RegisterShared<TService, TDeps...>(name);
                RegisterUnique<TService, TDeps...>(name);
                RegisterType<TService, TDeps...>(name);
            }

            template <typename TService, typename ... TDeps>
            void RegisterSharedConcrete(const std::string & name = "") {
                auto resolver = [] (const cdif::Container & ctx) { return std::make_shared<TService>(ctx.Resolve<TDeps>()...); };
                Register<std::shared_ptr<TService>>(resolver, name);
            }

            template <typename TService, typename TImpl, typename ... TDeps>
            void RegisterSharedInterface(const std::string & name = "") {
                auto resolver = [] (const cdif::Container & ctx) { return std::make_shared<TImpl>(ctx.Resolve<TDeps>()...); };
                RegisterShared<TService, TImpl>(resolver, name);
            }

            template <typename TService, typename TImpl, typename ... TDeps>
            void RegisterUniqueInterface(const std::string & name = "") {
                auto resolver = [] (const cdif::Container & ctx) { return std::make_unique<TImpl>(ctx.Resolve<TDeps>()...); };
                RegisterUnique<TService, TImpl>(resolver, name);
            }

            template <typename TService, typename ... TDeps>
            void RegisterUniqueConcrete(const std::string & name = "") {
                auto resolver = [] (const cdif::Container & ctx) { return std::make_unique<TService>(ctx.Resolve<TDeps>()...); };
                Register<std::unique_ptr<TService>>(resolver, name);
            }

        public:
            Container() :
                    m_registrar(std::move(std::make_unique<cdif::Registrar>())), 
                    m_serviceNameFactory(std::move(std::make_unique<cdif::ServiceNameFactory>())),
                    m_dependencyChain(std::move(std::make_unique<cdif::PerThreadDependencyChainTracker>()))
                    {};

            virtual ~Container() = default;

            Container(Container && other) : m_registrar(std::move(other.m_registrar)) {};

            Container & operator=(Container && other) {
                if (this != &other)
                    m_registrar = std::move(other.m_registrar);
                return *this;
            }

            template <typename TService, typename ... TDeps>
            void Register(const std::string & name = "") {
                if constexpr (std::is_abstract<TService>::value)
                    RegisterInterface<TService, TDeps...>(name);
                else
                    RegisterConcrete<TService, TDeps...>(name);
            }

            template <typename TService, typename ... TDeps>
            void Register(std::function<TDeps (const cdif::Container &)> ... depresolvers, const std::string name = "") {
                RegisterShared<TService, TDeps...>(depresolvers..., name);
                RegisterUnique<TService, TDeps...>(depresolvers..., name);
                RegisterType<TService, TDeps...>(depresolvers..., name);
            }

            template <typename TService, typename TImpl, typename ... TDeps>
            void Register(std::function<TDeps (const cdif::Container &)> ... depresolvers, const std::string name = "") {
                static_assert(std::is_base_of<TService, TImpl>::value, "Implementation must be derived from Service");

                RegisterShared<TService, TImpl, TDeps...>(depresolvers..., name);
                RegisterUnique<TService, TImpl, TDeps...>(depresolvers..., name);
                RegisterType<TImpl, TDeps...>(depresolvers..., name);
            }

            template <typename TService, typename ... TDeps>
            void RegisterShared(const std::string & name = "") {
                if constexpr (std::is_abstract<TService>::value)
                    RegisterSharedInterface<TService, TDeps...>(name);
                else
                    RegisterSharedConcrete<TService, TDeps...>(name);
            }

            template <typename TService, typename ... TDeps>
            void RegisterShared(const std::function<TDeps (const cdif::Container &)> & ... depresolvers, const std::string & name = "") {
                auto resolver = [depresolvers...] (const cdif::Container & ctx) { return std::make_shared<TService>(depresolvers(ctx)...); };
                Register<std::shared_ptr<TService>>(resolver, name);
            }

            template <typename TService, typename TImpl, typename ... TDeps>
            void RegisterShared(const std::function<TDeps (const cdif::Container &)> & ... depresolvers, const std::string & name = "") {
                auto resolver = [depresolvers...] (const cdif::Container & ctx) { return std::make_shared<TImpl>(depresolvers(ctx)...); };
                RegisterShared<TService, TImpl>(resolver, name);
            }

            template <typename TService, typename TImpl>
            void RegisterShared(const std::function<std::shared_ptr<TImpl> (const cdif::Container &)> & resolver, const std::string & name = "") {
                static_assert(std::is_base_of<TService, TImpl>::value, "Implementation must be derived from Service");
                auto serviceResolver = [resolver] (const cdif::Container & ctx) { return static_cast<std::shared_ptr<TService>>(resolver(ctx)); };

                Register<std::shared_ptr<TService>>(serviceResolver, name);
            }

            template <typename TService, typename ... TDeps>
            void RegisterUnique(const std::string & name = "") {
                if constexpr(std::is_abstract<TService>::value)
                    RegisterUniqueInterface<TService, TDeps...>(name);
                else
                    RegisterUniqueConcrete<TService, TDeps...>(name);
            }

            template <typename TService, typename ... TDeps>
            void RegisterUnique(const std::function<TDeps (const cdif::Container &)> & ... depresolvers, const std::string & name = "") {
                auto resolver = [depresolvers...] (const cdif::Container & ctx) { return std::make_unique<TService>(depresolvers(ctx)...); };
                Register<std::unique_ptr<TService>>(resolver, name);
            }

            template <typename TService, typename TImpl, typename ... TDeps>
            void RegisterUnique(const std::function<TDeps (const cdif::Container &)> & ... depresolvers, const std::string & name = "") {
                auto resolver = [depresolvers...] (const cdif::Container & ctx) { return std::make_unique<TImpl>(depresolvers(ctx)...); };
                RegisterUnique<TService, TImpl>(resolver, name);
            }

            template <typename TService, typename TImpl>
            void RegisterUnique(const std::function<std::unique_ptr<TImpl> (const cdif::Container &)> & resolver, const std::string & name = "") {
                static_assert(std::is_base_of<TService, TImpl>::value, "Implementation must be derived from Service");
                auto serviceResolver = [resolver] (const cdif::Container & ctx) { return std::move(static_cast<std::unique_ptr<TService>>(resolver(ctx))); };
                
                Register<std::unique_ptr<TService>>(resolver, name);
            }

            template <typename TService, typename ... TArgs>
            void RegisterFactory(const std::function<std::conditional_t<sizeof...(TArgs) == 0, TService (), TService (TArgs...)>> & factory, const std::string & name = "") {
                auto serviceResolver = [factory] (const cdif::Container &) { return factory; };
                Register<std::function<std::conditional_t<sizeof...(TArgs) == 0, TService (), TService (TArgs...)>>>(serviceResolver, name);
            }

            template <typename TService>
            void RegisterInstance(const std::shared_ptr<TService> & instance, const std::string & name = "") {
                auto resolver = [instance] (const cdif::Container &) { return instance; };
                Register<std::shared_ptr<TService>>(resolver, name);
            }

            template <typename TService, typename ... TDeps>
            void RegisterInstance(TDeps ... args, const std::string & name = "") {
                auto instance = std::make_shared<TService>(args...);
                RegisterInstance<TService>(instance, name);
            }
            template <typename TModule>
            void RegisterModule() {
                static_assert(std::is_base_of<IModule, TModule>::value, "Registered modules must derive from IModule.");
                static_assert(std::is_default_constructible<TModule>::value, "Module must have default constructor.");

                auto module = TModule();
                module.Load(*this);
            }

            template <typename TService, typename ... TDeps>
            void RegisterType(const std::string & name = "") {
                auto resolver = [] (const cdif::Container & ctx) { return TService(ctx.Resolve<TDeps>()...); };
                Register<TService>(resolver, name);
            }

            template <typename TService, typename ... TDeps>
            void RegisterType(const std::function<TDeps (const cdif::Container &)> & ... depresolvers, const std::string name = "") {
                auto resolver = [depresolvers...] (const cdif::Container & ctx) { return TService(depresolvers(ctx)...); };
                Register<TService>(resolver, name);
            }

            template <typename TService>
            void Register(const std::function<TService (const cdif::Container &)> & serviceResolver, const std::string & name = "") {
                m_registrar->Register<TService>(serviceResolver, m_serviceNameFactory->Create<TService>(name));
            }

            template <typename TService>
            TService Resolve(const std::string & name = "") const {
                auto serviceName = m_serviceNameFactory->Create<TService>(name);
                CheckCircularDependencyResolution(serviceName);
                auto service = UnguardedResolve<TService>(serviceName);
                m_dependencyChain->Clear(serviceName);
                return service;
            }
    };
}
