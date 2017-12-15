#pragma once

#include "cdif.h"

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
            std::function<T (const Container&)> buildScopedFactory(
                const std::function<TService (const Container&)>& factory) const
            {
                std::function scopedFactory = this->template getScopedFactory<T>();
                return [factory, scopedFactory] (const Container& ctx) -> decltype(auto)
                    {
                        return scopedFactory(factory, ctx); 
                    };
            }

            template <typename T>
            void buildScopedFactoryFrom(const std::function<TService (const Container&)>& factory) const
            {
                this->m_ctx->template bind<T>(Registration(buildScopedFactory<T>(factory)), this->m_name);
            }

            void buildScoped() const
            {
                auto factoryCopy = m_factory;
                std::function factory = [factoryCopy] (const Container&) { return factoryCopy; };

                buildScopedFactoryFrom<TService&>(factory);
                buildScopedFactoryFrom<TService*>(factory);
                buildScopedFactoryFrom<std::shared_ptr<TService>>(factory);
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
                return FactoryRegistrationBuilder<TNewScope, TReturn, TArgs...>(this->ctx, this->m_factory, this->m_name);
            }
    };
}
