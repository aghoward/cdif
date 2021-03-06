#include <string>

namespace cdif {
    class ServiceNameFactory {
        public:
            template <typename TService>
            const std::string create(const std::string& name) const
            {
                auto typeName = typeid(TService).name();
                return typeName + name;
            }
    };
}
