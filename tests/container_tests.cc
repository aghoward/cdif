#include <functional>
#include <stdexcept>
#include <string>

#include <gtest/gtest.h>

#include "cdif.h"
#include "test_types.h"


class ContainerTests : public ::testing::Test {
    protected:
        cdif::Container _subject;

        template <typename T>
        void GivenRegistrationReturningValue(T value, const std::string & name = "") {
            auto serviceResolver = [value] (const cdif::Container &) { return value; }; 
            _subject.Register<T>(serviceResolver, name);
        }

    public:
        ContainerTests() : _subject(cdif::Container()) {
        };
};

TEST_F(ContainerTests, Resolve_GivenNoMatchingRegistration_ThrowsException) {
    auto exceptionThrown = false;

    try {
        _subject.Resolve<int>();
    } catch (const std::invalid_argument & ex) {
        exceptionThrown = true;
    }

    ASSERT_TRUE(exceptionThrown);
}

TEST_F(ContainerTests, Resolve_GivenMatchingRegistration_ReturnsRegisteredItem) {
    auto expectedValue = 5;
    GivenRegistrationReturningValue(expectedValue);

    auto result = _subject.Resolve<int>();

    ASSERT_EQ(expectedValue, result);
}

TEST_F(ContainerTests, Resolve_GivenNamedRegistration_ReturnsNamedItem) {
    auto name = "NamedInt";
    auto expectedValue = 5;
    GivenRegistrationReturningValue(expectedValue, name);
    GivenRegistrationReturningValue(35);

    auto result = _subject.Resolve<int>(name);

    ASSERT_EQ(expectedValue, result);
}

TEST_F(ContainerTests, Resolve_GivenSimpleClassTypeRegistration_CanResolveSimpleType) {
    auto expectedValue = SimpleImplementation(45);
    GivenRegistrationReturningValue(expectedValue);

    auto result = _subject.Resolve<SimpleImplementation>();

    ASSERT_EQ(expectedValue.m_data, result.m_data);
}

TEST_F(ContainerTests, Resolve_GivenNonCopyableClass_CanResolveClass) {
    auto expectedValue = 95;
    auto resolver = [expectedValue] (const cdif::Container &) { 
        return std::move(
            UniqueImplementationDecorator(
                expectedValue,
                static_cast<std::unique_ptr<Interface>>(std::make_unique<SimpleImplementation>(444))));
    };
    _subject.Register<UniqueImplementationDecorator>(resolver);

    auto result = _subject.Resolve<UniqueImplementationDecorator>();

    ASSERT_EQ(expectedValue, result.m_data);
}

TEST_F(ContainerTests, Resolve_GivenCircularDependency_ThrowsException) {
    auto circularResolver = [] (const cdif::Container & ctx) {
        return std::make_shared<SharedImplementationDecorator>(
                55,
                static_cast<std::shared_ptr<Interface>>(ctx.Resolve<std::shared_ptr<SharedImplementationDecorator>>()));
    };
    _subject.Register<std::shared_ptr<SharedImplementationDecorator>>(circularResolver);
    auto exceptionThrown = false;

    try {
        _subject.Resolve<std::shared_ptr<SharedImplementationDecorator>>();
    } catch (std::runtime_error e) {
        exceptionThrown = true;
    }

    ASSERT_TRUE(exceptionThrown);
}

TEST_F(ContainerTests, Resolve_GivenMultipleResolutionsOfSameType_ResolvesSuccessfully) {
    auto expectedValue = 89;
    auto innerResolver = [] (const cdif::Container &) { return std::move(static_cast<std::shared_ptr<Interface>>(std::make_shared<SimpleImplementation>(54))); };
    auto resolver = [expectedValue] (const cdif::Container & ctx) {
            return ComplexImplementation(expectedValue, ctx.Resolve<std::shared_ptr<Interface>>(), ctx.Resolve<std::shared_ptr<Interface>>());
    };
    _subject.Register<std::shared_ptr<Interface>>(innerResolver);
    _subject.Register<ComplexImplementation>(resolver);

    auto result = _subject.Resolve<ComplexImplementation>();

    ASSERT_EQ(expectedValue, result.m_data);
}

TEST_F(ContainerTests, Resolve_GivenMultipleRegistrations_CanAutomaticallyResolveDependencies) {
    auto expectedValue = 89;
    auto dataResolver = [expectedValue] (const cdif::Container &) { return expectedValue; };
    _subject.Register<int>(dataResolver);

    _subject.RegisterType<SimpleImplementation, int>();
    auto result = _subject.Resolve<SimpleImplementation>();

    ASSERT_EQ(expectedValue, result.m_data);
}

TEST_F(ContainerTests, Resolve_GivenDependencyResolversInRegistration_CanResolveDependencies) {
    auto expectedValue = 89;
    auto dataResolver = [expectedValue] (const cdif::Container &) { return expectedValue; };
    
    _subject.RegisterType<SimpleImplementation, int>(dataResolver);
    auto result = _subject.Resolve<SimpleImplementation>();

    ASSERT_EQ(expectedValue, result.m_data);
}

TEST_F(ContainerTests, Resolve_GivenNoDependencies_CanResolveType) {
    _subject.RegisterType<std::string>();

    auto result = _subject.Resolve<std::string>();

    ASSERT_STREQ(std::string().c_str(), result.c_str());
}

TEST_F(ContainerTests, Resolve_GivenRegisteredFactoryWithNoArguments_ReturnsFactoryMethod) {
    auto expectedValue = 44;
    auto factory = [expectedValue] () { return expectedValue; };
    _subject.RegisterFactory<int>(factory);

    auto result = _subject.Resolve<std::function<int()>>();

    ASSERT_EQ(expectedValue, result());
}

TEST_F(ContainerTests, Resolve_GivenRegisteredFactoryWithSingleArgument_ReturnsFactoryMethod) {
    auto expectedValue = 44;
    auto factory = [] (int a) { return a; };
    _subject.RegisterFactory<int, int>(factory);

    auto result = _subject.Resolve<std::function<int(int)>>();

    ASSERT_EQ(expectedValue, result(expectedValue));
}

TEST_F(ContainerTests, Resolve_GivenRegisteredFactoryWithMultipleArguments_ReturnsFactoryMethod) {
    auto expectedValue = 44;
    auto factory = [] (int a, int b) { return a * b; };
    _subject.RegisterFactory<int, int, int>(factory);

    auto result = _subject.Resolve<std::function<int(int, int)>>();

    ASSERT_EQ(expectedValue, result(expectedValue / 2, 2));
}

TEST_F(ContainerTests, Resolve_GivenRegisteredSingleton_ReturnsSameInstance) {
    auto expectedValue = std::make_shared<SimpleImplementation>(33);
    _subject.RegisterInstance<SimpleImplementation>(expectedValue);

    auto result = _subject.Resolve<std::shared_ptr<SimpleImplementation>>();

    ASSERT_EQ(expectedValue.get(), result.get());
}

TEST_F(ContainerTests, Resolve_GivenRegisteredSingleton_CanMakeAndReturnSingleInstance) {
    _subject.RegisterInstance<SimpleImplementation, int>(45);

    auto first = _subject.Resolve<std::shared_ptr<SimpleImplementation>>();
    auto second = _subject.Resolve<std::shared_ptr<SimpleImplementation>>();

    ASSERT_EQ(first.get(), second.get());
}

TEST_F(ContainerTests, Resolve_GiveRegisterShared_CanResolveAsInterface) {
    auto expectedValue = 333;
    _subject.Register<int>([expectedValue] (const cdif::Container &) { return expectedValue; });
    _subject.RegisterShared<Interface, SimpleImplementation, int>();

    auto result = _subject.Resolve<std::shared_ptr<Interface>>();

    ASSERT_EQ(expectedValue, result->m_data);
}

TEST_F(ContainerTests, Resolve_GivenRegisterSharedWithDependencyResolvers_CanResolveObject) {
    auto expectedValue = 325;
    auto dataResolver = [expectedValue] (const cdif::Container &) { return expectedValue; };
    _subject.RegisterShared<Interface, SimpleImplementation, int>(dataResolver);

    auto result = _subject.Resolve<std::shared_ptr<Interface>>();

    ASSERT_EQ(expectedValue, result->m_data);
}

TEST_F(ContainerTests, Resolve_GivenRegisterSharedWithResolver_CanResolveInterface) {
    auto expectedValue = 653;
    auto resolver = [expectedValue] (const cdif::Container &) { return std::make_shared<SimpleImplementation>(expectedValue); };
    _subject.RegisterShared<Interface, SimpleImplementation>(resolver);

    auto result = _subject.Resolve<std::shared_ptr<Interface>>();

    ASSERT_EQ(expectedValue, result->m_data);
}

TEST_F(ContainerTests, Resolve_GivenRegisterSharedWithConcreteType_CanResolveConcreteType) {
    auto expectedValue = 531;
    auto dataResolver = [expectedValue] (const cdif::Container &) { return expectedValue; };
    _subject.Register<int>(dataResolver);
    _subject.RegisterShared<SimpleImplementation, int>();

    auto result = _subject.Resolve<std::shared_ptr<SimpleImplementation>>();

    ASSERT_EQ(expectedValue, result->m_data);
}

TEST_F(ContainerTests, Resolve_GivenRegisterSharedWithConcreteTypeAndDependencyResolvers_CanResolveConcreteType) {
    auto expectedValue = 531;
    auto dataResolver = [expectedValue] (const cdif::Container &) { return expectedValue; };
    _subject.RegisterShared<SimpleImplementation, int>(dataResolver);

    auto result = _subject.Resolve<std::shared_ptr<SimpleImplementation>>();

    ASSERT_EQ(expectedValue, result->m_data);
}


TEST_F(ContainerTests, Resolve_GivenRegisterUnique_CanResolveAsInterface) {
    auto expectedValue = 333;
    _subject.Register<int>([expectedValue] (const cdif::Container &) { return expectedValue; });
    _subject.RegisterUnique<Interface, SimpleImplementation, int>();

    auto result = _subject.Resolve<std::unique_ptr<Interface>>();

    ASSERT_EQ(expectedValue, result->m_data);
}

TEST_F(ContainerTests, Resolve_GivenRegisterUniqueWithDependencyResolvers_CanResolveObject) {
    auto expectedValue = 325;
    auto dataResolver = [expectedValue] (const cdif::Container &) { return expectedValue; };
    _subject.RegisterUnique<Interface, SimpleImplementation, int>(dataResolver);

    auto result = _subject.Resolve<std::unique_ptr<Interface>>();

    ASSERT_EQ(expectedValue, result->m_data);
}

TEST_F(ContainerTests, Resolve_GivenRegisterUniqueWithResolver_CanResolveInterface) {
    auto expectedValue = 653;
    auto resolver = [expectedValue] (const cdif::Container &) { return std::make_unique<SimpleImplementation>(expectedValue); };
    _subject.RegisterUnique<Interface, SimpleImplementation>(resolver);

    auto result = _subject.Resolve<std::unique_ptr<Interface>>();

    ASSERT_EQ(expectedValue, result->m_data);
}

TEST_F(ContainerTests, Resolve_GivenRegisterUniqueWithConcreteType_CanResolveConcreteType) {
    auto expectedValue = 531;
    auto dataResolver = [expectedValue] (const cdif::Container &) { return expectedValue; };
    _subject.Register<int>(dataResolver);
    _subject.RegisterUnique<SimpleImplementation, int>();

    auto result = _subject.Resolve<std::unique_ptr<SimpleImplementation>>();

    ASSERT_EQ(expectedValue, result->m_data);
}

TEST_F(ContainerTests, Resolve_GivenRegisterUniqueWithConcreteTypeAndDependencyResolvers_CanResolveConcreteType) {
    auto expectedValue = 531;
    auto dataResolver = [expectedValue] (const cdif::Container &) { return expectedValue; };
    _subject.RegisterUnique<SimpleImplementation, int>(dataResolver);

    auto result = _subject.Resolve<std::unique_ptr<SimpleImplementation>>();

    ASSERT_EQ(expectedValue, result->m_data);
}

TEST_F(ContainerTests, Resolve_GivenRegistrationOfInterface_CanResolveSharedInterface) {
    auto expectedValue = 325;
    auto dataResolver = [expectedValue] (const cdif::Container &) { return expectedValue; };
    _subject.Register<Interface, SimpleImplementation, int>(dataResolver);

    auto result = _subject.Resolve<std::shared_ptr<Interface>>();

    ASSERT_EQ(expectedValue, result->m_data);
}

TEST_F(ContainerTests, Resolve_GivenRegistrationOfInterface_CanResolveUniqueInterface) {
    auto expectedValue = 325;
    auto dataResolver = [expectedValue] (const cdif::Container &) { return expectedValue; };
    _subject.Register<Interface, SimpleImplementation, int>(dataResolver);

    auto result = _subject.Resolve<std::unique_ptr<Interface>>();

    ASSERT_EQ(expectedValue, result->m_data);
}

TEST_F(ContainerTests, Resolve_GivenRegistrationOfInterface_CanResolveImplementation) {
    auto expectedValue = 325;
    auto dataResolver = [expectedValue] (const cdif::Container &) { return expectedValue; };
    _subject.Register<Interface, SimpleImplementation, int>(dataResolver);

    auto result = _subject.Resolve<SimpleImplementation>();

    ASSERT_EQ(expectedValue, result.m_data);
}

TEST_F(ContainerTests, Resolve_GivenRegistrationOfConcreteType_CanResolveSharedConcreteType) {
    auto expectedValue = 885;
    auto dataResolver = [expectedValue] (const cdif::Container &) { return expectedValue; };
    _subject.Register<int>(dataResolver);
    _subject.Register<SimpleImplementation, int>();

    auto result = _subject.Resolve<std::shared_ptr<SimpleImplementation>>();

    ASSERT_EQ(expectedValue, result->m_data);
}

TEST_F(ContainerTests, Resolve_GivenRegistrationOfConcreteType_CanResolveUniqueConcreteType) {
    auto expectedValue = 885;
    auto dataResolver = [expectedValue] (const cdif::Container &) { return expectedValue; };
    _subject.Register<int>(dataResolver);
    _subject.Register<SimpleImplementation, int>();

    auto result = _subject.Resolve<std::unique_ptr<SimpleImplementation>>();

    ASSERT_EQ(expectedValue, result->m_data);
}

TEST_F(ContainerTests, Resolve_GivenRegistrationOfConcreteType_CanResolveType) {
    auto expectedValue = 885;
    auto dataResolver = [expectedValue] (const cdif::Container &) { return expectedValue; };
    _subject.Register<int>(dataResolver);
    _subject.Register<SimpleImplementation, int>();

    auto result = _subject.Resolve<SimpleImplementation>();

    ASSERT_EQ(expectedValue, result.m_data);
}

TEST_F(ContainerTests, Resolve_GivenRegistrationWithDependencyResolvers_CanResolveSharedType) {
    auto expectedValue = 885;
    auto dataResolver = [expectedValue] (const cdif::Container &) { return expectedValue; };
    _subject.Register<SimpleImplementation, int>(dataResolver);

    auto result = _subject.Resolve<std::shared_ptr<SimpleImplementation>>();

    ASSERT_EQ(expectedValue, result->m_data);
}

TEST_F(ContainerTests, Resolve_GivenRegistrationWithDependencyResolvers_CanResolveUniqueType) {
    auto expectedValue = 885;
    auto dataResolver = [expectedValue] (const cdif::Container &) { return expectedValue; };
    _subject.Register<SimpleImplementation, int>(dataResolver);

    auto result = _subject.Resolve<std::unique_ptr<SimpleImplementation>>();

    ASSERT_EQ(expectedValue, result->m_data);
}

TEST_F(ContainerTests, Resolve_GivenRegistrationWithDependencyResolvers_CanResolveType) {
    auto expectedValue = 885;
    auto dataResolver = [expectedValue] (const cdif::Container &) { return expectedValue; };
    _subject.Register<SimpleImplementation, int>(dataResolver);

    auto result = _subject.Resolve<SimpleImplementation>();

    ASSERT_EQ(expectedValue, result.m_data);
}
