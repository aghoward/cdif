#pragma once

#include "cdif.h"

#include <any>
#include <array>
#include <functional>
#include <utility>

namespace cdif {
    template <Scope TScope, typename TService, typename ... TCtorArgs>
    class TypeRegistrationBuilder : public RegistrationBuilder<TScope, TService, TCtorArgs...>
    {
        private:
            typedef std::array<std::function<std::any ()>, sizeof...(TCtorArgs)> ResolverCollection;

            template <typename TRet, typename Indices = std::make_index_sequence<sizeof...(TCtorArgs)>>
            void buildRegistrationFrom(
                const std::function<TRet (TCtorArgs&& ...)>& factory) const
            {
                this->m_ctx->template bind<TRet>(
                    Registration(this->buildResolverFrom(factory, Indices{})), this->m_name);
            }

            template <typename TRet>
            void buildScopedRegistrationFrom(
                const std::function<TService (const Container&)>& factory) const
            {
                this->m_ctx->template bind<TRet>(
                    Registration(buildScopedFactory<TRet>(factory)), this->m_name);
            }

            template <typename Indices = std::make_index_sequence<sizeof...(TCtorArgs)>>
            void buildImpl() const
            {
                buildRegistrationFrom(defaultFactory<TService, TCtorArgs...>());
                buildRegistrationFrom(defaultPtrFactory<TService, TCtorArgs...>());
                buildRegistrationFrom(defaultSharedPtrFactory<TService, TCtorArgs...>());
                buildRegistrationFrom(defaultUniquePtrFactory<TService, TCtorArgs...>());

                this->m_ctx->template bind<std::function<TService (TCtorArgs...)>>(
                    Registration(this->buildFactoryResolver()),
                    this->m_name);
            }

            template <typename T>
            std::function<T (const Container&)> buildScopedFactory(
                const std::function<TService (const Container&)>& resolver) const
            {
                std::function scopedFactory = this->template getScopedFactory<T>();
                return [resolver, scopedFactory] (const Container& ctx) -> decltype(auto) 
                    { 
                        return scopedFactory(resolver, ctx); 
                    };
            }

            template <typename Indices = std::make_index_sequence<sizeof...(TCtorArgs)>>
            void buildScoped()
            {
                auto resolver = this->buildResolverFrom(defaultFactory<TService, TCtorArgs...>(), Indices{});
                buildScopedRegistrationFrom<TService&>(resolver);
                buildScopedRegistrationFrom<TService*>(resolver);
                buildScopedRegistrationFrom<std::shared_ptr<TService>>(resolver);
            }
            
        public:
            TypeRegistrationBuilder(Container* ctx)
                : RegistrationBuilder<TScope, TService, TCtorArgs...>(ctx)
            {
                static_assert(has_constructor_with_args<TService, TCtorArgs...>::value,
                    "Cannot find constructor for service matching provided arguments");
            }

            TypeRegistrationBuilder(Container* ctx, ResolverCollection resolvers, std::string name)
                : RegistrationBuilder<TScope, TService, TCtorArgs...>(ctx, resolvers, name) {}

            virtual ~TypeRegistrationBuilder() {}
            TypeRegistrationBuilder (const TypeRegistrationBuilder&) = default;
            TypeRegistrationBuilder& operator=(const TypeRegistrationBuilder&) = default;
            TypeRegistrationBuilder (TypeRegistrationBuilder&&) = default;
            TypeRegistrationBuilder& operator=(TypeRegistrationBuilder&&) = default;

            void build() override
            {
                if constexpr (TScope == Scope::PerDependency)
                    buildImpl();
                else
                    buildScoped();
            }

            template <Scope TNewScope>
            auto in()
            {
                return TypeRegistrationBuilder<TNewScope, TService, TCtorArgs...>(
                    this->m_ctx, this->m_dependencyResolvers, this->m_name);
            }
    };
}
