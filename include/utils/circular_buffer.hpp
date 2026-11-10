#pragma once

#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <tl/expected.hpp>
#include <type_traits>
#include <utility>

namespace utils
{
enum class CircularBufferError : uint8_t
{
    UNKNOWN,
    OUT_OF_BOUNDS,
};

template<typename T, size_t N>
requires (N > 0) && std::semiregular<T>
class CircularBuffer
{
    public:
        using value_type       = T;
        using const_value_type = const value_type;
        using ref_type         = std::conditional_t<std::is_arithmetic_v<value_type>, value_type, value_type&>;
        using const_ref_type   = std::conditional_t<std::is_arithmetic_v<value_type>, const_value_type, const_value_type&>;
        using read_result_type = tl::expected<std::reference_wrapper<const_value_type>, CircularBufferError>;

        static constexpr size_t buffer_depth = N;

        constexpr CircularBuffer() = default;

        constexpr explicit CircularBuffer(const std::array<value_type, buffer_depth>& init_values) :
            buffer(init_values)
        {}

        constexpr explicit CircularBuffer(std::array<value_type, buffer_depth>&& init_values) :
            buffer(std::move(init_values))
        {}

        [[nodiscard]] constexpr read_result_type read(const size_t delay) const
        {
            if (delay >= buffer_depth)
            {
                return tl::unexpected{CircularBufferError::OUT_OF_BOUNDS};
            }

            const size_t idx = (head + buffer_depth - 1 - delay) % buffer_depth;

            return std::cref(buffer[idx]);
        }

        [[nodiscard]] constexpr read_result_type operator[](const size_t idx) const
        {
            if (idx >= buffer_depth)
            {
                return tl::unexpected{CircularBufferError::OUT_OF_BOUNDS};
            }

            return std::cref(buffer[idx]);
        }

        constexpr void write(const_ref_type data)
        {
            buffer[head] = data;
            head         = (head == buffer_depth-1) ? 0 : head + 1;
        }

        constexpr void clear()
        {
            head   = 0;
            buffer = {};
        }

    private:
        std::array<value_type, buffer_depth> buffer {};
        size_t head = 0;
};
}
