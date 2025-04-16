#include <iostream>

#include "bot.hpp"
#include "crypto/base64.hpp"
#include "macros/unwrap.hpp"
#include "spawn/process.hpp"
#include "util/argument-parser.hpp"
#include "util/concat.hpp"
#include "util/span.hpp"
#include "zlib.hpp"

namespace {
constexpr auto server_send = 's';
constexpr auto client_send = 'c';
constexpr auto complete(const char header) -> char {
    return header + ('A' - 'a');
}

auto send_fragmented(Bot& bot, const std::span<const std::byte> data, const char header) -> bool {
    const auto text    = crypto::base64::encode(data);
    auto       payload = std::string_view(text);
    while(!payload.empty()) {
        // PRINT("fragment left={}", payload.size());
        const auto last     = payload.size() < 2000 - 1;
        const auto frag_len = last ? payload.size() : 2000 - 1;
        const auto frag     = payload.substr(0, frag_len);
        const auto data     = char(last ? complete(header) : header) + std::string(frag);
        ensure(bot.send(data));
        payload = payload.substr(frag_len);
    }
    return true;
}

struct RecvBuf {
    std::string buf;
    char        header;

    auto feed(const std::string& message) -> std::optional<std::vector<std::byte>> {
        if(message[0] == header) {
            // PRINT("feed {}bytes", message.size());
            buf += std::string_view(message).substr(1);
        } else if(message[0] == complete(header)) {
            // PRINT("final {}bytes", message.size());
            buf += std::string_view(message).substr(1);
            const auto bin = crypto::base64::decode(std::exchange(buf, {}));
            unwrap_mut(raw, inflate(bin.data(), bin.size()));
            return raw;
        }
        return std::nullopt;
    }

    RecvBuf(char header) : header(header) {};
};

auto server_main(Bot& bot) -> bool {
loop:
    auto process = process::Process();

    auto outputs = std::vector<std::byte>();
    auto handler = [&outputs](std::span<char> output) {
        // std::println("read {}bytes", output.size());
        outputs = concat(outputs, output);
    };
    process.on_stdout = handler;
    process.on_stderr = handler;

    auto recv_buf  = RecvBuf(client_send);
    bot.on_message = [&process, &recv_buf](const std::string& message) {
        if(const auto opt = recv_buf.feed(message)) {
            std::print("command={}", from_span(*opt));
            ensure_v(process.get_stdin().write(opt->data(), opt->size()));
        }
    };

    ensure(process.start({.argv = std::array{"/usr/bin/script", "-eq", "/dev/null", (const char*)NULL}, .die_on_parent_exit = true}));
    while(process.get_status() == process::Status::Running) {
        ensure(process.collect_outputs());
        unwrap(bin, deflate(outputs.data(), outputs.size(), 9));
        outputs.clear();
        ensure(send_fragmented(bot, bin, server_send));
    }
    unwrap(result, process.join());
    std::println("shell exited reason={} code={}", int(result.reason), result.code);
    goto loop;
}

auto client_main(Bot& bot) -> bool {
    auto recv_buf  = RecvBuf(server_send);
    bot.on_message = [&recv_buf](const std::string& message) {
        if(const auto opt = recv_buf.feed(message)) {
            write(fileno(stdout), opt->data(), opt->size());
            fflush(stdout);
        }
    };
    auto input = std::string();
    while(std::getline(std::cin, input)) {
        input += '\n';
        unwrap(bin, deflate(input.data(), input.size(), 9));
        ensure(send_fragmented(bot, bin, client_send));
    }
    return true;
}
} // namespace

auto main(const int argc, const char* const* argv) -> int {
    auto token   = (const char*)(nullptr);
    auto channel = uint64_t();
    auto server  = false;
    {
        auto parser = args::Parser<uint64_t>();
        auto help   = false;
        parser.kwarg(&channel, {"-c", "--channel"}, "CHANNEL_ID", "channel id");
        parser.kwarg(&token, {"-t", "--token"}, "TOKEN", "discord bot token");
        parser.kwflag(&server, {"-s", "--server"}, "act as a server");
        parser.kwflag(&help, {"-h", "--help"}, "print this help message");
        if(!parser.parse(argc, argv) || help) {
            std::println("usage: dsh {}", parser.get_help());
            return -1;
        }
    }

    auto bot       = Bot();
    bot.on_message = [](const std::string& message) {
        std::println("message {}", message);
    };
    ensure(bot.init(channel, token));
    std::println("connected");
    ensure(server ? server_main(bot) : client_main(bot));
    return 0;
}
