#pragma once

#include <bsoncxx/oid.hpp>
#include <algorithm>
#include <optional>
#include <string>
#include <string_view>

namespace Infrastructure::Database {
// HTTP/domain IDs are hex strings; MongoDB identities and references are BSON ObjectIds.
inline std::optional<bsoncxx::oid> parse_object_id(std::string_view value) {
    if (value.size() != 24 || !std::all_of(value.begin(), value.end(), [](unsigned char c) {
            return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
        }))
        return std::nullopt;
    return bsoncxx::oid(std::string(value));
}
} // namespace Infrastructure::Database
