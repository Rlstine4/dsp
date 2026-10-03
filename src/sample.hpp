#pragma once

#include <complex>
#include <cstddef>
#include <type_traits>

namespace detail
{
    template<typename T>
    struct is_complex : std::false_type {};

    template<typename T>
    struct is_complex<std::complex<T>> : std::true_type {};
}

template<typename T>
concept ComplexType = detail::is_complex<T>::value;

template<typename T>
class Sample
{
public:
    Sample(const T & value) :
            _value(value) {}

    Sample operator+(const Sample & other) const
    {
        return _value + other._value;
    }

    Sample operator-(const Sample & other) const
    {
        return _value - other._value;
    }

    Sample operator*(const Sample & other) const
    {
        return _value * other._value;
    }

    Sample operator*(const size_t factor) const
    {
        return _value * factor;
    }

    Sample operator/(const Sample & other) const
    {
        return _value / other._value;
    }

    Sample operator/(const size_t factor) const
    {
        return _value / factor;
    }

    template<ComplexType U = T>
    void rotate(typename U::value_type radians)
    {
        _value *= std::polar(typename U::value_type(1), radians);
    }

    template<ComplexType U = T>
    void rotate(const U & phasor)
    {
        _value *= phasor;
    }

private:
    T _value;
};