#include "protocol/Packet.hpp"

#include <arpa/inet.h>
#include <cstring>

namespace chat::protocol {

    std::string encode_packet(const Packet& p){
        std::string out;
        out.resize(sizeof(PacketHeader));

        const std::uint16_t magic = htons(kMagic);
        std::memcpy(&out[0], &magic, sizeof magic);

        out[2] = static_cast<char>(kVersion);
        out[3] = static_cast<char>(p.type);

        const std::uint32_t length = 
            htonl(static_cast<std::uint32_t>(p.body.size()));
        std::memcpy(&out[4], &length, sizeof length);

        const std::uint32_t seq = htonl(p.seq);
        std::memcpy(&out[8], &seq, sizeof seq);

        out += p.body;
        return out;
    }

}