/**
 * @file Serialization.hpp
 * @brief Bitsery reader/writer types shared by every component plugin
 *
 * @author seb
 * @date 2026-10-29
**/

#ifndef SERIALIZATION_HPP_
    #define SERIALIZATION_HPP_

#include <cstdint>
#include <cstddef>
#include <bitsery/bitsery.h>
#include <bitsery/adapter/stream.h>
#include <bitsery/traits/string.h>
#include <bitsery/traits/vector.h>

namespace OrByte {

    using Serializer = bitsery::Serializer<bitsery::OutputBufferedStreamAdapter>;
    using Deserializer = bitsery::Deserializer<bitsery::InputStreamAdapter>;

    inline constexpr std::size_t MaxStringSize = 1024;
    inline constexpr std::size_t MaxContainerSize = 4096;

} // namespace OrByte

#endif /* SERIALIZATION_HPP_ */
