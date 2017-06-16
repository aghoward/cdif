#include <string>

namespace cdif {
    class ServiceNameFactory {
        public:
            template <typename TService>
            const std::string Create(const std::string & name) const {
                auto typeName = typeid(TService).name();
                if (!name.empty())
                    return typeName + name;

                return typeName;
            }
    };
};

