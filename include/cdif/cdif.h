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
    
    template <Scope TScope, typename TService, typename ... TImplementations>
    class ListRegistrationBuilder;
}

#include "cdif/type_traits.h"
#include "cdif/typefactories.h"
#include "cdif/scopedtypefactories.h"
#include "cdif/dependencychaintracker.h"
#include "cdif/registration.h"
#include "cdif/registrar.h"
#include "cdif/imodule.h"
#include "cdif/servicenamefactory.h"
#include "cdif/container.h"
#include "cdif/builders/registrationbuilder.h"
#include "cdif/builders/typeregistrationbuilder.h"
#include "cdif/builders/interfaceregistrationbuilder.h"
#include "cdif/builders/factoryregistrationbuilder.h"
#include "cdif/builders/listregistrationbuilder.h"
