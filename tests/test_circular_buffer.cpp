#include <catch2/catch.hpp>

#include "dsp/sample.hpp"
#include "utils/circular_buffer.hpp"

#include <array>
#include <complex>
#include <cstddef>
#include <limits>
#include <string>
#include <tl/expected.hpp>
#include <type_traits>
#include <utility>

namespace
{
using utils::CircularBuffer;
using utils::CircularBufferError;
constexpr std::size_t DEPTH = 4;
using TestBuffer = CircularBuffer<int, DEPTH>;

template <typename Result>
void require_out_of_bounds_error(const Result& result)
{
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() == CircularBufferError::OUT_OF_BOUNDS);
}

template <typename Buffer, typename T, std::size_t N>
void require_history_matches(const Buffer & buf, const std::array<T, N> & expected)
{
    for (std::size_t delay = 0; delay < N; ++delay)
    {
        CAPTURE(delay);
        const auto result = buf.read(delay);
        REQUIRE(result.has_value());
        REQUIRE(result.value().get() == expected[delay]);
    }
}

constexpr bool compile_time_smoke_test()
{
    TestBuffer buf;
    buf.write(1);
    buf.write(2);
    buf.write(3);

    bool is_ok = buf.read(0).value().get() == 3 && buf.read(1).value().get() == 2
           && buf.read(2).value().get() == 1
           && buf.read(4) == tl::unexpected(CircularBufferError::OUT_OF_BOUNDS)
           && buf[0].value().get() == 1
           && buf[4] == tl::unexpected(CircularBufferError::OUT_OF_BOUNDS);

    buf.write(4);
    is_ok = is_ok && buf.read(0).value().get() == 4;

    buf.write(5);  // wraps to slot 0 and overwrites the oldest value
    is_ok = is_ok && buf.read(0).value().get() == 5 && buf.read(3).value().get() == 2 && buf[0].value().get() == 5;

    buf.clear();
    return is_ok && buf.read(0).value().get() == 0;
}
}  // namespace

// ---------------------------------------------------------------------------

TEST_CASE("default construction yields a zeroed buffer", "[circular_buffer]")
{
    constexpr TestBuffer buf;
    require_history_matches(buf, std::array<int, DEPTH>{});
}

TEST_CASE("read returns the newest value first", "[circular_buffer]")
{
    TestBuffer buf;
    buf.write(1);
    buf.write(2);
    buf.write(3);

    REQUIRE(buf.read(0).value().get() == 3);
    REQUIRE(buf.read(1).value().get() == 2);
    REQUIRE(buf.read(2).value().get() == 1);
    REQUIRE(buf.read(3).value().get() == 0);
}

TEST_CASE("read exposes a const reference to the stored value", "[circular_buffer]")
{
    static_assert(std::is_same_v<
        decltype(std::declval<const TestBuffer &>().read(0).value().get()),
        const int &>);
    static_assert(std::is_same_v<
        decltype(std::declval<const TestBuffer &>()[0].value().get()),
        const int &>);

    CircularBuffer<std::string, 1> buf;
    buf.write("before");

    const auto & stored_value = buf.read(0).value().get();
    REQUIRE(stored_value == "before");

    buf.write("after");
    REQUIRE(stored_value == "after");
}

TEST_CASE("history matches a reference model for any number of writes", "[circular_buffer]")
{
    constexpr int depth = static_cast<int>(DEPTH);

    // Covers empty, partially filled, exactly full, wrapped, and multi-lap.
    const int writes = GENERATE(range(0, 3 * depth + 2));
    CAPTURE(writes);

    TestBuffer buf;
    for (int i = 1; i <= writes; ++i)
        buf.write(i);

    for (int delay = 0; delay < depth; ++delay)
    {
        CAPTURE(delay);
        const int expected = delay < writes ? writes - delay : 0;  // unwritten slots stay 0

        const auto result = buf.read(static_cast<std::size_t>(delay));
        REQUIRE(result.has_value());
        REQUIRE(result.value().get() == expected);
    }
}

TEST_CASE("read and operator[] reject out-of-range arguments", "[circular_buffer]")
{
    const TestBuffer buf;

    const std::size_t idx = GENERATE(TestBuffer::buffer_depth,
                                     TestBuffer::buffer_depth + 1,
                                     std::size_t{999},
                                     std::numeric_limits<std::size_t>::max());
    CAPTURE(idx);

    require_out_of_bounds_error(buf.read(idx));
    require_out_of_bounds_error(buf[idx]);
}

TEST_CASE("operator[] indexes physical slots, independent of head", "[circular_buffer]")
{
    TestBuffer buf;
    buf.write(100);
    buf.write(200);
    buf.write(300);

    REQUIRE(buf[0].value().get() == 100);
    REQUIRE(buf[1].value().get() == 200);
    REQUIRE(buf[2].value().get() == 300);
    REQUIRE(buf[3].value().get() == 0);  // largest valid index, never written

    buf.write(400);  // fills slot 3
    buf.write(500);  // wraps and overwrites slot 0; head moved, slot numbering did not

    REQUIRE(buf[0].value().get() == 500);
    REQUIRE(buf[1].value().get() == 200);
    REQUIRE(buf[3].value().get() == 400);
}

TEST_CASE("clear resets every slot to its default and resets the write position", "[circular_buffer]")
{
    TestBuffer buf;
    // DEPTH + 2 writes leaves head mid-buffer, so the reset is observable.
    for (int i = 1; i <= static_cast<int>(DEPTH) + 2; ++i)
        buf.write(i);

    buf.clear();
    require_history_matches(buf, std::array<int, DEPTH>{});

    buf.write(42);
    REQUIRE(buf.read(0).value().get() == 42);
    REQUIRE(buf[0].value().get() == 42);  // writing restarts at slot 0
}

TEST_CASE("construction from initial values populates every slot", "[circular_buffer]")
{
    const std::array<int, DEPTH> initial_values{1, 2, 3, 4};
    const TestBuffer buf(initial_values);
    const TestBuffer rvalue_buf(std::array<int, DEPTH>{1, 2, 3, 4});

    for (std::size_t i = 0; i < DEPTH; ++i)
        REQUIRE(buf[i].value().get() == static_cast<int>(i) + 1);

    // Head starts at 0, so the newest sample is the last element.
    require_history_matches(buf, std::array{4, 3, 2, 1});
    require_history_matches(rvalue_buf, std::array{4, 3, 2, 1});
}

TEST_CASE("works with non-arithmetic value types", "[circular_buffer][string]")
{
    CircularBuffer<std::string, 3> buf;
    buf.write("alpha");
    buf.write("beta");
    buf.write("gamma");
    require_history_matches(buf, std::array<std::string, 3>{"gamma", "beta", "alpha"});

    buf.write("delta");  // overwrites a slot that already holds a string
    require_history_matches(buf, std::array<std::string, 3>{"delta", "gamma", "beta"});

    require_out_of_bounds_error(buf.read(3));
}

TEST_CASE("works with complex values", "[circular_buffer][complex]")
{
    using Complex = std::complex<double>;
    CircularBuffer<Complex, 3> buf;

    buf.write(Complex{1.0, 2.0});
    buf.write(Complex{3.0, 4.0});
    buf.write(Complex{5.0, 6.0});
    require_history_matches(buf, std::array{Complex{5.0, 6.0}, Complex{3.0, 4.0}, Complex{1.0, 2.0}});

    buf.write(Complex{7.0, 8.0});
    require_history_matches(buf, std::array{Complex{7.0, 8.0}, Complex{5.0, 6.0}, Complex{3.0, 4.0}});
}

TEST_CASE("works with DSP sample values", "[circular_buffer][sample]")
{
    using Sample = dsp::Sample<std::complex<double>>;
    CircularBuffer<Sample, 3> buf;

    buf.write(Sample{{1.0, 2.0}});
    buf.write(Sample{{3.0, 4.0}});
    buf.write(Sample{{5.0, 6.0}});
    require_history_matches(buf, std::array{Sample{{5.0, 6.0}}, Sample{{3.0, 4.0}}, Sample{{1.0, 2.0}}});

    buf.write(Sample{{7.0, 8.0}});
    require_history_matches(buf, std::array{Sample{{7.0, 8.0}}, Sample{{5.0, 6.0}}, Sample{{3.0, 4.0}}});
}

TEST_CASE("operations are usable in constant expressions", "[circular_buffer][constexpr]")
{
    STATIC_REQUIRE(compile_time_smoke_test());
}
