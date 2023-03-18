#pragma once

#include "cdif/cdif.h"

#include <any>
#include <array>
#include <functional>

namespace cdif {
    template <Scope TScope, typename TReturn, typename ... TArgs>
    class FactoryRegistrationBuilder : public RegistrationBuilder<TScope, TReturn, TArgs...>
    {
        private:
            typedef std::function<TReturn (TArgs...)> TService;
            TService m_factory;

            template <typename T>
            constexpr auto getScopedFactoryFactory() const
            {
                if constexpr (TScope == Scope::Singleton)
                    return createSingletonFactory<T, TArgs...>;
                else if constexpr (TScope == Scope::PerThread)
                    return createThreadLocalFactory<T, TArgs...>;
            }

            void buildImpl()
            {
                auto factoryCopy = m_factory;
                std::function factory = [factoryCopy] (const Container&) { return factoryCopy; };
                this->m_ctx->template bind<TService>(Registration(factory), this->m_name);

                std::function valueFactory = [factoryCopy] (const Container& ctx)
                {
                    return factoryCopy(std::forward<TArgs>(ctx.resolve<TArgs>())...);
                };
                this->m_ctx->template bind<TReturn>(Registration(valueFactory), this->m_name);
            }

            template <typename T>
            void buildScopedFactoryFrom(TService factory) const
            {
                auto scopedFactoryFactory = getScopedFactoryFactory<T>();
                std::function scopedFactory = [factory, scopedFactoryFactory] (TArgs...args) -> T
                {
                    return scopedFactoryFactory(factory, std::forward<TArgs>(args)...);
                };

                std::function f = [scopedFactory] (const Container&)
                {
                    return scopedFactory;
                };
                this->m_ctx->template bind<std::function<T (TArgs...)>>(Registration(f), this->m_name);
            }


            void buildScoped() const
            {
                buildScopedFactoryFrom<TReturn&>(m_factory);
                buildScopedFactoryFrom<TReturn*>(m_factory);
                buildScopedFactoryFrom<std::shared_ptr<TReturn>>(m_factory);
            }

        public:
            FactoryRegistrationBuilder (Container* ctx, const TService& factory, const std::string& name)
                : RegistrationBuilder<TScope, TReturn, TArgs...>(ctx),
                m_factory(factory)
            {
                this->m_name = name;
            }

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
                return FactoryRegistrationBuilder<TNewScope, TReturn, TArgs...>(this->m_ctx, this->m_factory, this->m_name);
            }
    };
}
