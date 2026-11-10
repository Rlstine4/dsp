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

    // S is the sample type (stored in the history buffer) and T the tap type.
    // T must be assignable for set_taps, and sample * tap must accumulate via +=.
    template<typename S, typename T>
    concept FirCompatible =
        std::semiregular<S> &&                                 // stored in CircularBuffer<S, N> (copyable and default-constructible)
        std::default_initializable<T> &&                       // std::array<T, N> taps {}
        std::copy_constructible<T> &&                          // built/set from an lvalue array
        std::assignable_from<T &, const T &> &&                // set_taps(const &) (can we assign const T & to T&)
        std::assignable_from<T &, T &&> &&                     // set_taps(&&) (std::move)
        std::default_initializable<accumulator_type<S, T>> &&  // accumulator_type acc {}
        std::move_constructible<accumulator_type<S, T>> &&     // return acc from process()
        requires (const S & sample, const T & tap, accumulator_type<S, T> acc)
        {
            acc += sample * tap;
        };

    template<typename S, typename T, size_t N>
    requires (N > 0) && FirCompatible<S, T>
    class FirFilter
    {
    public:
        using tap_value_type       = T;
        using const_tap_value_type = const tap_value_type;
        using tap_ref_type         = std::conditional_t<std::is_arithmetic_v<tap_value_type>, tap_value_type, tap_value_type&>;
        using tap_const_ref_type   = std::conditional_t<std::is_arithmetic_v<tap_value_type>, const_tap_value_type, const_tap_value_type&>;

        using sample_value_type       = S;
        using const_sample_value_type = const sample_value_type;
        using sample_ref_type         = std::conditional_t<std::is_arithmetic_v<sample_value_type>, sample_value_type, sample_value_type&>;
        using sample_const_ref_type   = std::conditional_t<std::is_arithmetic_v<sample_value_type>, const_sample_value_type, const_sample_value_type&>;

        static constexpr size_t num_taps = N;

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

        [[nodiscard]] constexpr accumulator_type<S, T> process(sample_const_ref_type sample)
        {
            accumulator_type<S, T> acc {};

            circular_buf.write(sample);

            for (size_t i = 0; i < num_taps ; ++i)
            {
                const auto & s = circular_buf.read(i).value().get();
                acc += s * taps[i];
            }

            return acc;
        }

        constexpr void reset()
        {
            circular_buf.clear();
        }

    private:
        std::array<tap_value_type, num_taps> taps {};
        utils::CircularBuffer<sample_value_type, num_taps> circular_buf {};
    };
}
