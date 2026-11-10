#include <catch2/catch.hpp>

#include "dsp/sample.hpp"

#include <complex>
#include <numbers>
#include <sstream>

namespace
{
using RealSample    = dsp::Sample<double>;
using ComplexSample = dsp::Sample<std::complex<double>>;

constexpr bool compile_time_smoke_test()
{
    const RealSample a{2.0};
    const RealSample b{3.0};

    return (a + b).value() == 5.0 &&
           (b - a).value() == 1.0 &&
           (a * b).value() == 6.0 &&
           (b / a).value() == 1.5 &&
           (a * 2.0).value() == 4.0 &&
           (2.0 * a).value() == 4.0 &&
           (b / 2.0).value() == 1.5 &&
           a.norm() == 4.0;
}

static_assert(compile_time_smoke_test());
}

TEST_CASE("Sample default construction and value access", "[sample]")
{
    const RealSample real;
    REQUIRE(real.value() == Approx(0.0));

    ComplexSample complex;
    REQUIRE(complex.value() == std::complex<double>{});

    complex.value() = {1.0, 2.0};
    REQUIRE(complex.value().real() == Approx(1.0));
    REQUIRE(complex.value().imag() == Approx(2.0));
}

TEST_CASE("Sample arithmetic", "[sample]")
{
    const RealSample a{2.0};
    const RealSample b{3.0};

    REQUIRE((a + b).value() == Approx(5.0));
    REQUIRE((b - a).value() == Approx(1.0));
    REQUIRE((a * b).value() == Approx(6.0));
    REQUIRE((b / a).value() == Approx(1.5));
    REQUIRE((a * 2.5).value() == Approx(5.0));
    REQUIRE((2.5 * a).value() == Approx(5.0));
    REQUIRE((b / 2.0).value() == Approx(1.5));
}

TEST_CASE("Sample compound arithmetic", "[sample]")
{
    RealSample value{2.0};
    value += RealSample{3.0};
    REQUIRE(value.value() == Approx(5.0));

    value = RealSample{2.0};
    value -= RealSample{3.0};
    REQUIRE(value.value() == Approx(-1.0));

    value = RealSample{2.0};
    value *= RealSample{3.0};
    REQUIRE(value.value() == Approx(6.0));

    value = RealSample{6.0};
    value /= RealSample{3.0};
    REQUIRE(value.value() == Approx(2.0));

    value = RealSample{2.0};
    value *= 2.5;
    REQUIRE(value.value() == Approx(5.0));

    value = RealSample{6.0};
    value /= 2.0;
    REQUIRE(value.value() == Approx(3.0));
}

TEST_CASE("Sample equality and stream output", "[sample]")
{
    REQUIRE(RealSample{2.0} == RealSample{2.0});
    REQUIRE_FALSE(RealSample{2.0} == RealSample{3.0});

    std::ostringstream stream;
    stream << RealSample{2.5};
    REQUIRE(stream.str() == "2.5");
}

TEST_CASE("Sample magnitude and norm", "[sample]")
{
    REQUIRE(RealSample{-3.0}.magnitude() == Approx(3.0));
    REQUIRE(RealSample{-3.0}.norm() == Approx(9.0));

    const ComplexSample value{std::complex<double>{3.0, 4.0}};
    REQUIRE(value.magnitude() == Approx(5.0));
    REQUIRE(value.norm() == Approx(25.0));
}

TEST_CASE("Sample complex phase", "[sample][complex]")
{
    using c = std::complex<double>;

    REQUIRE(ComplexSample{c{0.0, 1.0}}.phase() == Approx(std::numbers::pi / 2.0));
    REQUIRE(ComplexSample{c{0.0, -1.0}}.phase() == Approx(-std::numbers::pi / 2.0));
}

TEST_CASE("Sample complex arithmetic and conjugation", "[sample][complex]")
{
    using c = std::complex<double>;

    const ComplexSample a{c{1.0, 2.0}};
    const ComplexSample b{c{3.0, 4.0}};
    const auto product = a * b;

    REQUIRE(product.value().real() == Approx(-5.0));
    REQUIRE(product.value().imag() == Approx(10.0));

    const auto scaled = a * 2.0;
    REQUIRE(scaled.value().real() == Approx(2.0));
    REQUIRE(scaled.value().imag() == Approx(4.0));

    const auto left_scaled = 2.0 * a;
    REQUIRE(left_scaled.value().real() == Approx(2.0));
    REQUIRE(left_scaled.value().imag() == Approx(4.0));

    const auto conjugated = a.conj();
    REQUIRE(conjugated.value().real() == Approx(1.0));
    REQUIRE(conjugated.value().imag() == Approx(-2.0));
    REQUIRE(a.value() == c{1.0, 2.0});
}

TEST_CASE("Sample complex rotation", "[sample][complex]")
{
    using c = std::complex<double>;

    SECTION("rotate by radians")
    {
        ComplexSample value{c{1.0, 0.0}};
        value.rotate(std::numbers::pi / 2.0);

        REQUIRE(value.value().real() == Approx(0.0).margin(1e-12));
        REQUIRE(value.value().imag() == Approx(1.0));
    }

    SECTION("rotate by phasor")
    {
        ComplexSample value{c{1.0, 0.0}};
        value.rotate_by(c{0.0, 1.0});

        REQUIRE(value.value().real() == Approx(0.0));
        REQUIRE(value.value().imag() == Approx(1.0));
    }
}
