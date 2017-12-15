#include <functional>
#include <stdexcept>
#include <string>
#include <thread>

#include <gtest/gtest.h>

#include "cdif.h"
#include "test_types.h"


class ContainerTests : public ::testing::Test
{
    protected:
        cdif::Container _subject;

        template <typename T>
        void givenRegistrationReturningValue(T value, const std::string& name = "")
        {
            auto factory = [value] () { return value; };
            _subject.bind<T>(factory).named(name).build();
        }

    public:
        ContainerTests() : _subject(cdif::Container())
        {
        };
};

TEST_F(ContainerTests, Resolve_GivenNoMatchingRegistration_ThrowsException)
{
    auto exceptionThrown = false;

    try {
        _subject.resolve<int>();
    } catch (const std::invalid_argument & ex) {
        exceptionThrown = true;
    }

    ASSERT_TRUE(exceptionThrown);
}

TEST_F(ContainerTests, Resolve_GivenMatchingRegistration_ReturnsRegisteredItem)
{
    auto expectedValue = 5;
    givenRegistrationReturningValue(expectedValue);

    auto result = _subject.resolve<int>();

    ASSERT_EQ(expectedValue, result);
}

TEST_F(ContainerTests, Resolve_GivenNamedRegistration_ReturnsNamedItem)
{
    auto name = "NamedInt";
    auto expectedValue = 5;
    givenRegistrationReturningValue(expectedValue, name);
    givenRegistrationReturningValue(35);

    auto result = _subject.resolve<int>(name);

    ASSERT_EQ(expectedValue, result);
}

TEST_F(ContainerTests, Resolve_GivenSimpleClassTypeRegistration_CanResolveSimpleType)
{
    auto expectedValue = SimpleImplementation(45);
    givenRegistrationReturningValue(expectedValue);

    auto result = _subject.resolve<SimpleImplementation>();

    ASSERT_EQ(expectedValue.m_data, result.m_data);
}

TEST_F(ContainerTests, Resolve_GivenNonCopyableClass_CanResolveClass)
{
    auto expectedValue = 95;
    givenRegistrationReturningValue(expectedValue);
    _subject.bind<NonCopyable, int>().build();

    auto result = _subject.resolve<NonCopyable>();

    ASSERT_EQ(expectedValue, result.m_a);
}

TEST_F(ContainerTests, Resolve_GivenCircularDependency_ThrowsException)
{
    givenRegistrationReturningValue(55);
    _subject.bind<SharedImplementationDecorator, int, std::shared_ptr<Interface>>().as<Interface>().build();
    auto exceptionThrown = false;

    try {
        _subject.resolve<SharedImplementationDecorator>();
    } catch (std::runtime_error e) {
        exceptionThrown = true;
    }

    ASSERT_TRUE(exceptionThrown);
}

TEST_F(ContainerTests, Resolve_GivenMultipleResolutionsOfSameType_ResolvesSuccessfully)
{
    auto expectedValue = 89;
    givenRegistrationReturningValue(expectedValue);
    _subject.bind<SimpleImplementation, int>().as<Interface>().build();
    _subject.bind<ComplexImplementation, int, std::shared_ptr<Interface>, std::shared_ptr<Interface>>().build();

    auto result = _subject.resolve<ComplexImplementation>();

    ASSERT_EQ(expectedValue, result.m_data);
}

TEST_F(ContainerTests, Resolve_GivenMultipleRegistrations_CanAutomaticallyResolveDependencies)
{
    auto expectedValue = 89;
    givenRegistrationReturningValue(expectedValue);
    _subject.bind<SimpleImplementation, int>().build();

    auto result = _subject.resolve<SimpleImplementation>();

    ASSERT_EQ(expectedValue, result.m_data);
}

TEST_F(ContainerTests, Resolve_GivenNoDependencies_CanResolveType)
{
    _subject.bind<std::string>().build();

    auto result = _subject.resolve<std::string>();

    ASSERT_STREQ(std::string().c_str(), result.c_str());
}

TEST_F(ContainerTests, Resolve_GivenRegisteredFactoryWithNoArguments_ReturnsFactoryMethod)
{
    auto expectedValue = 44;
    _subject.bind<int>([expectedValue] () { return expectedValue; }).build();

    auto result = _subject.resolve<std::function<int()>>();

    ASSERT_EQ(expectedValue, result());
}

TEST_F(ContainerTests, Resolve_GivenRegisteredFactoryWithSingleArgument_ReturnsFactoryMethod)
{
    auto expectedValue = 44;
    auto factory = [] (int a) { return a; };
    _subject.bind<int, int>(factory).build();

    auto result = _subject.resolve<std::function<int(int)>>();

    ASSERT_EQ(expectedValue, result(expectedValue));
}

TEST_F(ContainerTests, Resolve_GivenRegisteredFactoryWithMultipleArguments_ReturnsFactoryMethod)
{
    auto expectedValue = 44;
    auto factory = [] (int a, int b) { return a * b; };
    _subject.bind<int, int, int>(factory).build();

    auto result = _subject.resolve<std::function<int(int, int)>>();

    ASSERT_EQ(expectedValue, result(expectedValue / 2, 2));
}

TEST_F(ContainerTests, Resolve_GivenFactoryRegistrationWithRegisteredParameterTypes_ReturnsValueFromCallingFactoryWithRegisteredValues)
{
    auto expectedValue = 3242;
    givenRegistrationReturningValue(expectedValue);
    auto factory = [] (int num) { return std::to_string(num); };
    _subject.bind<std::string, int>(factory).build();
    
    auto result = _subject.resolve<std::string>();

    ASSERT_STREQ(std::to_string(expectedValue).c_str(), result.c_str());
}

TEST_F(ContainerTests, Resolve_GivenTypeRegistration_CanResolveConcreteType)
{
    auto expectedValue = 352;
    givenRegistrationReturningValue(expectedValue);
    _subject.bind<SimpleImplementation, int>().build();

    auto result = _subject.resolve<SimpleImplementation>();

    ASSERT_EQ(expectedValue, result.m_data);
}

TEST_F(ContainerTests, Resolve_GivenTypeRegistration_CanResolveRawPtr)
{
    auto expectedValue = 352;
    givenRegistrationReturningValue(expectedValue);
    _subject.bind<SimpleImplementation, int>().build();

    auto result = _subject.resolve<SimpleImplementation*>();

    ASSERT_EQ(expectedValue, result->m_data);
    delete result;
}

TEST_F(ContainerTests, Resolve_GivenTypeRegistration_CanResolveSharedPtr)
{
    auto expectedValue = 352;
    givenRegistrationReturningValue(expectedValue);
    _subject.bind<SimpleImplementation, int>().build();

    auto result = _subject.resolve<std::shared_ptr<SimpleImplementation>>();

    ASSERT_EQ(expectedValue, result->m_data);
}

TEST_F(ContainerTests, Resolve_GivenTypeRegistration_CanResolveUniquePtr)
{
    auto expectedValue = 352;
    givenRegistrationReturningValue(expectedValue);
    _subject.bind<SimpleImplementation, int>().build();

    auto result = _subject.resolve<std::unique_ptr<SimpleImplementation>>();

    ASSERT_EQ(expectedValue, result->m_data);
}

TEST_F(ContainerTests, Resolve_GivenTypeRegistration_CanResolveFactory)
{
    auto expectedValue = 98;
    _subject.bind<SimpleImplementation, int>().build();

    auto factory = _subject.resolve<std::function<SimpleImplementation (int)>>();
    auto result = factory(expectedValue);

    ASSERT_EQ(expectedValue, result.m_data);
}

TEST_F(ContainerTests, Resolve_GivenInterfaceRegistration_CanResolveConcreteType)
{
    auto expectedValue = 343;
    givenRegistrationReturningValue(expectedValue);
    _subject.bind<SimpleImplementation, int>().as<Interface>().build();

    auto result = _subject.resolve<SimpleImplementation>();

    ASSERT_EQ(expectedValue, result.m_data);
}

TEST_F(ContainerTests, Resolve_GivenInterfaceRegistration_CanResolveRawPtr)
{
    auto expectedValue = 343;
    givenRegistrationReturningValue(expectedValue);
    _subject.bind<SimpleImplementation, int>().as<Interface>().build();

    auto* result = _subject.resolve<Interface*>();

    ASSERT_EQ(expectedValue, result->m_data);
    delete result;
}

TEST_F(ContainerTests, Resolve_GivenInterfaceRegistration_CanResolveSharedPtr)
{
    auto expectedValue = 343;
    givenRegistrationReturningValue(expectedValue);
    _subject.bind<SimpleImplementation, int>().as<Interface>().build();

    auto result = _subject.resolve<std::shared_ptr<Interface>>();

    ASSERT_EQ(expectedValue, result->m_data);
}

TEST_F(ContainerTests, Resolve_GivenInterfaceRegistration_CanResolveUniquePtr)
{
    auto expectedValue = 343;
    givenRegistrationReturningValue(expectedValue);
    _subject.bind<SimpleImplementation, int>().as<Interface>().build();

    auto result = _subject.resolve<std::unique_ptr<Interface>>();

    ASSERT_EQ(expectedValue, result->m_data);
}

TEST_F(ContainerTests, Resolve_GivenRegistrationWithParameterFactory_ResolvesParametersFromFactory)
{
    auto expectedValue = 324;
    givenRegistrationReturningValue(333333);
    _subject.bind<SimpleImplementation, int>()
        .withIndexedParameterFrom<0, int>([expectedValue] (const cdif::Container&) { return expectedValue; })
        .build();

    auto result = _subject.resolve<SimpleImplementation>();

    ASSERT_EQ(expectedValue, result.m_data);
}

TEST_F(ContainerTests, Resolve_GivenPerDependencyRegistration_ResolvesNewInstancePerCall)
{
    givenRegistrationReturningValue(322);
    _subject.bind<SimpleImplementation, int>().in<cdif::Scope::PerDependency>().build();

    auto* a = _subject.resolve<SimpleImplementation*>();
    auto* b = _subject.resolve<SimpleImplementation*>();

    ASSERT_NE(a, b);
    delete a;
    delete b;
}

TEST_F(ContainerTests, Resolve_GivenSingletonRegistration_ResolvesSameInstancePerCall)
{
    givenRegistrationReturningValue(322);
    _subject.bind<SimpleImplementation, int>().in<cdif::Scope::Singleton>().build();

    auto& a = _subject.resolve<SimpleImplementation&>();
    auto& b = _subject.resolve<SimpleImplementation&>();

    ASSERT_EQ(std::addressof(a), std::addressof(b));
}

TEST_F(ContainerTests, Resolve_GivenSingletonRegistration_CanResolveReference)
{
    givenRegistrationReturningValue(343);
    _subject.bind<SimpleImplementation, int>().in<cdif::Scope::Singleton>().build();

    auto& result = _subject.resolve<SimpleImplementation&>();

    ASSERT_NE(std::addressof(result), nullptr);
}

TEST_F(ContainerTests, Resolve_GivenSingletonRegistration_CanResolveRawPointer)
{
    givenRegistrationReturningValue(343);
    _subject.bind<SimpleImplementation, int>().in<cdif::Scope::Singleton>().build();

    auto* result = _subject.resolve<SimpleImplementation*>();

    ASSERT_NE(result, nullptr);
}

TEST_F(ContainerTests, Resolve_GivenSingletonRegistration_CanResolveSharedPointer)
{
    givenRegistrationReturningValue(343);
    _subject.bind<SimpleImplementation, int>().in<cdif::Scope::Singleton>().build();

    auto result = _subject.resolve<std::shared_ptr<SimpleImplementation>>();

    ASSERT_NE(result.get(), nullptr);
}

TEST_F(ContainerTests, Resolve_GivenPerThreadRegistration_ResolvesNewInstancePerThread)
{
    NonCopyable* first = nullptr, * second = nullptr;
    givenRegistrationReturningValue(333);
    _subject.bind<NonCopyable, int>().in<cdif::Scope::PerThread>().build();
    auto functor = [&] (NonCopyable** ptr)
    {
        *ptr = _subject.resolve<NonCopyable*>();
    };

    auto t1 = std::thread(functor, &first);
    auto t2 = std::thread(functor, &second);
    t1.join();
    t2.join();

    ASSERT_NE(first, second);
}

TEST_F(ContainerTests, Resolve_GivenPerThreadRegistration_ResolvesSingleInstanceInThread)
{
    givenRegistrationReturningValue(343);
    _subject.bind<NonCopyable, int>().in<cdif::Scope::PerThread>().build();
    auto functor = [&] ()
    {
        auto* p1 = _subject.resolve<NonCopyable*>();
        auto* p2 = _subject.resolve<NonCopyable*>();

        ASSERT_EQ(p1, p2);
    };

    auto t = std::thread(functor);
    t.join();
}
