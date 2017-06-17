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
            std::map<std::string, size_t> _dependencyChain;

        public:
            DependencyChainTracker() : _dependencyChain(std::map<std::string, size_t>()) {};

            size_t Increment(const std::string & name) {
                auto iter = _dependencyChain.find(name);
                auto value = (iter == _dependencyChain.end()) ? (size_t)0 : iter->second;
                _dependencyChain.insert_or_assign(name, ++value);
                return value;
            }

            void Clear(const std::string & name) {
                _dependencyChain.erase(name);
            }

            bool IsEmpty() const {
                return _dependencyChain.empty();
            }
    };

    class PerThreadDependencyChainTracker {
        private:
            std::hash<std::thread::id> _hasher;
            mutable std::shared_mutex _mutex;
            std::map<size_t, std::unique_ptr<DependencyChainTracker>> _threadChains;

            size_t GetThreadId() const {
                return _hasher(std::this_thread::get_id());
            }

            std::unique_ptr<DependencyChainTracker> & GetThisChain() {
                auto id = GetThreadId();

                std::shared_lock<std::shared_mutex> readLock(_mutex);
                auto iter = _threadChains.find(id);
                if (iter == _threadChains.end()) {
                    readLock.unlock();
                    CreateChain(id);
                    readLock.lock();
                    iter = _threadChains.find(id);
                }
                
                return iter->second;
            }

            void CreateChain(size_t id) {
                std::unique_lock<std::shared_mutex> writeLock(_mutex);
                auto result = _threadChains.insert_or_assign(id, std::move(std::make_unique<DependencyChainTracker>()));
            }

        public:
            PerThreadDependencyChainTracker() : _threadChains(std::map<size_t, std::unique_ptr<DependencyChainTracker>>()) {};

            size_t Increment(const std::string & name) {
                auto & chain = GetThisChain();
                return chain->Increment(name);
            }

            void Clear(const std::string & name) {
                auto & chain = GetThisChain();
                chain->Clear(name);

                if (chain->IsEmpty()) {
                    std::unique_lock<std::shared_mutex> writeLock(_mutex);
                    _threadChains.erase(GetThreadId());
                }
            }

            bool IsEmpty() {
                return _threadChains.empty();
            }
    };
};
