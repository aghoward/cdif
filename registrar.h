#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <utility>

#include "context.h"
#include "registration.h"

class Registrar { 
    private:
        std::map<std::string, std::unique_ptr<Registration>> _registrations;

    public:
        Registrar() : _registrations(std::map<std::string, std::unique_ptr<Registration>>()) {};

        template <typename T>
        const std::unique_ptr<Registration> & GetRegistration(const std::string & name) const {
            auto it = _registrations.find(name);
            
            if (it == _registrations.end())
                throw std::invalid_argument(std::string("Type not registered: ") + typeid(T).name());

            return it->second;
        }

        template <typename TService>
        void Register(const std::function<TService (const Context &)> & serviceResolver, const std::string & name) {
            auto registration = std::make_unique<Registration>(serviceResolver);
            _registrations.insert_or_assign(name, std::move(registration));
        }
};
