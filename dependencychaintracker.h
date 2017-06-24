#pragma once

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <thread>

namespace cdif {
    class DependencyChainTracker {
        private:
            std::map<std::string, size_t> m_dependencyChain;

        public:
            DependencyChainTracker() : m_dependencyChain(std::map<std::string, size_t>()) {};

            size_t Increment(const std::string & name) {
                auto iter = m_dependencyChain.find(name);
                auto value = (iter == m_dependencyChain.end()) ? static_cast<size_t>(0) : iter->second;
                m_dependencyChain.insert_or_assign(name, ++value);
                return value;
            }

            void Clear(const std::string & name) {
                m_dependencyChain.erase(name);
            }

            bool IsEmpty() const {
                return m_dependencyChain.empty();
            }
    };

    class PerThreadDependencyChainTracker {
        private:
            std::hash<std::thread::id> m_hasher;
            mutable std::shared_mutex m_mutex;
            std::map<size_t, std::unique_ptr<DependencyChainTracker>> m_threadChains;

            size_t GetThreadId() const {
                return m_hasher(std::this_thread::get_id());
            }

            std::unique_ptr<DependencyChainTracker> & GetThisChain() {
                auto id = GetThreadId();

                std::shared_lock<std::shared_mutex> readLock(m_mutex);
                auto iter = m_threadChains.find(id);
                if (iter == m_threadChains.end()) {
                    readLock.unlock();
                    CreateChain(id);
                    readLock.lock();
                    iter = m_threadChains.find(id);
                }
                
                return iter->second;
            }

            void CreateChain(size_t id) {
                std::unique_lock<std::shared_mutex> writeLock(m_mutex);
                m_threadChains.insert_or_assign(id, std::move(std::make_unique<DependencyChainTracker>()));
            }

        public:
            PerThreadDependencyChainTracker() : m_threadChains(std::map<size_t, std::unique_ptr<DependencyChainTracker>>()) {};

            size_t Increment(const std::string & name) {
                auto & chain = GetThisChain();
                return chain->Increment(name);
            }

            void Clear(const std::string & name) {
                auto & chain = GetThisChain();
                chain->Clear(name);

                if (chain->IsEmpty()) {
                    std::unique_lock<std::shared_mutex> writeLock(m_mutex);
                    m_threadChains.erase(GetThreadId());
                }
            }

            bool IsEmpty() {
                return m_threadChains.empty();
            }
    };
}
