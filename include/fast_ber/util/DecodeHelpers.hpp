#pragma once

#include "absl/types/span.h"
#include "fast_ber/ber_types/Identifier.hpp"
#include "fast_ber/util/BerView.hpp"

namespace fast_ber
{

struct DecodeResult
{
    bool success;
};

template <typename T, Class T2, Tag T3>
DecodeResult decode_impl(BerViewIterator& input, T& output, Id<T2, T3> id) noexcept
{
    if (!input->is_valid() || id != input->identifier())
    {
        return DecodeResult{false};
    }

    bool success = output.assign_ber(*input) > 0;

    ++input;
    return DecodeResult{success};
}

template <typename T, typename OuterId, typename InnerId>
DecodeResult decode_impl(BerViewIterator& input, T& output, DoubleId<OuterId, InnerId> id) noexcept
{
    if (!input->is_valid() || input->identifier() != OuterId{})
    {
        return DecodeResult{false};
    }

    BerView         inner = input->content();
    BerViewIterator iter  = input->content();
    if (!decode_impl(iter, output, id.inner_id()).success)
    {
        return DecodeResult{false};
    }

    bool success = output.assign_ber(inner) > 0;
    ++input;
    return DecodeResult{success};
}

template <typename T>
DecodeResult decode(absl::Span<const uint8_t> input, T& output) noexcept
{
    BerViewIterator iter(input);
    if (!iter->is_valid())
    {
        return DecodeResult{false};
    }

    return decode(iter, output);
}

template <Class class_, Tag tag>
bool has_correct_header(BerView input, Id<class_, tag>, Construction construction)
{
    return input.is_valid() && input.class_() == class_ && input.tag() == tag && input.construction() == construction;
}

template <typename Identifier1, typename Identifier2>
bool has_correct_header(BerView input, DoubleId<Identifier1, Identifier2>, Construction construction)
{
    return input.is_valid() && input.identifier() == Identifier1{} &&
           input.construction() == Construction::constructed && input.begin()->is_valid() &&
           input.begin()->identifier() == Identifier2{} && input.begin()->construction() == construction;
}

} // namespace fast_ber
