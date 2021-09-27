#pragma once

#include <bitset>

namespace ECS
{
    using FEntity = std::uint16_t;
    constexpr std::uint32_t MAX_ENTITIES = std::numeric_limits<uint16_t>::max();


    using FComponentType = std::uint8_t;
    const std::uint8_t MAX_COMPONENTS = 64;

    using FSignature = std::bitset<MAX_COMPONENTS>;

}