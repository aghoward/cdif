# cdif
C++ Dependency Injection Framework

![Build Status](https://travis-ci.org/aghoward/cdif.svg?branch=master)

cdif is a light-weight header only dependency injection framework for C++ 17

*Note*: cdif requires a compiler compatible with the not yet finalized C++17
standard. This means that you will need to compile all code using cdif with the
`--std=c++17` flag or your compiler's equivalent. Additionally it requires
concepts support (`-fconcepts`).

## Notes on Compiling

Being header only, cdif doesn't require compilation by itself. Obviously this
means that your code will be compiling cdif into itself whenever you use it.
cdif doesn't require any 3rd party libraries, but the thread-safety feature
makes it rely on `pthread` so you'll need to link that when compiling.

As mentioned above, you'll need to compile against the c++17 standard with
concepts (for gcc 7+ this is done with `--std=c++17 -fconcepts`).

To compile the tests you'll need a copy of googletest, and additionally will
need to link `gtest` and `gtest_main`. If you have googletest installed all you
should need to do is run `make` in the tests folder.

## Goals

 * Light-weight - Relies only on the standard library, there are no
   3rd-party dependencies.
 * Easy to use - It is easy to plugin to an existing code base that was
   written with the Dependency Inversion Priciple in mind. There is no need to
   rewrite existing classes or add macros to the header files when switching to
   cdif.
 * Works with all types - There is little no restriction on the types that
   can be used. Want to resolve a `std::function<std::string()>`? No problem.

## About

cdif was designed to make use of constructor dependency injection, it won't do
property injection. The most basic use case is resolve a
`std::shared_ptr<IType>` where `IType` is an interface (pure virtual class). The
container can also resolve concrete types (`Type`), unique pointers 
(`std::unique_ptr<IType>`), factory functions (`std::function<TReturn()>`), and 
singletons (`std::shared_ptr<Type>`).

### Interface Resolution

The most basic use case is class `A` has a dependency on interface `I` and class
`B` implements `I`. Because `I` is a pure virtual class, class `A` cannot have a
member variable `I m_dependency`, so it has a `std::shared_ptr<I> m_dependency`
or `std::unique_ptr<I> m_dependency` which is constructor injected.

Without dependency injection, anyone who wants to use class `A`, needs to first
create a `std::shared_ptr<I>`, then create an `A` instance. With cdif the
container can create the dependency when an instance of `A` is resolved. This is
accomplished by registering both the dependency and the class relying on the
dependency.

 
```cpp
cdif::Container ctx = cdif::Container();
ctx.Register<I, B>();
ctx.Register<I2, A, std::shared_ptr<I>>();
```

In this example, we first create a `cdif::Container` object which is the container and
is responsible for both registrations and resolution. Registrations are unique
to the instance of the container, so registering a service in one container does
not register it in another. Next, the `Register` method is used associate
class `B` with the interface `I`. After this step any resolution of
`std::shared_ptr<I>` will be bound to the implementation of B. Finally class 
`A` is bound to the interface `I2` and will be created using the constructor
which takes a single argument of type `std::shared_ptr<I>`.

In reality, a call to `cdif::Container::Register<TService, TImpl, TDeps...>` 
will make three unique registrations: `TImpl`, `std::shared_ptr<TService>`, and
`std::unique_ptr<TService>`. That is, you can resolve the concrete class, or
either smart pointer to the interface with no additional work.

The signatures of the interface registration methods are:

```cpp
template <typename TService, typename TImpl, typename ... TDeps>
void Register(const std::string & name = "");

template <typename TService, typename TImpl, typename ... TDeps>
void Register(std::function<TDeps (const cdif::Container &)> ... depresolvers,
const std::string & name = "");
```

`TService` is the interface which must be requested by a dependency for
resolution to occur. `TImpl` is the concrete class which implements `TService`.
The parameter pack `TDeps` lists, in order, all the types necessary to construct
an instance of `TImpl` (i.e. all the types listed in the constructor).

In the above example, one can obtain a `std::shared_ptr<I>` by calling
`ctx.Resolve<std::shared_ptr<I>>()`, which will result in the implementation
`B` being returned. Likewise, calling `ctx.Resolve<std::shared_ptr<I2>>()` will
result in the implementation of `A` being returned. Resolving
`std::shared_ptr<I2>` in this example will also result in resolving
`std::shared_ptr<I>` because `A` has a dependency on `std::shared_ptr<I>`;
however, this is hidden from the consumer, all that is necessary to resolve
`std::shared_ptr<I2>` is a call to `ctx.Resolve<std::shared_ptr<I2>>()`. The
container takes care of resolving all the dependencies of the implemenation of
`I2`.

The second signature of the `Register` methods takes a parameter pack of
functions, one for each dependency of `TImpl`. This allows overriding the
default behaviour of dependency resolution which is to call `Resolve` for each
dependency without passing parameters.

Aside from just using the `Register` method, there are also `RegisterShared` and
`RegisterUnique` for registering `std::shared_ptr<TService>` and
`std::unique_ptr<TService>` respectively. They each have two signatures that are
the same as those of `Register`, plus another signature that allows specifying
the function that creates the resolved type. The default functions used to
create smart pointers are `std::make_shared` and `std::make_unique`.

```cpp
template <typename TService, typename TImpl, typename ... TDeps>
void RegisterShared(std::function<std::shared_ptr<TImpl> (const cdif::Container
&)> resolver, const std::string & name = "");

template <typename TService, typename TImpl, typename ... TDeps>
void RegisterUnique(std::function<std::unique_ptr<TImpl> (const cdif::Container
&)> resolver, const std::string & name = "");
```

### Concrete Type Resolution

There are instances where you may want to resolve a class which does not
implement an interface, but may still have its own dependencies. For this we can
use the `RegisterType` method, its signature is:

```cpp
template <typename TService, typename ... TDeps>
void RegisterType(const std::string & name = "");

template <typename TService, typename ... TDeps>
void RegisterType(std::function<TDeps (const cdif::Container &)> ...
depresolvers, consst std::string & name = "");
```

In this case, `TService` is the concrete class we want to register, and `TDeps`
is a parameter pack of all the types required to create an instance of
`TService`. Using the example from the previous section, we could have the
following registrations:

```cpp
cdif::Container ctx = cdif::Container();
ctx.RegisterShared<I, B>();
ctx.RegisterType<A, std::shared_ptr<I>>();
```

In this instance, `A` will only be resolved when an instance of `A` is
requested, it will not be resolved for requests for `std::shared_ptr<I2>`, or
`std::shared_ptr<A>`. Resolution of `A` looks like the following:

```cpp
A instance = ctx.Resolve<A>();
```

The instance of `A` will be created by calling the constructor with the
parameters that result from calling `ctx.Resolve<std::shared_ptr<I>>()`, which
in this exaample will be an instance of `B` wrapped in a `std::shared_ptr<I>`.


### Factory Functions

cdif also allows registering and resolving `std::function<TReturn(TDeps...)>`
objects. This can be achieved through the `RegisterFactory` method which has the
signature:

```cpp
template <typename TService, typename ... TDeps>
void RegisterFactory(const std::function<TService (TDeps...)> & factory, const std::string & name = "");

template <typename TService>
void RegisterFactory(const std::function<TService ()> & factory, const std::string & name = "");
```

The factory function will be copied internally, so the original function does
not need to outlive the `cdif::Container` instance. The only difference between the two
signatures is that one of the registered functions can take any number of
arguments, and the other takes none. This can be used as shown below:

```cpp
std::function<int (double, double)> func = [] (double a, double b) { return (int) (a * b); };
ctx.RegisterFactory<int, double, double>(func);

// ... do some other stuff

auto factory = ctx.Resolve<std::function<int (double, double)>>();
```


### Singletons

Sometimes you may need a single instance of an object resolved in each class
that depends on it. For this, you can use the `RegisterInstance` methods. One of
which takes the exact instance you want resolved (`std::shared_ptr<TService>`),
the other takes the same arguments as the constructor of `TService` and create
the `std::shared_ptr` for you. The two signatures are as show below:

```cpp
template <typename TService>
void RegisterInstance(std::shared_ptr<TService> & instance, const std::string & name = "");

template <typename TService, typename ... TDeps>
void RegisterInstance(TDeps ... args, const std::string & name = "");
```

After calling this, every request for a `std::shared_ptr<TService>` will result
in a `std::shared_ptr` that points to the same object.


### Other Registrations

The above are convenience methods for the most common use-cases, but sometimes
you need something different. All the aforementioned methods are just wrappers
around another `Register` overload which has the following signature:

```cpp
template <typename TService>
void Register(const std::function<TService (const cdif::Container & ctx)> & serviceResolver, const std::string & name = "");
```

Note that if you use this method to register a service, the function which you
pass as `serviceResolver` will be used directly to resolve any requests for
`TService`. As before, the function you register as `serviceResolver` will be
copied internally and thus does not need to outlive the container.


### Named Registrations

You may have noticed that all the above mentioned methods of registration take
an optional parameter `const std::string & name`. By default, all registrations
that do not specify this value, will be registered under the name provided by
calling `typeid(TService).name()`. Subsequent registrations can use the same
name as long they have differing types. Registering the same type twice with the
same name will overwrite the original registration.

Specifying a value for `name` when registering a component allows more than one
registration for a given `TService`. Named components can be resolved by
specifying a matching name to the `Resolve` method:

```cpp
template <typename TService>
TService Resolve(const std::string & name) const;
```

## Registration Modules

Now that we know how to create registrations, its time to look at how to
separate all that "glue" code (in this case registrations) from the business
code. For this you can create classes which implement the `cdif::IModule` interface:

```cpp
// This is defined in imodule.h
namespace cdif {
    class IModule {
        public:
            virtual void Load(cdif::Container & ctx) = 0;
    };
}
```

Implementations must have a default constructor. Implementing this interface
allow you to put all calls to `Register*` inside a separate class and allows you
to separate all the details of dependencies away from your code containing
logic. To make use of a module you can call the `RegisterModule` function:

```cpp
cdif::Container ctx = cdif::Container();
ctx.RegisterModule<ModuleA>();
ctx.RegisterModule<ModuleB>();
```

You can register as many modules as you need and all the registrations contained
within will be made. I tend to wrap even this setup code inside a
`ContainerFactory` object which registers all modules and returns a
`std::unique_ptr<cdif::Container>` which reduces my `main` method to something similar
to:

```cpp
int main () {
    auto ctx = ContainerFactory::Create();
    auto object = ctx->Resolve<MainObject>();
    object.DoWork();
}
```

## Notes

Most of these registrations result in a new instance of `TService` being created
for each call to `Resolve<TService>()`. That is, the lifetime scope of these
registrations is instance per dependency. Of course, `RegisterInstance` is the
exception to this fact. I have also considered implementing instance per thread,
but I do not currently have a use-case for this situation.
