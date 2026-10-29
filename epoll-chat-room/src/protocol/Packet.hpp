#pragma once
#include <cstdint>
#include <string>

namespace chat::protocol {

    constexpr std::uint16_t kMagic = 0xCAFE;
    constexpr std::uint8_t kVersion = 1;
    constexpr std::uint32_t kMaxBody = 1024 * 1024;

    enum class MsgType : std::uint8_t {
        HeartbeatReq = 1,
        HeartbeatRsp = 2,
        RegisterReq = 3,
        RegisterRsp = 4,
        LoginReq = 5,
        LoginRsp = 6,
        LogoutReq = 7,
        LogoutRsp = 8,
        ChatMsg = 9,
    };

    #pragma pack(push, 1)
    struct PacketHeader{
        std::uint16_t magic;
        std::uint8_t version;
        std::uint8_t type;
        std::uint32_t length;
        std::uint32_t seq;
    };
    #pragma pack(pop)

    static_assert(sizeof(PacketHeader) == 12, "PacketHeader must be exactly 12 bytes");

    struct Packet{
        MsgType     type{};
        std::uint32_t seq{};
        std::string body{};
    };

    enum class ParseResult{
        Ok,
        NeedMore,
        Error,
    };

    std::string encode_packet(const Packet& p);
}