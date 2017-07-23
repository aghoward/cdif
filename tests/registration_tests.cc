#include <any>
#include <functional>

#include <gtest/gtest.h>

#include "cdif.h"

class RegistrationTests : public ::testing::Test {
    protected:
        cdif::Container _container;
        double _returnedValue = 3.14;

        std::function<double (const cdif::Container &)> _resolver;

    public:
        RegistrationTests() {
            _container = cdif::Container();
            _resolver = [&] (const cdif::Container &) { return _returnedValue; };
        }
};

TEST_F(RegistrationTests, Resolve_ProperlyCastsValueBackToOriginal) {
    auto subject = cdif::Registration(_resolver);
    auto actual = subject.Resolve<double>(_container);

    ASSERT_EQ(actual, _returnedValue) << "Value not properly casted to original";
}
