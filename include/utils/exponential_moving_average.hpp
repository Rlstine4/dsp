#pragma once

#include <cstddef>
#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <tl/expected.hpp>

namespace utils
{

enum class EMAError : uint8_t
{
    UNKNOWN,
    INVALID_ALPHA_VALUE
};

template<typename T>
class ExponentialMovingAverage
{
    public:
        using value_type      = T;
        using const_value_type = const value_type;
        using ref_type        = std::conditional_t<std::is_arithmetic_v<value_type>, value_type, value_type&>;
        using const_ref_type  = std::conditional_t<std::is_arithmetic_v<value_type>, value_type, const_value_type&>;

        static constexpr double     DEFAULT_ALPHA_VALUE  = 1.0;
        static constexpr value_type DEFAULT_OUTPUT_VALUE = 0;

        ExponentialMovingAverage(const_value_type initial_value) :
            m_output(initial_value) {}

        double getAlpha() const
        {
            return m_alpha;
        }

        tl::expected<void, EMAError> setAlpha(const double alpha)
        {
            if (alpha <= 0.0 || alpha > 1.0)
            {
                return tl::unexpected{EMAError::INVALID_ALPHA_VALUE};
            }

            m_alpha = alpha;
        }

        const_ref_type update(const_ref_type in_data)
        {
            m_output += m_alpha * (in_data - m_output);

            return m_output;
        }

        void reset()
        {
            m_output = 0;
        }

    private:
        value_type m_output{DEFAULT_OUTPUT_VALUE};
        double     m_alpha {DEFAULT_ALPHA_VALUE};
};
}
