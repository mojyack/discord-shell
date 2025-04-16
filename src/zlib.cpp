#include <vector>

#include <zlib.h>

#include "macros/assert.hpp"

auto deflate(const void* const ptr, const size_t size, const int level) -> std::optional<std::vector<std::byte>> {
    auto ret = std::vector<std::byte>();
    auto z   = z_stream();

    ensure(deflateInit(&z, level) == Z_OK);
    z.avail_in = size;
    z.next_in  = (Bytef*)ptr;
    while(true) {
        ret.resize(ret.size() + 0xff);
        z.next_out  = (Bytef*)ret.data() + ret.size() - 0xff;
        z.avail_out = 0xff;
        deflate(&z, Z_FINISH);
        if(z.avail_out != 0) {
            ret.resize(ret.size() - z.avail_out);
            break;
        }
    }
    deflateEnd(&z);

    return ret;
}

auto inflate(const void* const ptr, const size_t size) -> std::optional<std::vector<std::byte>> {
    auto ret = std::vector<std::byte>();
    auto z   = z_stream();

    ensure(inflateInit(&z) == Z_OK);
    z.avail_in = size;
    z.next_in  = (Bytef*)ptr;
    while(true) {
        ret.resize(ret.size() + 0xff);
        z.next_out  = (Bytef*)ret.data() + ret.size() - 0xff;
        z.avail_out = 0xff;
        inflate(&z, Z_FINISH);
        if(z.avail_out != 0) {
            ret.resize(ret.size() - z.avail_out);
            break;
        }
    }
    inflateEnd(&z);

    return ret;
}
