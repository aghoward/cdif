#pragma once

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>
#include <string>
#include <utility>

#include "cdif.h"

namespace cdif {
    class Registrar {
        private:
            std::map<std::string, std::unique_ptr<cdif::Registration>> _registrations;
            mutable std::shared_mutex _mutex;

        public:
            Registrar() : _registrations(std::map<std::string, std::unique_ptr<cdif::Registration>>()) {};

            virtual ~Registrar() = default;

            Registrar(Registrar && other) {
                std::shared_lock<std::shared_mutex> lock(_mutex);
                _registrations = std::move(other._registrations);
            }

            Registrar & operator=(Registrar && other) {
                if (this != &other) {
                    std::shared_lock<std::shared_mutex> lock(_mutex);
                    _registrations = std::move(other._registrations);
                }
                return *this;
            }

            template <typename T>
            const std::unique_ptr<cdif::Registration> & GetRegistration(const std::string & name) const {
                std::shared_lock<std::shared_mutex> lock(_mutex);
                auto it = _registrations.find(name);

                if (it == _registrations.end())
                    throw std::invalid_argument(std::string("Type not registered: ") + typeid(T).name());

                return it->second;
            }

            template <typename TService>
            void Register(const std::function<TService (const cdif::Container &)> & serviceResolver, const std::string & name) {
                std::unique_lock<std::shared_mutex> lock(_mutex);
                auto registration = std::make_unique<Registration>(serviceResolver);
                _registrations.insert_or_assign(name, std::move(registration));
            }
    };
}
