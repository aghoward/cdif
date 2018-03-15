#include "cdif.h"
#include "test_types.h"

#include <gtest/gtest.h>

#include <memory>
#include <string>

class InterfaceRegistrationBuilderTests : public ::testing::Test
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
        InterfaceRegistrationBuilderTests() : _subject(cdif::Container())
        {
        };
};

TEST_F(InterfaceRegistrationBuilderTests, Resolve_GivenInterfaceRegistration_CanResolveConcreteType)
{
    auto expectedValue = 343;
    givenRegistrationReturningValue(expectedValue);
    _subject.bind<SimpleImplementation, int>().as<Interface>().build();

    auto result = _subject.resolve<SimpleImplementation>();

    ASSERT_EQ(expectedValue, result.m_data);
}

TEST_F(InterfaceRegistrationBuilderTests, Resolve_GivenInterfaceRegistration_CanResolveRawPtr)
{
    auto expectedValue = 343;
    givenRegistrationReturningValue(expectedValue);
    _subject.bind<SimpleImplementation, int>().as<Interface>().build();

    auto* result = _subject.resolve<Interface*>();

    ASSERT_EQ(expectedValue, result->m_data);
    delete result;
}

TEST_F(InterfaceRegistrationBuilderTests, Resolve_GivenInterfaceRegistration_CanResolveSharedPtr)
{
    auto expectedValue = 343;
    givenRegistrationReturningValue(expectedValue);
    _subject.bind<SimpleImplementation, int>().as<Interface>().build();

    auto result = _subject.resolve<std::shared_ptr<Interface>>();

    ASSERT_EQ(expectedValue, result->m_data);
}

TEST_F(InterfaceRegistrationBuilderTests, Resolve_GivenInterfaceRegistration_CanResolveUniquePtr)
{
    auto expectedValue = 343;
    givenRegistrationReturningValue(expectedValue);
    _subject.bind<SimpleImplementation, int>().as<Interface>().build();

    auto result = _subject.resolve<std::unique_ptr<Interface>>();

    ASSERT_EQ(expectedValue, result->m_data);
}
