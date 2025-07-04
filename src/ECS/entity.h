#pragma once

#include <bitset>
#include <cstdint>
#include <limits>

namespace ECS
{
    /**
    * Entity is just a custom type for uint16. This also defines the maximum number of entities
    */
    using FEntity = std::uint32_t;

	constexpr FEntity INVALID_ENTITY = UINT32_MAX;

    /**
     * It also can be limited by any hardcoded value.
     */
    constexpr std::uint32_t MAX_ENTITIES = 2 * 1024 * 1024;

     /**
     * Component type is just a custom type for uint8
     */
    using FComponentType = std::uint8_t;

     /**
     * As MAX_ENTITIES, it also can be haedcoded by any other value
     */
    const std::uint8_t MAX_COMPONENTS = std::numeric_limits<FComponentType>::max();

    /**
     * Signature is a bitset, that stores information about the component's that attached to entity
     */
    using FSignature = std::bitset<MAX_COMPONENTS>;

}