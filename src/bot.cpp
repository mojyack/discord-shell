#include <dpp/dpp.h>

#include "bot.hpp"
#include "macros/print.hpp"
#include "util/event.hpp"

auto Bot::init(const uint64_t channel_id, const std::string& token) -> bool {
    this->cluster    = new dpp::cluster(token, dpp::i_default_intents | dpp::i_message_content);
    this->channel_id = channel_id;

    cluster->on_message_create([this](dpp::message_create_t message) {
        auto& msg = message.msg;
        if(msg.channel_id != this->channel_id) {
            return;
        }
        if(on_message) {
            on_message(msg.content);
        }
    });

    auto ready = Event();
    cluster->on_ready([&ready](dpp::ready_t /*ready*/) {
        ready.notify();
    });
    cluster->start(dpp::st_return);
    ready.wait();

    return true;
}

auto Bot::send(std::string_view message) -> bool {
    auto event  = Event();
    auto result = false;
    cluster->message_create(dpp::message(channel_id, message), [&event, &result](dpp::confirmation_callback_t callback) {
        result = std::get_if<dpp::message>(&callback.value);
        if(!result) {
            WARN("{}", callback.get_error().human_readable);
        }
        event.notify();
    });
    event.wait();
    return result;
}

