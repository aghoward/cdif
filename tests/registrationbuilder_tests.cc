#include "cdif/cdif.h"
#include "test_types.h"

#include <gtest/gtest.h>

#include <functional>
#include <string>
#include <thread>

class RegistrationBuilderTests : public ::testing::Test
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
        RegistrationBuilderTests() : _subject(cdif::Container())
        {
        };
};

TEST_F(RegistrationBuilderTests, Resolve_GivenNamedRegistration_ReturnsNamedItem)
{
    auto name = "NamedInt";
    auto expectedValue = 5;
    givenRegistrationReturningValue(expectedValue, name);
    givenRegistrationReturningValue(35);

    auto result = _subject.resolve<int>(name);

    ASSERT_EQ(expectedValue, result);
}

TEST_F(RegistrationBuilderTests, Resolve_GivenPerDependencyRegistration_ResolvesNewInstancePerCall)
{
    givenRegistrationReturningValue(322);
    _subject.bind<SimpleImplementation, int>().in<cdif::Scope::PerDependency>().build();

    auto* a = _subject.resolve<SimpleImplementation*>();
    auto* b = _subject.resolve<SimpleImplementation*>();

    ASSERT_NE(a, b);
    delete a;
    delete b;
}

TEST_F(RegistrationBuilderTests, Resolve_GivenSingletonRegistration_ResolvesSameInstancePerCall)
{
    givenRegistrationReturningValue(322);
    _subject.bind<SimpleImplementation, int>().in<cdif::Scope::Singleton>().build();

    auto& a = _subject.resolve<SimpleImplementation&>();
    auto& b = _subject.resolve<SimpleImplementation&>();

    ASSERT_EQ(std::addressof(a), std::addressof(b));
}

TEST_F(RegistrationBuilderTests, Resolve_GivenSingletonRegistration_CanResolveReference)
{
    givenRegistrationReturningValue(343);
    _subject.bind<SimpleImplementation, int>().in<cdif::Scope::Singleton>().build();

    auto& result = _subject.resolve<SimpleImplementation&>();

    ASSERT_NE(std::addressof(result), nullptr);
}

TEST_F(RegistrationBuilderTests, Resolve_GivenSingletonRegistration_CanResolveRawPointer)
{
    givenRegistrationReturningValue(343);
    _subject.bind<SimpleImplementation, int>().in<cdif::Scope::Singleton>().build();

    auto* result = _subject.resolve<SimpleImplementation*>();

    ASSERT_NE(result, nullptr);
}

TEST_F(RegistrationBuilderTests, Resolve_GivenSingletonRegistration_CanResolveSharedPointer)
{
    givenRegistrationReturningValue(343);
    _subject.bind<SimpleImplementation, int>().in<cdif::Scope::Singleton>().build();

    auto result = _subject.resolve<std::shared_ptr<SimpleImplementation>>();

    ASSERT_NE(result.get(), nullptr);
}

TEST_F(RegistrationBuilderTests, Resolve_GivenPerThreadRegistration_ResolvesNewInstancePerThread)
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

TEST_F(RegistrationBuilderTests, Resolve_GivenPerThreadRegistration_ResolvesSingleInstanceInThread)
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

TEST_F(RegistrationBuilderTests, Resolve_GivenRegistrationWithParameterFactory_ResolvesParametersFromFactory) {
    auto expectedValue = 324;
    givenRegistrationReturningValue(333333);
    _subject.bind<SimpleImplementation, int>()
        .withIndexedParameterFrom<0, int>([expectedValue] (const cdif::Container&) { return expectedValue; })
        .build();

    auto result = _subject.resolve<SimpleImplementation>();

    ASSERT_EQ(expectedValue, result.m_data);
}

