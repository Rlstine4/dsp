#include <catch2/catch.hpp>

#include "dsp/fir.hpp"
#include "dsp/sample.hpp"

#include <array>
#include <complex>
#include <type_traits>
#include <utility>

using Complex = std::complex<double>;
using RealSample = dsp::Sample<double>;
using ComplexSample = dsp::Sample<Complex>;

using ComplexRealTapFilter = dsp::FirFilter<Complex, double, 2>;
using RealSampleFilter = dsp::FirFilter<RealSample, double, 2>;
using ComplexSampleFilter = dsp::FirFilter<ComplexSample, double, 2>;

static_assert(dsp::FirCompatible<double, double>);
static_assert(dsp::FirCompatible<Complex, double>);
static_assert(dsp::FirCompatible<RealSample, double>);
static_assert(dsp::FirCompatible<ComplexSample, double>);
static_assert(dsp::FirCompatible<ComplexSample, ComplexSample>);
static_assert(dsp::FirCompatible<double, Complex>);

constexpr bool compile_time_smoke_test()
{
    dsp::FirFilter<int, int, 2> filter(std::array<int, 2>{2, -1});

    bool is_ok = filter.process(1) == 2;
    is_ok = is_ok && filter.process(0) == -1;

    filter.reset();
    return is_ok && filter.process(3) == 6;
}

static_assert(compile_time_smoke_test());

TEST_CASE("an asymmetric impulse response verifies tap order and zero initial history", "[fir]")
{
    dsp::FirFilter<int, int, 3> filter(std::array<int, 3>{1, 2, 3});

    REQUIRE(filter.process(1) == 1);
    REQUIRE(filter.process(0) == 2);
    REQUIRE(filter.process(0) == 3);
    REQUIRE(filter.process(0) == 0);
    REQUIRE(filter.process(0) == 0);
}

TEST_CASE("sample history wraps without retaining samples older than the tap count", "[fir]")
{
    dsp::FirFilter<int, int, 3> filter(std::array<int, 3>{1, 10, 100});

    REQUIRE(filter.process(1) == 1);
    REQUIRE(filter.process(2) == 12);
    REQUIRE(filter.process(3) == 123);
    REQUIRE(filter.process(4) == 234);
    REQUIRE(filter.process(5) == 345);
}

TEST_CASE("reset clears sample history without changing the taps", "[fir][reset]")
{
    const std::array<int, 3> taps{2, -1, 3};
    dsp::FirFilter<int, int, 3> filter(taps);
    dsp::FirFilter<int, int, 3> fresh_filter(taps);

    filter.process(4);
    filter.process(-2);
    filter.process(5);
    filter.reset();

    for (const int sample : {0, 1, 2, 3, 4})
    {
        CAPTURE(sample);
        REQUIRE(filter.process(sample) == fresh_filter.process(sample));
    }
}

TEST_CASE("taps can be moved from an rvalue array", "[fir]")
{
    std::array<int, 3> taps{1, 2, 3};
    dsp::FirFilter<int, int, 3> filter(std::move(taps));

    REQUIRE(filter.process(1) == 1);
    REQUIRE(filter.process(0) == 2);
    REQUIRE(filter.process(0) == 3);
}

TEST_CASE("taps can be copied from an lvalue array without clearing sample history", "[fir][set_taps]")
{
    dsp::FirFilter<int, int, 2> filter(std::array<int, 2>{1, 0});
    REQUIRE(filter.process(2) == 2);
    REQUIRE(filter.process(5) == 5);

    const std::array<int, 2> new_taps{10, 100};
    filter.set_taps(new_taps);

    REQUIRE(filter.get_taps() == new_taps);
    REQUIRE(filter.process(1) == 510);
    REQUIRE(filter.process(0) == 100);
}

TEST_CASE("taps can be moved from an rvalue array and survive reset", "[fir][set_taps][reset]")
{
    dsp::FirFilter<int, int, 2> filter(std::array<int, 2>{1, 1});
    REQUIRE(filter.process(2) == 2);

    std::array<int, 2> new_taps{2, -1};
    filter.set_taps(std::move(new_taps));

    REQUIRE((filter.get_taps() == std::array<int, 2>{2, -1}));
    REQUIRE(filter.process(3) == 4);

    filter.reset();
    REQUIRE(filter.process(3) == 6);
    REQUIRE(filter.process(4) == 5);
}

TEST_CASE("complex samples support real taps and preserve both components", "[fir][complex]")
{
    ComplexRealTapFilter filter(std::array<double, 2>{2.0, -0.5});
    const Complex first{1.0, 2.0};
    const Complex second{3.0, -4.0};

    REQUIRE((filter.process(first) == Complex{2.0, 4.0}));
    REQUIRE((filter.process(second) == Complex{5.5, -9.0}));
    REQUIRE((filter.process(Complex{}) == Complex{-1.5, 2.0}));
}

TEST_CASE("real Sample values support real taps", "[fir][sample]")
{
    RealSampleFilter filter(std::array<double, 2>{0.5, 2.0});

    REQUIRE(filter.process(RealSample{4.0}).value() == 2.0);
    REQUIRE(filter.process(RealSample{1.0}).value() == 8.5);
}

TEST_CASE("complex Sample values support real taps", "[fir][sample][complex]")
{
    ComplexSampleFilter filter(std::array<double, 2>{2.0, -0.5});
    const ComplexSample first{{1.0, 2.0}};
    const ComplexSample second{{3.0, -4.0}};

    REQUIRE((filter.process(first).value() == Complex{2.0, 4.0}));
    REQUIRE((filter.process(second).value() == Complex{5.5, -9.0}));
    REQUIRE((filter.process(ComplexSample{}).value() == Complex{-1.5, 2.0}));
}

TEST_CASE("complex Sample values support complex Sample taps", "[fir][sample][complex]")
{
    dsp::FirFilter<ComplexSample, ComplexSample, 1> filter(
        std::array<ComplexSample, 1>{ComplexSample{{0.0, 1.0}}});

    REQUIRE((filter.process(ComplexSample{{2.0, 3.0}}).value() == Complex{-3.0, 2.0}));
}

TEST_CASE("real samples support complex taps and produce complex output", "[fir][complex]")
{
    dsp::FirFilter<double, Complex, 1> filter(
        std::array<Complex, 1>{Complex{0.0, 2.0}});

    REQUIRE((filter.process(3.0) == Complex{0.0, 6.0}));
}

TEST_CASE("processing and reset are usable in constant expressions", "[fir][constexpr]")
{
    STATIC_REQUIRE(compile_time_smoke_test());
}
