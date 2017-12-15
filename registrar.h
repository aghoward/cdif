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
            std::map<std::string, std::unique_ptr<cdif::Registration>> m_registrations;
            mutable std::shared_mutex m_mutex;

        public:
            Registrar() : m_registrations(std::map<std::string, std::unique_ptr<cdif::Registration>>()) {};

            virtual ~Registrar() = default;

            Registrar(Registrar&& other)
            {
                std::shared_lock<std::shared_mutex> lock(m_mutex);
                m_registrations = std::move(other.m_registrations);
            }

            Registrar& operator=(Registrar&& other)
            {
                if (this != &other) {
                    std::shared_lock<std::shared_mutex> lock(m_mutex);
                    m_registrations = std::move(other.m_registrations);
                }
                return *this;
            }

            template <typename T>
            const std::unique_ptr<cdif::Registration>& getRegistration(const std::string& name) const
            {
                std::shared_lock<std::shared_mutex> lock(m_mutex);
                auto it = m_registrations.find(name);

                if (it == m_registrations.end())
                    throw std::invalid_argument(std::string("Type not registered: ") + typeid(T).name());

                return it->second;
            }

            void bind(const Registration& reg, const std::string& name)
            {
                std::unique_lock<std::shared_mutex> lock(m_mutex);
                auto registration = std::make_unique<Registration>(reg);
                m_registrations.insert_or_assign(name, std::move(registration));
            }
    };
}
