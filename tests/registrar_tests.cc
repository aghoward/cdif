#include <any>
#include <functional>
#include <stdexcept>
#include <string>

#include <gtest/gtest.h>

#include "cdif.h"

class RegistrarTests : public ::testing::Test {
    protected:
        cdif::Registrar _subject;
        cdif::Container _container;

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
};
