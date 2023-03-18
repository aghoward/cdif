#pragma once

#include "cdif/cdif.h"

#include <any>
#include <array>
#include <functional>
#include <initializer_list>
#include <list>
#include <utility>
#include <vector>

namespace cdif {
    template <Scope TScope, typename TService, typename ... TCtorArgs>
    class ListRegistrationBuilder : public RegistrationBuilder<TScope, TService, TCtorArgs...>
    {
        private:
            typedef std::array<std::function<std::any ()>, sizeof...(TCtorArgs)> ResolverCollection;

            template <typename T>
            std::function<T (const Container&)> buildListResolverFor(const std::function<std::initializer_list<TService> (const Container&)>& initializerFactory) const
            {
                return [initializerFactory] (const Container& ctx) -> T
                {
                    return initializerFactory(ctx);
                };
            }

            template <typename TRet>
            void buildRegistrationFrom(
                const std::function<TRet (const Container&)>& factory) const
            {
                this->m_ctx->template bind<TRet>(Registration(factory), this->m_name);
            }

            template <typename TRet, typename TBase = typename get_base_type<TRet>::type>
            void buildScopedRegistrationFrom(
                const std::function<TBase (const Container&)>& factory) const
            {
                this->m_ctx->template bind<TRet>(Registration(buildScopedFactory<TRet>(factory)), this->m_name);
            }

            void buildImpl() const
            {
                buildRegistrationFrom(buildListFrom<std::vector<TService>, TService, TCtorArgs...>(this->m_dependencyResolvers));
                buildRegistrationFrom(buildListFrom<std::list<TService>, TService, TCtorArgs...>(this->m_dependencyResolvers));
                buildRegistrationFrom(buildArrayFrom<std::array<TService, sizeof...(TCtorArgs)>, TService, TCtorArgs...>(this->m_dependencyResolvers));
            }

            template <typename T, typename TBase = typename get_base_type<T>::type>
            std::function<T (const Container&)> buildScopedFactory(
                const std::function<TBase (const Container&)>& resolver) const
            {
                std::function scopedFactory = this->template getScopedFactory<T>();
                return [resolver, scopedFactory] (const Container& ctx) -> decltype(auto) 
                    { 
                        return scopedFactory(resolver, ctx); 
                    };
            }

            void buildScoped()
            {
                auto vectorFactory = buildListFrom<std::vector<TService>, TService, TCtorArgs...>(this->m_dependencyResolvers);
                buildScopedRegistrationFrom<std::vector<TService>&>(vectorFactory);
                buildScopedRegistrationFrom<std::vector<TService>*>(vectorFactory);
                buildScopedRegistrationFrom<std::shared_ptr<std::vector<TService>>>(vectorFactory);

                auto listFactory = buildListFrom<std::list<TService>, TService, TCtorArgs...>(this->m_dependencyResolvers);
                buildScopedRegistrationFrom<std::list<TService>&>(listFactory);
                buildScopedRegistrationFrom<std::list<TService>*>(listFactory);
                buildScopedRegistrationFrom<std::shared_ptr<std::list<TService>>>(listFactory);
                
                using arrayType = std::array<TService, sizeof...(TCtorArgs)>;
                auto arrayFactory = buildArrayFrom<arrayType, TService, TCtorArgs...>(this->m_dependencyResolvers);
                buildScopedRegistrationFrom<arrayType&>(arrayFactory);
                buildScopedRegistrationFrom<arrayType*>(arrayFactory);
                buildScopedRegistrationFrom<std::shared_ptr<arrayType>>(arrayFactory);
            }
            
        public:
            ListRegistrationBuilder(Container* ctx)
                : RegistrationBuilder<TScope, TService, TCtorArgs...>(ctx)
            {
            }

            ListRegistrationBuilder(Container* ctx, ResolverCollection resolvers, std::string name)
                : RegistrationBuilder<TScope, TService, TCtorArgs...>(ctx, resolvers, name) {}

            virtual ~ListRegistrationBuilder() {}
            ListRegistrationBuilder (const ListRegistrationBuilder&) = default;
            ListRegistrationBuilder& operator=(const ListRegistrationBuilder&) = default;
            ListRegistrationBuilder (ListRegistrationBuilder&&) = default;
            ListRegistrationBuilder& operator=(ListRegistrationBuilder&&) = default;

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
                return ListRegistrationBuilder<TNewScope, TService, TCtorArgs...>(
                    this->m_ctx, this->m_dependencyResolvers, this->m_name);
            }
    };
}
