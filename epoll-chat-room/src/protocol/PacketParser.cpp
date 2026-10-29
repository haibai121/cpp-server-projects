#include "protocol/PacketParser.hpp"
#include <cstring>
#include <arpa/inet.h>

namespace chat::protocol{
    void PacketParser::append(const char* data, std::size_t len){
        buffer_.append(data, len);
    }

    std::size_t PacketParser::readable() const{
        return buffer_.size() - read_idx_;
    }

    ParseResult PacketParser::next(Packet& out){
        if(PacketParser::readable() < sizeof(PacketHeader)){
            return ParseResult::NeedMore;
        }
        PacketHeader h{};
        std::memcpy(&h, buffer_.data() + read_idx_, sizeof(PacketHeader));
        h.magic = ntohs(h.magic);
        h.length = ntohl(h.length);
        h.seq = ntohl(h.seq);
        if(h.magic != kMagic){
            std::cout << "Unknown Protocol!\n" ;
            return ParseResult::Error;
        }
        if(h.version != kVersion){
            std::cout << "Unknown Version!\n";
            return ParseResult::Error;
        }
        if(h.length > kMaxBody){
            std::cout << "Exceeded memory limit!\n"; // 这些不能依靠iostream直接输出，而是返回一个错误，具体见阶段7，上面的一样
            return ParseResult::Error;
        }
        const std::size_t total = sizeof(PacketHeader) + h.length;
        if(readable() < total){
            return ParseResult::NeedMore;
        }
        out.type = static_cast<MsgType>(h.type);
        out.seq = h.seq;
        out.body.assign(
            buffer_.data() + read_idx_ + sizeof(PacketHeader),
        h.length);
        read_idx_ += total;
        compact();
        return ParseResult::Ok;
    }
    void PacketParser::compact(){
        if(read_idx_ == buffer_.size()){
            buffer_.clear();
            read_idx_ = 0;
        }else if(read_idx_ >= 8192) {
            buffer_.erase(0, read_idx_);
            read_idx_ = 0;
        }
    }
    
}