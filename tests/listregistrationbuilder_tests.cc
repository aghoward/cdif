#include "cdif.h"
#include "test_types.h"

#include <gtest/gtest.h>

#include <array>
#include <list>
#include <memory>
#include <string>
#include <vector>

class ListRegistrationBuilderTests : public ::testing::Test
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
        ListRegistrationBuilderTests() : _subject(cdif::Container())
        {
        };
};

TEST_F(ListRegistrationBuilderTests, Resolve_GivenListRegistration_CanResolveVectorOfInterface)
{
    givenRegistrationReturningValue(333);
    _subject.bind<SimpleImplementation, int>().as<Interface>().build();
    _subject.bind<SimpleImplementation, int>().build();
    _subject.bind<SharedImplementationDecorator, int, std::shared_ptr<Interface>>().build();
    _subject.bind<UniqueImplementationDecorator, int, std::unique_ptr<Interface>>().build();
    _subject.bindList<Interface*, SimpleImplementation*, SharedImplementationDecorator*, UniqueImplementationDecorator*>().build();

    auto list = _subject.resolve<std::vector<Interface*>>();

    ASSERT_EQ(list.size(), 3);

    for (auto* item : list)
        delete item;
}

TEST_F(ListRegistrationBuilderTests, Resolve_GivenListRegistration_CanResolveVectorOfSharedPtrInterface)
{
    givenRegistrationReturningValue(333);
    _subject.bind<SimpleImplementation, int>().as<Interface>().build();
    _subject.bind<SimpleImplementation, int>().build();
    _subject.bind<SharedImplementationDecorator, int, std::shared_ptr<Interface>>().build();
    _subject.bind<UniqueImplementationDecorator, int, std::unique_ptr<Interface>>().build();
    _subject.bindList<std::shared_ptr<Interface>,
                      std::shared_ptr<SimpleImplementation>,
                      std::shared_ptr<SharedImplementationDecorator>,
                      std::shared_ptr<UniqueImplementationDecorator>>()
        .build();

    auto list = _subject.resolve<std::vector<std::shared_ptr<Interface>>>();

    ASSERT_EQ(list.size(), 3);
}

TEST_F(ListRegistrationBuilderTests, Resolve_GivenListRegistration_CanResolveVectorOfUniquePtrInterface)
{
    givenRegistrationReturningValue(333);
    _subject.bind<SimpleImplementation, int>().as<Interface>().build();
    _subject.bind<SimpleImplementation, int>().build();
    _subject.bind<SharedImplementationDecorator, int, std::shared_ptr<Interface>>().build();
    _subject.bind<UniqueImplementationDecorator, int, std::unique_ptr<Interface>>().build();
    _subject.bindList<std::unique_ptr<Interface>,
                      std::unique_ptr<SimpleImplementation>,
                      std::unique_ptr<SharedImplementationDecorator>,
                      std::unique_ptr<UniqueImplementationDecorator>>()
        .build();

    auto list = _subject.resolve<std::vector<std::unique_ptr<Interface>>>();

    ASSERT_EQ(list.size(), 3);
}

TEST_F(ListRegistrationBuilderTests, Resolve_GivenListRegistration_CanResolveListOfInterface)
{
    givenRegistrationReturningValue(333);
    _subject.bind<SimpleImplementation, int>().as<Interface>().build();
    _subject.bind<SimpleImplementation, int>().build();
    _subject.bind<SharedImplementationDecorator, int, std::shared_ptr<Interface>>().build();
    _subject.bind<UniqueImplementationDecorator, int, std::unique_ptr<Interface>>().build();
    _subject.bindList<Interface*, SimpleImplementation*, SharedImplementationDecorator*, UniqueImplementationDecorator*>().build();

    auto list = _subject.resolve<std::list<Interface*>>();

    ASSERT_EQ(list.size(), 3);

    for (auto* item : list)
        delete item;
}

TEST_F(ListRegistrationBuilderTests, Resolve_GivenListRegistration_CanResolveListOfSharedPtrInterface)
{
    givenRegistrationReturningValue(333);
    _subject.bind<SimpleImplementation, int>().as<Interface>().build();
    _subject.bind<SimpleImplementation, int>().build();
    _subject.bind<SharedImplementationDecorator, int, std::shared_ptr<Interface>>().build();
    _subject.bind<UniqueImplementationDecorator, int, std::unique_ptr<Interface>>().build();
    _subject.bindList<std::shared_ptr<Interface>,
                      std::shared_ptr<SimpleImplementation>,
                      std::shared_ptr<SharedImplementationDecorator>,
                      std::shared_ptr<UniqueImplementationDecorator>>()
        .build();

    auto list = _subject.resolve<std::list<std::shared_ptr<Interface>>>();

    ASSERT_EQ(list.size(), 3);
}

TEST_F(ListRegistrationBuilderTests, Resolve_GivenListRegistration_CanResolveListOfUniquePtrInterface)
{
    givenRegistrationReturningValue(333);
    _subject.bind<SimpleImplementation, int>().as<Interface>().build();
    _subject.bind<SimpleImplementation, int>().build();
    _subject.bind<SharedImplementationDecorator, int, std::shared_ptr<Interface>>().build();
    _subject.bind<UniqueImplementationDecorator, int, std::unique_ptr<Interface>>().build();
    _subject.bindList<std::unique_ptr<Interface>,
                      std::unique_ptr<SimpleImplementation>,
                      std::unique_ptr<SharedImplementationDecorator>,
                      std::unique_ptr<UniqueImplementationDecorator>>()
        .build();

    auto list = _subject.resolve<std::list<std::unique_ptr<Interface>>>();

    ASSERT_EQ(list.size(), 3);
}

TEST_F(ListRegistrationBuilderTests, Resolve_GivenListRegistration_CanResolveArrayOfInterface)
{
    givenRegistrationReturningValue(333);
    _subject.bind<SimpleImplementation, int>().as<Interface>().build();
    _subject.bind<SimpleImplementation, int>().build();
    _subject.bind<SharedImplementationDecorator, int, std::shared_ptr<Interface>>().build();
    _subject.bind<UniqueImplementationDecorator, int, std::unique_ptr<Interface>>().build();
    _subject.bindList<Interface*, SimpleImplementation*, SharedImplementationDecorator*, UniqueImplementationDecorator*>().build();

    auto list = _subject.resolve<std::array<Interface*, 3>>();

    ASSERT_EQ(list.size(), 3);

    for (auto* item : list)
        delete item;
}

TEST_F(ListRegistrationBuilderTests, Resolve_GivenListRegistration_CanResolveArrayOfSharedPtrInterface)
{
    givenRegistrationReturningValue(333);
    _subject.bind<SimpleImplementation, int>().as<Interface>().build();
    _subject.bind<SimpleImplementation, int>().build();
    _subject.bind<SharedImplementationDecorator, int, std::shared_ptr<Interface>>().build();
    _subject.bind<UniqueImplementationDecorator, int, std::unique_ptr<Interface>>().build();
    _subject.bindList<std::shared_ptr<Interface>,
                      std::shared_ptr<SimpleImplementation>,
                      std::shared_ptr<SharedImplementationDecorator>,
                      std::shared_ptr<UniqueImplementationDecorator>>()
        .build();

    auto list = _subject.resolve<std::array<std::shared_ptr<Interface>, 3>>();

    ASSERT_EQ(list.size(), 3);
}

TEST_F(ListRegistrationBuilderTests, Resolve_GivenListRegistration_CanResolveArrayOfUniquePtrInterface)
{
    givenRegistrationReturningValue(333);
    _subject.bind<SimpleImplementation, int>().as<Interface>().build();
    _subject.bind<SimpleImplementation, int>().build();
    _subject.bind<SharedImplementationDecorator, int, std::shared_ptr<Interface>>().build();
    _subject.bind<UniqueImplementationDecorator, int, std::unique_ptr<Interface>>().build();
    _subject.bindList<std::unique_ptr<Interface>,
                      std::unique_ptr<SimpleImplementation>,
                      std::unique_ptr<SharedImplementationDecorator>,
                      std::unique_ptr<UniqueImplementationDecorator>>()
        .build();

    auto list = _subject.resolve<std::array<std::unique_ptr<Interface>, 3>>();

    ASSERT_EQ(list.size(), 3);
}

TEST_F(ListRegistrationBuilderTests, Resolve_GivenListRegistrationInSingletonScope_CanResolveArrayOfInterface)
{
    givenRegistrationReturningValue(333);
    _subject.bind<SimpleImplementation, int>().as<Interface>().build();
    _subject.bind<SimpleImplementation, int>().build();
    _subject.bind<SharedImplementationDecorator, int, std::shared_ptr<Interface>>().build();
    _subject.bind<UniqueImplementationDecorator, int, std::unique_ptr<Interface>>().build();
    _subject.bindList<Interface*, SimpleImplementation*, SharedImplementationDecorator*, UniqueImplementationDecorator*>()
        .in<cdif::Scope::Singleton>()
        .build();

    auto& list = _subject.resolve<std::array<Interface*, 3>&>();

    ASSERT_EQ(list.size(), 3);

    for (auto* item : list)
        delete item;
}

TEST_F(ListRegistrationBuilderTests, Resolve_GivenListRegistrationInSingletonScope_CanResolveArrayOfSharedPtrInterface)
{
    givenRegistrationReturningValue(333);
    _subject.bind<SimpleImplementation, int>().as<Interface>().build();
    _subject.bind<SimpleImplementation, int>().build();
    _subject.bind<SharedImplementationDecorator, int, std::shared_ptr<Interface>>().build();
    _subject.bind<UniqueImplementationDecorator, int, std::unique_ptr<Interface>>().build();
    _subject.bindList<std::shared_ptr<Interface>,
                      std::shared_ptr<SimpleImplementation>,
                      std::shared_ptr<SharedImplementationDecorator>,
                      std::shared_ptr<UniqueImplementationDecorator>>()
        .in<cdif::Scope::Singleton>()
        .build();

    auto& list = _subject.resolve<std::array<std::shared_ptr<Interface>, 3>&>();

    ASSERT_EQ(list.size(), 3);
}

TEST_F(ListRegistrationBuilderTests, Resolve_GivenListRegistrationInSingletonScope_CanResolveArrayOfUniquePtrInterface)
{
    givenRegistrationReturningValue(333);
    _subject.bind<SimpleImplementation, int>().as<Interface>().build();
    _subject.bind<SimpleImplementation, int>().build();
    _subject.bind<SharedImplementationDecorator, int, std::shared_ptr<Interface>>().build();
    _subject.bind<UniqueImplementationDecorator, int, std::unique_ptr<Interface>>().build();
    _subject.bindList<std::unique_ptr<Interface>,
                      std::unique_ptr<SimpleImplementation>,
                      std::unique_ptr<SharedImplementationDecorator>,
                      std::unique_ptr<UniqueImplementationDecorator>>()
        .in<cdif::Scope::Singleton>()
        .build();

    auto& list = _subject.resolve<std::array<std::unique_ptr<Interface>, 3>&>();

    ASSERT_EQ(list.size(), 3);
}

TEST_F(ListRegistrationBuilderTests, Resolve_GivenListRegistrationInSingletonScope_CanResolveVectorOfInterface)
{
    givenRegistrationReturningValue(333);
    _subject.bind<SimpleImplementation, int>().as<Interface>().build();
    _subject.bind<SimpleImplementation, int>().build();
    _subject.bind<SharedImplementationDecorator, int, std::shared_ptr<Interface>>().build();
    _subject.bind<UniqueImplementationDecorator, int, std::unique_ptr<Interface>>().build();
    _subject.bindList<Interface*, SimpleImplementation*, SharedImplementationDecorator*, UniqueImplementationDecorator*>()
        .in<cdif::Scope::Singleton>()
        .build();

    auto& list = _subject.resolve<std::vector<Interface*>&>();

    ASSERT_EQ(list.size(), 3);

    for (auto* item : list)
        delete item;
}

TEST_F(ListRegistrationBuilderTests, Resolve_GivenListRegistrationInSingletonScope_CanResolveVectorOfSharedPtrInterface)
{
    givenRegistrationReturningValue(333);
    _subject.bind<SimpleImplementation, int>().as<Interface>().build();
    _subject.bind<SimpleImplementation, int>().build();
    _subject.bind<SharedImplementationDecorator, int, std::shared_ptr<Interface>>().build();
    _subject.bind<UniqueImplementationDecorator, int, std::unique_ptr<Interface>>().build();
    _subject.bindList<std::shared_ptr<Interface>,
                      std::shared_ptr<SimpleImplementation>,
                      std::shared_ptr<SharedImplementationDecorator>,
                      std::shared_ptr<UniqueImplementationDecorator>>()
        .in<cdif::Scope::Singleton>()
        .build();

    auto& list = _subject.resolve<std::vector<std::shared_ptr<Interface>>&>();

    ASSERT_EQ(list.size(), 3);
}

TEST_F(ListRegistrationBuilderTests, Resolve_GivenListRegistrationInSingletonScope_CanResolveVectorOfUniquePtrInterface)
{
    givenRegistrationReturningValue(333);
    _subject.bind<SimpleImplementation, int>().as<Interface>().build();
    _subject.bind<SimpleImplementation, int>().build();
    _subject.bind<SharedImplementationDecorator, int, std::shared_ptr<Interface>>().build();
    _subject.bind<UniqueImplementationDecorator, int, std::unique_ptr<Interface>>().build();
    _subject.bindList<std::unique_ptr<Interface>,
                      std::unique_ptr<SimpleImplementation>,
                      std::unique_ptr<SharedImplementationDecorator>,
                      std::unique_ptr<UniqueImplementationDecorator>>()
        .in<cdif::Scope::Singleton>()
        .build();

    auto& list = _subject.resolve<std::vector<std::unique_ptr<Interface>>&>();

    ASSERT_EQ(list.size(), 3);
}

TEST_F(ListRegistrationBuilderTests, Resolve_GivenListRegistrationInSingletonScope_CanResolveListOfInterface)
{
    givenRegistrationReturningValue(333);
    _subject.bind<SimpleImplementation, int>().as<Interface>().build();
    _subject.bind<SimpleImplementation, int>().build();
    _subject.bind<SharedImplementationDecorator, int, std::shared_ptr<Interface>>().build();
    _subject.bind<UniqueImplementationDecorator, int, std::unique_ptr<Interface>>().build();
    _subject.bindList<Interface*, SimpleImplementation*, SharedImplementationDecorator*, UniqueImplementationDecorator*>()
        .in<cdif::Scope::Singleton>()
        .build();

    auto& list = _subject.resolve<std::list<Interface*>&>();

    ASSERT_EQ(list.size(), 3);

    for (auto* item : list)
        delete item;
}

TEST_F(ListRegistrationBuilderTests, Resolve_GivenListRegistrationInSingletonScope_CanResolveListOfSharedPtrInterface)
{
    givenRegistrationReturningValue(333);
    _subject.bind<SimpleImplementation, int>().as<Interface>().build();
    _subject.bind<SimpleImplementation, int>().build();
    _subject.bind<SharedImplementationDecorator, int, std::shared_ptr<Interface>>().build();
    _subject.bind<UniqueImplementationDecorator, int, std::unique_ptr<Interface>>().build();
    _subject.bindList<std::shared_ptr<Interface>,
                      std::shared_ptr<SimpleImplementation>,
                      std::shared_ptr<SharedImplementationDecorator>,
                      std::shared_ptr<UniqueImplementationDecorator>>()
        .in<cdif::Scope::Singleton>()
        .build();

    auto& list = _subject.resolve<std::list<std::shared_ptr<Interface>>&>();

    ASSERT_EQ(list.size(), 3);
}

TEST_F(ListRegistrationBuilderTests, Resolve_GivenListRegistrationInSingletonScope_CanResolveListOfUniquePtrInterface)
{
    givenRegistrationReturningValue(333);
    _subject.bind<SimpleImplementation, int>().as<Interface>().build();
    _subject.bind<SimpleImplementation, int>().build();
    _subject.bind<SharedImplementationDecorator, int, std::shared_ptr<Interface>>().build();
    _subject.bind<UniqueImplementationDecorator, int, std::unique_ptr<Interface>>().build();
    _subject.bindList<std::unique_ptr<Interface>,
                      std::unique_ptr<SimpleImplementation>,
                      std::unique_ptr<SharedImplementationDecorator>,
                      std::unique_ptr<UniqueImplementationDecorator>>()
        .in<cdif::Scope::Singleton>()
        .build();

    auto& list = _subject.resolve<std::list<std::unique_ptr<Interface>>&>();

    ASSERT_EQ(list.size(), 3);
}

