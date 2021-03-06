#pragma once

#include "fast_ber/ber_types/StringImpl.hpp"
#include "fast_ber/ber_types/Tag.hpp"

namespace fast_ber
{
template <typename Identifier = ExplicitId<UniversalTag::universal_string>>
using UniversalString = fast_ber::StringImpl<UniversalTag::universal_string, Identifier>;
}
