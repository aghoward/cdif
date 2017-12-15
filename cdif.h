#pragma once

namespace cdif {
    class Container;
    class Registrar;
    class Registration;
    class IModule;
    class ServiceNameFactory;
    class DependencyChainTracker;
    class PerThreadDependencyChainTracker;

    enum class Scope
    {
        PerDependency,
        PerThread,
        Singleton
    };
    
    template <Scope TScope, typename TService, typename ... TArgs>
    class RegistrationBuilder;
    
    template <Scope TScope, typename TService, typename ... TArgs>
    class TypeRegistrationBuilder;
    
    template <Scope TScope, typename TService, typename TInterface, typename ... TArgs>
    class InterfaceRegistrationBuilder;

    template <Scope TScope, typename TReturn, typename ... TArgs>
    class FactoryRegistrationBuilder;
}

#include "typefactories.h"
#include "type_traits.h"
#include "scopedtypefactories.h"
#include "dependencychaintracker.h"
#include "registration.h"
#include "registrar.h"
#include "imodule.h"
#include "servicenamefactory.h"
#include "container.h"
#include "builders/registrationbuilder.h"
#include "builders/typeregistrationbuilder.h"
#include "builders/interfaceregistrationbuilder.h"
#include "builders/factoryregistrationbuilder.h"
