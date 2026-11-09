#pragma once

#include <array>
#include <concepts>
#include <cstddef>
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

template<typename T, std::size_t N>
requires (N > 0) && std::semiregular<T>
class CircularBuffer
{
    public:
        using value_type      = T;
        using const_value_type = const value_type;
        using ref_type        = std::conditional_t<std::is_arithmetic_v<value_type>, value_type, value_type&>;
        using const_ref_type  = std::conditional_t<std::is_arithmetic_v<value_type>, value_type, const_value_type&>;

        static constexpr std::size_t buffer_depth = N;

        constexpr explicit CircularBuffer() = default;

        constexpr explicit CircularBuffer(const std::array<value_type, buffer_depth>& init_values) :
            buffer(init_values)
        {}

        constexpr explicit CircularBuffer(std::array<value_type, buffer_depth>&& init_values) :
            buffer(std::move(init_values))
        {}

        constexpr bool operator==(const CircularBuffer & other) const
            requires std::equality_comparable<T>
        {
            for (std::size_t i = 0; i < buffer_depth; ++i)
            {
                if (this->read(i).value().get() != other.read(i).value().get())
                {
                    return false;
                }
            }

            return true;
        }

        [[nodiscard]] constexpr tl::expected<std::reference_wrapper<const value_type>, CircularBufferError> read(const size_t delay) const
        {
            if (delay >= buffer_depth)
            {
                return tl::unexpected{CircularBufferError::OUT_OF_BOUNDS};
            }

            const size_t idx = (head + buffer_depth - 1 - delay) % buffer_depth;

            return std::cref(buffer[idx]);
        }

        constexpr tl::expected<value_type, CircularBufferError> operator[](const size_t idx) const
        {
            if (idx >= buffer_depth)
            {
                return tl::unexpected{CircularBufferError::OUT_OF_BOUNDS};
            }

            return buffer[idx];
        }

        constexpr void write(const value_type & data)
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
        std::array<value_type,buffer_depth> buffer {};
        std::size_t head   = 0;
};
}
