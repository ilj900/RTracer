#pragma once

#include <bitset>

namespace ECS
{
    /// Entity is just a custom type for uint16
    using FEntity = std::uint16_t;
    /// MAX_ENTITIES = std::numeric_limits<uint16_t>::max();
    constexpr std::uint32_t MAX_ENTITIES = std::numeric_limits<uint16_t>::max();

    /// Component type is just a custom type for uint8
    using FComponentType = std::uint8_t;
    /// MAX_COMPONENTS = std::numeric_limits<uint8_t>::max();
    const std::uint8_t MAX_COMPONENTS = std::numeric_limits<uint8_t>::max();

    /// Signature is a bitset, that stores information about the component's that attached to entity
    using FSignature = std::bitset<MAX_COMPONENTS>;

}