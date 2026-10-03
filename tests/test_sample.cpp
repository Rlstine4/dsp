#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include "sample.hpp"

#include <cmath>
#include <complex>

TEST_CASE("Sample arithmetic with double", "[sample]")
{
    Sample<double> a(2.0);
    Sample<double> b(3.0);

    SECTION("addition and subtraction compile and run")
    {
        // Operators return Sample; exercise them so regressions fail to compile/run
        Sample<double> sum = a + b;
        Sample<double> diff = b - a;
        (void)sum;
        (void)diff;
        SUCCEED();
    }
}

TEST_CASE("Sample rotate with complex", "[sample]")
{
    using c = std::complex<double>;
    Sample<c> s(c(1.0, 0.0));

    SECTION("rotate by radians")
    {
        s.rotate(std::acos(-1.0)); // pi -> (-1, ~0)
        SUCCEED();
    }

    SECTION("rotate by phasor")
    {
        s.rotate(c(0.0, 1.0));
        SUCCEED();
    }
}
