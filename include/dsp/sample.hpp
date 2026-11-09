#pragma once

#include <complex>
#include <cstddef>
#include <ostream>
#include <type_traits>

namespace dsp
{
    namespace detail
    {
        template<typename T>
        struct is_complex : std::false_type {};

        template<typename T>
        struct is_complex<std::complex<T>> : std::true_type {};

        template<typename T>
        struct scalar_of { using type = T; };

        template<typename T>
        struct scalar_of<std::complex<T>> { using type = T; };
    }

    template<typename T>
    concept ComplexType = detail::is_complex<T>::value;

    template<typename T>
    class Sample
    {
    public:
        using raw_value_type        = T;
        using ref_type              = std::conditional_t<std::is_arithmetic_v<raw_value_type>, raw_value_type, raw_value_type&>;
        using const_ref_type        = std::conditional_t<std::is_arithmetic_v<raw_value_type>, raw_value_type, const raw_value_type&>;

        using value_type            = raw_value_type;
        using scalar_type           = typename detail::scalar_of<raw_value_type>::type;
        using scalar_ref_type       = std::conditional_t<std::is_arithmetic_v<scalar_type>, scalar_type, scalar_type&>;
        using scalar_const_ref_type = std::conditional_t<std::is_arithmetic_v<scalar_type>, scalar_type, const scalar_type&>;

        constexpr Sample() = default;

        constexpr explicit Sample(const_ref_type value)
                : _value(value)
        {}

        [[nodiscard]] constexpr Sample operator+(const Sample & other) const
        {
            return Sample{_value + other._value};
        }

        [[nodiscard]] constexpr Sample operator-(const Sample & other) const
        {
            return Sample{_value - other._value};
        }

        [[nodiscard]] constexpr Sample operator*(const Sample & other) const
        {
            return Sample{_value * other._value};
        }

        [[nodiscard]] constexpr Sample operator*(scalar_const_ref_type factor) const
        {
            return Sample{_value * factor};
        }

        [[nodiscard]] constexpr Sample operator/(const Sample & other) const
        {
            return Sample{_value / other._value};
        }

        [[nodiscard]] constexpr Sample operator/(scalar_const_ref_type factor) const
        {
            return Sample{_value / factor};
        }

        constexpr Sample & operator+=(const Sample & other)
        {
            _value += other._value;
            return *this;
        }

        constexpr Sample & operator-=(const Sample & other)
        {
            _value -= other._value;
            return *this;
        }

        constexpr Sample & operator*=(const Sample & other)
        {
            _value *= other._value;
            return *this;
        }

        constexpr Sample & operator/=(const Sample & other)
        {
            _value /= other._value;
            return *this;
        }

        constexpr Sample & operator*=(scalar_const_ref_type factor)
        {
            _value *= factor;
            return *this;
        }

        constexpr Sample & operator/=(scalar_const_ref_type factor)
        {
            _value /= factor;
            return *this;
        }

        [[nodiscard]] constexpr bool operator==(const Sample & other) const
        {
            return _value == other._value;
        }

        friend std::ostream & operator<<(std::ostream & os, const Sample<T> & s)
        {
            return os << s.value();
        }

        constexpr void rotate(scalar_type radians) requires ComplexType<T>
        {
            _value *= std::polar(scalar_type{1}, radians);
        }

        constexpr void rotate_by(const_ref_type phasor) requires ComplexType<T>
        {
            _value *= phasor;
        }

        [[nodiscard]] constexpr Sample conj() const requires ComplexType<T>
        {
            return Sample{std::conj(_value)};
        }

        [[nodiscard]] constexpr scalar_type magnitude() const
        {
            return std::abs(_value);
        }

        [[nodiscard]] constexpr scalar_type phase() const requires ComplexType<T>
        {
            return std::arg(_value);
        }

        [[nodiscard]] constexpr scalar_type norm() const
        {
            return std::norm(_value);
        }

        [[nodiscard]] constexpr const_ref_type value() const
        {
            return _value;
        }

        [[nodiscard]] constexpr ref_type value()
        {
            return _value;
        }

    private:
        raw_value_type _value{};
    };
}
