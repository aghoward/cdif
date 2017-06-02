#pragma once

#include <functional>
#include <map>
#include <memory>
#include <utility>

#include "context.h"
#include "registration.h"

class Registrar { 
    private:
        std::map<size_t, std::unique_ptr<Registration>> _registrations;

    public:
        Registrar() : _registrations(std::map<size_t, std::unique_ptr<Registration>>()) {};

        template <typename T>
        const std::unique_ptr<Registration> & GetRegistration() const {
            auto type = typeid(T).hash_code();
            auto it = _registrations.find(type);
            
            if (it == _registrations.end())
                throw std::invalid_argument(std::string("Type not registered: ") + typeid(T).name());

            return it->second;
        }

        template <typename TService>
        void Register(const std::function<TService (const Context &)> & serviceResolver) {
            auto registration = std::make_unique<Registration>(serviceResolver);
            auto type = typeid(TService).hash_code();
            _registrations.insert_or_assign(type, std::move(registration));
        }      
};
