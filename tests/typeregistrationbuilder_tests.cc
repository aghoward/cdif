#include "cdif/cdif.h"
#include "test_types.h"

#include <gtest/gtest.h>

#include <memory>
#include <string>

class TypeRegistrationBuilderTests : public ::testing::Test
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
        TypeRegistrationBuilderTests() : _subject(cdif::Container())
        {
        };
};

TEST_F(TypeRegistrationBuilderTests, Resolve_GivenSimpleClassTypeRegistration_CanResolveSimpleType)
{
    auto expectedValue = SimpleImplementation(45);
    givenRegistrationReturningValue(expectedValue);

    auto result = _subject.resolve<SimpleImplementation>();

    ASSERT_EQ(expectedValue.m_data, result.m_data);
}

TEST_F(TypeRegistrationBuilderTests, Resolve_GivenNonCopyableClass_CanResolveClass)
{
    auto expectedValue = 95;
    givenRegistrationReturningValue(expectedValue);
    _subject.bind<NonCopyable, int>().build();

    auto result = _subject.resolve<NonCopyable>();

    ASSERT_EQ(expectedValue, result.m_a);
}

TEST_F(TypeRegistrationBuilderTests, Resolve_GivenTypeRegistration_CanResolveConcreteType)
{
    auto expectedValue = 352;
    givenRegistrationReturningValue(expectedValue);
    _subject.bind<SimpleImplementation, int>().build();

    auto result = _subject.resolve<SimpleImplementation>();

    ASSERT_EQ(expectedValue, result.m_data);
}

TEST_F(TypeRegistrationBuilderTests, Resolve_GivenTypeRegistration_CanResolveRawPtr)
{
    auto expectedValue = 352;
    givenRegistrationReturningValue(expectedValue);
    _subject.bind<SimpleImplementation, int>().build();

    auto result = _subject.resolve<SimpleImplementation*>();

    ASSERT_EQ(expectedValue, result->m_data);
    delete result;
}

TEST_F(TypeRegistrationBuilderTests, Resolve_GivenTypeRegistration_CanResolveSharedPtr)
{
    auto expectedValue = 352;
    givenRegistrationReturningValue(expectedValue);
    _subject.bind<SimpleImplementation, int>().build();

    auto result = _subject.resolve<std::shared_ptr<SimpleImplementation>>();

    ASSERT_EQ(expectedValue, result->m_data);
}

TEST_F(TypeRegistrationBuilderTests, Resolve_GivenTypeRegistration_CanResolveUniquePtr)
{
    auto expectedValue = 352;
    givenRegistrationReturningValue(expectedValue);
    _subject.bind<SimpleImplementation, int>().build();

    auto result = _subject.resolve<std::unique_ptr<SimpleImplementation>>();

    ASSERT_EQ(expectedValue, result->m_data);
}

TEST_F(TypeRegistrationBuilderTests, Resolve_GivenTypeRegistration_CanResolveFactory)
{
    auto expectedValue = 98;
    _subject.bind<SimpleImplementation, int>().build();

    auto factory = _subject.resolve<std::function<SimpleImplementation (int)>>();
    auto result = factory(expectedValue);

    ASSERT_EQ(expectedValue, result.m_data);
}
