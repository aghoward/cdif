# cdif
C++ Dependency Injection Framework

![Build Status](https://travis-ci.org/aghoward/cdif.svg?branch=master)

cdif is a light-weight header only dependency injection framework for C++ 17

*Note*: cdif requires a compiler compatible with the not yet finalized C++17
standard. This means that you will need to compile all code using cdif with the
`--std=c++17` flag or your compiler's equivalent. 


## Notes on Compiling

Being header only, cdif doesn't require compilation by itself. Obviously this
means that your code will be compiling cdif into itself whenever you use it.
cdif doesn't require any 3rd party libraries, but the thread-safety feature
makes it rely on `pthread` so you'll need to link that when compiling.

As mentioned above, you'll need to compile against the c++17 standard
(for gcc 7+ this is done with `--std=c++17`).

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
 * Works with all types - There is no restriction on the types that can be used.
   Want to resolve a `std::function<std::string()>`? Or a
   `std::unique_ptr<MyInterface>`? No problem.

## About

cdif was designed to make use of constructor dependency injection, it won't do
property injection. The most basic use case is resolve a
`std::shared_ptr<IType>` where `IType` is an interface (pure virtual class). The
container can also resolve concrete types (`Type`), unique pointers 
(`std::unique_ptr<IType>`), factory functions (`std::function<TReturn()>`), and 
singletons (`std::shared_ptr<Type>`).

cdif allows registering and resolving
  * concrete types - [wiki](https://github.com/aghoward/cdif/wiki/Concrete-Type-Registrations)
  * interfaces - [wiki](https://github.com/aghoward/cdif/wiki/Interface-Registrations)
  * lists of interfaces - [wiki](https://github.com/aghoward/cdif/wiki/Interface-List-Registrations)
  * factory functions - [wiki](https://github.com/aghoward/cdif/wiki/Factory-Method-Registrations)

All of the registrations can be customized with the following features:
  * Named Registrations - allows multiple registrations of the same service. [wiki](https://github.com/aghoward/cdif/wiki/Named-Registrations)
  * Scoped Registrations - allows controlling lifetime of your registrations. [wiki](https://github.com/aghoward/cdif/wiki/Scoped-Registrations)
  * Per Dependency Resolution Functions - control how your registration will resolve its' dependencies [wiki](https://github.com/aghoward/cdif/wiki/Named-Registrations#dependency-parameter-customization)

All of the documentation is now contained in the wiki, please see that or the tests
for information on how to use cdif.
