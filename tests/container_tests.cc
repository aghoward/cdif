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

TEST_F(ContainerTests, Resolve_GivenRegistrationRequiringUniquePtr_CanResolveSuccessfully)
{
    givenRegistrationReturningValue(684);
    _subject.bind<SimpleImplementation, int>().as<Interface>().build();
    _subject.bind<UniqueImplementationDecorator, int, std::unique_ptr<Interface>>().build();

    auto result = _subject.resolve<UniqueImplementationDecorator>();
}

