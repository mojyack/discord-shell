#pragma once
#include <functional>
#include <string_view>

namespace dpp {
struct cluster;
}

struct Bot {
    uint64_t      channel_id;
    dpp::cluster* cluster;

    std::function<void(std::string&)> on_message;

    auto init(uint64_t channel_id, const std::string& token) -> bool;
    auto send(std::string_view message) -> bool;
};

