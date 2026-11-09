#pragma once
#include <array>
#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>

#include "utils/circular_buffer.hpp"

namespace dsp
{
    template<typename S, typename T>
    using accumulator_type = decltype(std::declval<const S &>() * std::declval<const T &>());

    template<typename S, typename T>
    concept FirCompatible =
        std::semiregular<S> &&
        std::default_initializable<T> &&
        std::copy_constructible<T> &&
        requires (const S & sample, const T & tap, accumulator_type<S, T> acc)
        {
            requires std::default_initializable<accumulator_type<S, T>>;
            requires std::move_constructible<accumulator_type<S, T>>;
            acc += sample * tap;
        };

    template<typename S, typename T, std::size_t N>
    requires (N > 0) && FirCompatible<S, T>
    class FirFilter
    {
    public:
        using tap_value_type        = T;
        using tap_ref_type          = std::conditional_t<std::is_arithmetic_v<tap_value_type>, tap_value_type, tap_value_type&>;
        using tap_const_ref_type    = std::conditional_t<std::is_arithmetic_v<tap_value_type>, tap_value_type, const tap_value_type&>;

        using sample_value_type     = S;
        using sample_ref_type       = std::conditional_t<std::is_arithmetic_v<sample_value_type>, sample_value_type, sample_value_type&>;
        using sample_const_ref_type = std::conditional_t<std::is_arithmetic_v<sample_value_type>, sample_value_type, const sample_value_type&>;

        static constexpr std::size_t num_taps = N;

        constexpr explicit FirFilter(const std::array<tap_value_type, num_taps> & t) : taps(t) {}

        constexpr explicit FirFilter(std::array<tap_value_type, num_taps> && t) : taps(std::move(t)) {}

        constexpr void set_taps(const std::array<tap_value_type, num_taps> & incoming_taps)
        {
            taps = incoming_taps;
        }

        constexpr void set_taps(std::array<tap_value_type, num_taps> && incoming_taps)
        {
            taps = std::move(incoming_taps);
        }

        [[nodiscard]] constexpr const std::array<tap_value_type, num_taps> & get_taps() const noexcept
        {
            return taps;
        }

        constexpr accumulator_type<S, T> process(sample_const_ref_type sample)
        {
            accumulator_type<S, T> acc {};

            circular_buf.write(sample);

            for (std::size_t i = 0; i < num_taps ; ++i)
            {
                const auto & s = circular_buf.read(i).value().get();
                acc += s * taps[i];
            }

            return acc;
        }

        constexpr void reset() noexcept
        {
            circular_buf.clear();
        }

    private:
        std::array<tap_value_type, num_taps> taps {};
        utils::CircularBuffer<sample_value_type, num_taps> circular_buf {};
    };
}
