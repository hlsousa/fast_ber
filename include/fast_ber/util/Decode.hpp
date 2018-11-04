#pragma once

#include "absl/types/span.h"
#include "fast_ber/ber_types/Class.hpp"
#include "fast_ber/ber_types/Construction.hpp"
#include "fast_ber/util/BerView.hpp"
#include "fast_ber/util/DecodePrimitive.hpp"

#include <iostream>

namespace fast_ber
{

template <typename T>
int val(T t) noexcept
{
    return static_cast<int>(t);
}

inline bool decode_combine_impl(BerViewIterator&) noexcept { return true; }

template <typename... Args, typename T>
bool decode_combine_impl(BerViewIterator& input, T& object, Tag tag, Args&&... args) noexcept
{
    bool success = decode(input, object, tag);
    if (!success)
    {
        return false;
    }
    return decode_combine_impl(input, args...);
}

template <typename... Args>
bool decode_combine(const BerView& input, Tag tag, Args&&... args) noexcept
{
    if (!input.is_valid())
    {
        return false;
    }

    auto iterator = input.begin();
    return decode_combine_impl(iterator, args...);
}

} // namespace fast_ber
