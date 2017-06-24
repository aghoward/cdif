#include <any>
#include <functional>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include "cdif.h"

class RegistrarTests : public ::testing::Test {
    protected:
        cdif::Registrar _subject;
        cdif::Container _container;
        std::hash<std::thread::id> _hasher;

        std::string GivenRegistrationWithThreadUniqueName() {
            auto tid = GetTid();
            auto name = std::string("UniqueName") + tid;

            auto functor = [tid] (const cdif::Container &) { return tid; };
            _subject.Register<std::string>(functor, name);

            return name;
        }

        std::string GetTid() {
            return std::to_string(_hasher(std::this_thread::get_id()));
        }

    public:
        RegistrarTests() : _subject(cdif::Registrar()), _container(cdif::Container()) {
            
        };
};

TEST_F(RegistrarTests, GetRegistration_GivenNoMatch_ThrowsException) {
    auto name = std::string("Unregistered Name");
    auto exceptionThrown = false;

    try {
        _subject.GetRegistration<int>(name);
    } catch (const std::invalid_argument & ex) {
        exceptionThrown = true;
    }

    ASSERT_TRUE(exceptionThrown);
}

TEST_F(RegistrarTests, GetRegistration_GivenMatchingRegistration_ReturnsFunctor) {
    auto name = GivenRegistrationWithThreadUniqueName();
    auto expectedValue = GetTid();

    auto & registration = _subject.GetRegistration<std::string>(name);
    auto result = registration->Resolve<std::string>(_container);

    ASSERT_EQ(expectedValue, result);
}

TEST_F(RegistrarTests, Registrar_IsThreadSafeBetweenMultipleReadsAndWrites) {
    const auto threadCount = 100;
    auto functor = [&] (){
        auto tid = GetTid();
        auto name = GivenRegistrationWithThreadUniqueName();
        _subject.GetRegistration<std::string>(name);
    };

    auto threads = std::vector<std::thread>();

    for (auto i = 0; i < threadCount; i++)
        threads.push_back(std::thread(functor));

    for (auto & t : threads)
        t.join();
}
