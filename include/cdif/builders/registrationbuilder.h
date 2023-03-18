#pragma once

#include "cdif/cdif.h"

#include <any>
#include <array>
#include <functional>
#include <string>
#include <utility>

namespace cdif {
    template <typename TService>
    static const std::function<std::any ()> defaultResolver()
    {
        return [] () -> std::function<TService (const Container&)>
            {
                return [] (const Container& ctx) -> TService { return ctx.template resolve<TService>(); };
            };
    }

    template <Scope TScope, typename TService, typename ... TCtorArgs>
    class RegistrationBuilder
    {
        protected:
            typedef std::array<std::function<std::any ()>, sizeof...(TCtorArgs)> ResolverCollection;

            Container* m_ctx;
            ResolverCollection m_dependencyResolvers;
            std::string m_name;

            template <typename TRet, size_t ... Indices>
            std::function<TRet (const Container&)> buildResolverFrom(
                const std::function<TRet (TCtorArgs&& ...)>& factory,
                std::index_sequence<Indices...>) const
            {
                auto funcs = m_dependencyResolvers;
                return [factory, funcs] (const Container& ctx) 
                { 
                    return factory(std::forward<TCtorArgs>(
                        getResolverAtIndex<TCtorArgs, Indices, sizeof...(TCtorArgs)>(funcs)(ctx))...);
                };
            }

            std::function<std::function<TService (TCtorArgs...)> (const Container&)> buildFactoryResolver() const
            {
                return [] (const Container&) { return defaultFactory<TService, TCtorArgs...>(); };
            }

            template <typename T>
            constexpr auto& getScopedFactory() const
            {
                if constexpr (TScope == Scope::Singleton)
                    return createSingleton<T>;
                else if constexpr (TScope == Scope::PerThread)
                    return createThreadLocal<T>;
            }

        public:
            RegistrationBuilder(Container* ctx)
                : m_ctx(ctx),
                m_name()
            {
                m_dependencyResolvers = 
                {
                    defaultResolver<TCtorArgs>()...
                };
            }

            RegistrationBuilder(Container* ctx, ResolverCollection resolvers, std::string name)
                : m_ctx(ctx), m_dependencyResolvers(resolvers), m_name(name) {}

            virtual ~RegistrationBuilder() {}
            RegistrationBuilder(const RegistrationBuilder&) = default;
            RegistrationBuilder& operator=(const RegistrationBuilder&) = default;
            RegistrationBuilder(RegistrationBuilder&&) = default;
            RegistrationBuilder& operator=(RegistrationBuilder&&) = default;

            template <size_t Index, typename TDependency>
            auto& withIndexedParameterFrom(std::function<TDependency (const Container&)> resolver)
            {
                static_assert(type_at_index_matches<Index, TDependency, TCtorArgs...>(),
                    "Constructor argument at that index does not match function return type");
                m_dependencyResolvers[Index] = [resolver] () { return resolver; };
                return *this;
            }

            auto& named(const std::string& name)
            {
                m_name = name;
                return *this;
            }

            template <typename TInterface>
            auto as()
            {
                return InterfaceRegistrationBuilder<TScope, TService, TInterface, TCtorArgs...>(
                    m_ctx, m_dependencyResolvers, m_name);
            }

            template <Scope TNewScope>
            auto in()
            {
                return TypeRegistrationBuilder<TNewScope, TService, TCtorArgs...>(
                    m_ctx, m_dependencyResolvers, m_name);
            }

            virtual void build() = 0;
    };
}
