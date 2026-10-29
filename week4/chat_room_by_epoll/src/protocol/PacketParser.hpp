#include<iostream>
#include "protocol/Packet.hpp"

namespace chat::protocol{

class PacketParser{
public:
    void append(const char* data, std::size_t len);
    ParseResult next(Packet& out);

private:
    std::string buffer_;
    std::size_t read_idx_ = 0;
    std::size_t readable() const;
    void compact();
};
} //namespace chat::protocol