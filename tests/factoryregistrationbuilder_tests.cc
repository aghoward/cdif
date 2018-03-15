#include "cdif.h"
#include "test_types.h"

#include <gtest/gtest.h>

#include <functional>
#include <string>

class FactoryRegistrationBuilderTests : public ::testing::Test
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
        FactoryRegistrationBuilderTests() : _subject(cdif::Container())
        {
        };
};

TEST_F(FactoryRegistrationBuilderTests, Resolve_GivenRegisteredFactoryWithNoArguments_ReturnsFactoryMethod)
{
    auto expectedValue = 44;
    _subject.bind<int>([expectedValue] () { return expectedValue; }).build();

    auto result = _subject.resolve<std::function<int()>>();

    ASSERT_EQ(expectedValue, result());
}

TEST_F(FactoryRegistrationBuilderTests, Resolve_GivenRegisteredFactoryWithSingleArgument_ReturnsFactoryMethod)
{
    auto expectedValue = 44;
    auto factory = [] (int a) { return a; };
    _subject.bind<int, int>(factory).build();

    auto result = _subject.resolve<std::function<int(int)>>();

    ASSERT_EQ(expectedValue, result(expectedValue));
}

TEST_F(FactoryRegistrationBuilderTests, Resolve_GivenRegisteredFactoryWithMultipleArguments_ReturnsFactoryMethod)
{
    auto expectedValue = 44;
    auto factory = [] (int a, int b) { return a * b; };
    _subject.bind<int, int, int>(factory).build();

    auto result = _subject.resolve<std::function<int(int, int)>>();

    ASSERT_EQ(expectedValue, result(expectedValue / 2, 2));
}

TEST_F(FactoryRegistrationBuilderTests, Resolve_GivenFactoryRegistrationWithRegisteredParameterTypes_ReturnsValueFromCallingFactoryWithRegisteredValues)
{
    auto expectedValue = 3242;
    givenRegistrationReturningValue(expectedValue);
    auto factory = [] (int num) { return std::to_string(num); };
    _subject.bind<std::string, int>(factory).build();
    
    auto result = _subject.resolve<std::string>();

    ASSERT_STREQ(std::to_string(expectedValue).c_str(), result.c_str());
}

