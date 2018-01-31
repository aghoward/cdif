#pragma once

#include "cdif.h"

#include <any>
#include <array>
#include <functional>
#include <utility>

namespace cdif {
    template <Scope TScope, typename TService, typename TInterface, typename ... TCtorArgs>
    class InterfaceRegistrationBuilder : public RegistrationBuilder<TScope, TService, TCtorArgs...>
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

            template <typename TRet, typename TCasted>
            void buildScopedRegistrationFrom(
                const std::function<TService (const Container&)>& factory) const
            {
                this->m_ctx->template bind<TCasted>(
                    Registration(buildScopedFactory<TRet, TCasted>(factory)), this->m_name);
            }

            
            template <typename Indices = std::make_index_sequence<sizeof...(TCtorArgs)>>
            void buildImpl() const
            {
                buildRegistrationFrom<TService>(
                    defaultFactory<TService, TCtorArgs...>());
                buildRegistrationFrom<TInterface*>(
                    defaultInterfacePtrFactory<TService, TInterface, TCtorArgs...>());
                buildRegistrationFrom<std::shared_ptr<TInterface>>(
                    defaultInterfaceSharedPtrFactory<TService, TInterface, TCtorArgs...>());
                buildRegistrationFrom<std::unique_ptr<TInterface>>(
                    defaultInterfaceUniquePtrFactory<TService, TInterface, TCtorArgs...>());

                this->m_ctx->template bind<std::function<TService (TCtorArgs...)>>(
                    Registration(this->buildFactoryResolver()), this->m_name);
            }

            template <typename T, typename TCasted>
            std::function<TCasted (const Container&)> buildScopedFactory(
                const std::function<TService (const Container&)>& resolver) const
            {
                std::function scopedFactory = this->template getScopedFactory<T>();
                return [resolver, scopedFactory] (const Container& ctx) -> decltype(auto)
                    {
                        return static_cast<TCasted>(scopedFactory(resolver, ctx));
                    };
            }

            template <typename Indices = std::make_index_sequence<sizeof...(TCtorArgs)>>
            void buildScoped() const
            {
                auto resolver = this->buildResolverFrom(defaultFactory<TService, TCtorArgs...>(), Indices{});
                
                buildScopedRegistrationFrom<TService&, TService&>(resolver);
                buildScopedRegistrationFrom<TService&, TInterface&>(resolver);
                buildScopedRegistrationFrom<TService*, TInterface*>(resolver);
                buildScopedRegistrationFrom<std::shared_ptr<TService>, std::shared_ptr<TInterface>>(resolver);
                
                this->m_ctx->template bind<std::function<TService (TCtorArgs...)>>(
                    Registration(this->buildFactoryResolver()), this->m_name);
            }
        
        public:
            InterfaceRegistrationBuilder(Container* ctx, ResolverCollection resolvers, std::string name)
                : RegistrationBuilder<TScope, TService, TCtorArgs...>(ctx, resolvers, name)
            {
                static_assert(has_constructor_with_args<TService, TCtorArgs...>::value,
                    "Cannot find constructor for service matching provided arguments");
                static_assert(std::is_base_of<TInterface, TService>::value, "TService must derive TInterface");
            }

            virtual ~InterfaceRegistrationBuilder() {}
            InterfaceRegistrationBuilder (const InterfaceRegistrationBuilder&) = default;
            InterfaceRegistrationBuilder& operator=(const InterfaceRegistrationBuilder&) = default;
            InterfaceRegistrationBuilder (InterfaceRegistrationBuilder&&) = default;
            InterfaceRegistrationBuilder& operator=(InterfaceRegistrationBuilder&&) = default;
            
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
                return InterfaceRegistrationBuilder<TNewScope, TService, TInterface, TCtorArgs...>(
                    this->m_ctx, this->m_dependencyResolvers, this->m_name);
            }
    };

}
