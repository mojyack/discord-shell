#pragma once
#include <optional>
#include <vector>

auto deflate(const void* ptr, size_t size, int level) -> std::optional<std::vector<std::byte>>;
auto inflate(const void* ptr, size_t size) -> std::optional<std::vector<std::byte>>;
