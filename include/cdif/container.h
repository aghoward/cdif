#pragma once

#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeinfo>

#include "cdif/cdif.h"

namespace cdif {
    class Container
    {
        private:
            static constexpr Scope DefaultScope = Scope::PerDependency;

            std::unique_ptr<cdif::Registrar> m_registrar;
            std::unique_ptr<cdif::ServiceNameFactory> m_serviceNameFactory;
            std::unique_ptr<cdif::PerThreadDependencyChainTracker> m_dependencyChain;

            void checkCircularDependencyResolution(const std::string & name) const
            {
                auto count = m_dependencyChain->increment(name);
                if (count > 1)
                    throw std::runtime_error("Circular dependecy detected while resolving: " + name);
            }

            template <typename TService>
            TService unguardedResolve(const std::string& serviceName) const 
            {
                const std::unique_ptr<Registration> & registration = m_registrar->getRegistration<TService>(serviceName);
                return registration->resolve<TService>(*this);
            }

       public:
            Container() :
                    m_registrar(std::make_unique<cdif::Registrar>()),
                    m_serviceNameFactory(std::make_unique<cdif::ServiceNameFactory>()),
                    m_dependencyChain(std::make_unique<cdif::PerThreadDependencyChainTracker>())
                    {};

            virtual ~Container() = default;

            Container(Container&& other) : m_registrar(std::move(other.m_registrar)) {};

            Container& operator=(Container&& other)
            {
                if (this != &other)
                    m_registrar = std::move(other.m_registrar);
                return *this;
            }

            template <typename TService, typename ... TDeps>
            auto bind() 
            {
                return TypeRegistrationBuilder<DefaultScope, remove_cvref_t<TService>, TDeps...>(this);
            }

            template <typename TReturn,
                typename ... TArgs,
                typename Factory = std::conditional_t<sizeof...(TArgs) == 0, std::function<TReturn()>, std::function<TReturn (TArgs&&...)>>>
            auto bind(const Factory& factory)
            {
                return FactoryRegistrationBuilder<DefaultScope, remove_cvref_t<TReturn>, TArgs...>(this, factory, "");
            }

            template <typename TService, typename ... TImplementations>
            auto bindList()
            {
                return ListRegistrationBuilder<DefaultScope, TService, TImplementations...>(this);
            }

            template <typename TService>
            void bind(const Registration& registration, const std::string& name)
            {
                auto registrationName = m_serviceNameFactory->create<remove_cvref_t<TService>>(name);
                m_registrar->bind(registration, registrationName);
            }

            template <typename TModule>
            void registerModule()
            {
                static_assert(std::is_base_of_v<IModule, TModule>, "TModule must derive IModule");
                auto module = TModule();
                module.load(*this);
            }

            template <typename TService>
            TService resolve(const std::string& name = "") const
            {
                auto serviceName = m_serviceNameFactory->create<remove_cvref_t<TService>>(name);
                checkCircularDependencyResolution(serviceName);
                TService service = unguardedResolve<TService>(serviceName);
                m_dependencyChain->clear(serviceName);
                return service;
            }
    };
}
