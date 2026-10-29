#include "protocol/Packet.hpp"
#include "protocol/PacketParser.hpp"
#include <cstdio>
#include <string>
#include <cstring>
#include <vector>
#include <random>
#include <algorithm>

static int g_failed = 0;
#define CHECK(cond) \
    do { if(!(cond)) { \
        std::fprintf(stderr, "FAIL %s:%d: %s\n", \
            __FILE__, __LINE__, #cond); \
        ++g_failed; \
        } \
    } while(0);

//大端辅助函数
static void put_u32(std::string& out, std::size_t offset, std::uint32_t value){
    out[offset + 0] = static_cast<char>((value >> 24) & 0xFF);
    out[offset + 1] = static_cast<char>((value >> 16) & 0xFF);
    out[offset + 2] = static_cast<char>((value >> 8) & 0xFF);
    out[offset + 3] = static_cast<char>(value & 0xFF);
}

static std::string make_packet(std::uint8_t type, std::uint32_t seq, const std::string& body){
    std::string out(12, '\0');
    out[0] = static_cast<char>(0xCA);
    out[1] = static_cast<char>(0xFE);
    out[2] = 1;                       // version
    out[3] = static_cast<char>(type);
    put_u32(out, 4, static_cast<std::uint32_t>(body.size()));   // length
    put_u32(out, 8, seq);
    out += body;
    return out;
}

int main(){
    //测试1：包头必须是12字节
    CHECK(sizeof(chat::protocol::PacketHeader) == 12);

    //测试2：一个完整的包
    //magic=0xCAFE version=1 type=1 length=2 seq=7 body="hi"
    const unsigned char one_packet[] = {
        0xCA, 0xFE, 0x01, 0x01,
        0x00, 0x00, 0x00, 0x02,
        0x00, 0x00, 0x00, 0x07,
        'h', 'i',
    };
    chat::protocol::PacketParser parser;
    parser.append(reinterpret_cast<const char*>(one_packet), sizeof one_packet);

    chat::protocol::Packet p;
    CHECK(parser.next(p) == chat::protocol::ParseResult::Ok);
    CHECK(static_cast<int>(p.type) == 1);
    CHECK(p.seq == 7);
    CHECK(p.body == "hi");
    CHECK(parser.next(p) == chat::protocol::ParseResult::NeedMore);
    
    //测试3：包头分两次到达
    chat::protocol::PacketParser parser2;
    parser2.append(reinterpret_cast<const char*>(one_packet), 5);
    CHECK(parser2.next(p) == chat::protocol::ParseResult::NeedMore);
    parser2.append(reinterpret_cast<const char*>(one_packet) + 5, sizeof one_packet - 5);
    CHECK(parser2.next(p) == chat::protocol::ParseResult::Ok);

    // 测试4：粘包，一次append连续放两次
    std::string stream;
    stream.append(reinterpret_cast<const char*>(one_packet), sizeof one_packet);
    stream.append(reinterpret_cast<const char*>(one_packet), sizeof one_packet);

    chat::protocol::PacketParser parser3;
    parser3.append(stream.data(), stream.size());

    CHECK(parser3.next(p) == chat::protocol::ParseResult::Ok);
    CHECK(p.seq == 7);
    CHECK(p.body == "hi");

    CHECK(parser3.next(p) == chat::protocol::ParseResult::Ok);
    CHECK(p.seq == 7);
    CHECK(p.body == "hi");

    CHECK(parser3.next(p) == chat::protocol::ParseResult::NeedMore);

    //测试5：包体分片
    chat::protocol::PacketParser parser4;
    parser4.append(reinterpret_cast<const char*>(one_packet), 13);
    CHECK(parser4.next(p) == chat::protocol::ParseResult::NeedMore);

    parser4.append(reinterpret_cast<const char*>(one_packet) + 13, 1);
    CHECK(parser4.next(p) == chat::protocol::ParseResult::Ok);
    CHECK(p.body == "hi");

    //测试6：空包体
    const unsigned char empty_packet[] = {
        0xCA, 0xFE, 0x01, 0x01,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x09,
    };
    chat::protocol::PacketParser parser5;
    parser5.append(reinterpret_cast<const char*>(empty_packet), sizeof empty_packet);
    CHECK(parser5.next(p) == chat::protocol::ParseResult::Ok);
    CHECK(p.body.empty() == true);
    CHECK(p.seq == 9);

    //测试7：坏包三连（坏magic, 坏version和超大length）
    //测试7a,magic = 00FE
    unsigned char bad_magic[sizeof one_packet];
    std::memcpy(bad_magic, one_packet, sizeof one_packet);
    bad_magic[0] = 0x00;

    chat::protocol::PacketParser parser_magic;
    parser_magic.append(reinterpret_cast<const char*>(bad_magic), sizeof bad_magic);
    CHECK(parser_magic.next(p) == chat::protocol::ParseResult::Error);
    //测试7b，version = 2
    unsigned char bad_version[sizeof one_packet];
    std::memcpy(bad_version, one_packet, sizeof one_packet);
    bad_version[2] = 0x02;  // magic 占 0~1，version 是索引 2

    chat::protocol::PacketParser parser_version;
    parser_version.append(reinterpret_cast<const char*>(bad_version), sizeof bad_version);
    CHECK(parser_version.next(p) == chat::protocol::ParseResult::Error);
    //测试7c，超长length
    unsigned char huge_header[] = {
        0xCA, 0xFE, 0x01, 0x01,
        0x00, 0x10, 0x00, 0x01,    // 1MB + 1
        0x00, 0x00, 0x00, 0x01,
    };

    chat::protocol::PacketParser parser_huge;
    parser_huge.append(reinterpret_cast<const char*>(huge_header), sizeof huge_header);
    CHECK(parser_huge.next(p) == chat::protocol::ParseResult::Error);

    //测试8：随机切分
    //验证构造函数make_packet()
    CHECK(make_packet(1, 7, "hi") == 
    std::string(reinterpret_cast<const char*>(one_packet), sizeof one_packet));
    //1.随机生成100个包，记录期望值
    std::vector<std::string> expected_bodies;
    std::string all;
    for(int i = 0; i < 100; i++){
        std::string body(static_cast<std::size_t>(i % 501),
            static_cast<char>('a' + i % 26));
        expected_bodies.push_back(body);
        all += make_packet(static_cast<std::uint8_t>(chat::protocol::MsgType::ChatMsg), 
        static_cast<std::uint32_t>(i), body);
    }
    //2.按1~17字节随机切片喂入
    std::mt19937 rng(12345);
    std::uniform_int_distribution<int> dist(1, 17);

    chat::protocol::PacketParser parser6;
    std::vector<chat::protocol::Packet> got;
    std::size_t pos = 0;

    while(pos < all.size()){
        std::size_t n = std::min<std::size_t>(
            static_cast<std::size_t>(dist(rng)), all.size() - pos);
        parser6.append(all.data() + pos, n);
        pos +=n;

        for(;;){
            chat::protocol::Packet pkt;
            auto r = parser6.next(pkt);
            if(r == chat::protocol::ParseResult::NeedMore) break;
            CHECK(r == chat::protocol::ParseResult::Ok);
            if(r != chat::protocol::ParseResult::Ok) break;
            got.push_back(pkt);
        }
    }
    //3.校验
    CHECK(got.size() == 100);
    for(std::size_t i = 0; i < got.size(); ++i){
        CHECK(got[i].seq == i);
        CHECK(got[i].body == expected_bodies[i]);
    }

    //测试9：补编码函数
    chat::protocol::Packet a{chat::protocol::MsgType::HeartbeatReq, 7,"hi"};
    CHECK(encode_packet(a) == std::string(reinterpret_cast<const char*>(one_packet),
    sizeof one_packet)
    );
    
    const unsigned char expected_empty[] = {
        0xCA, 0xFE, 0x01, 0x01,
        0x00, 0x00, 0x00, 0x00,
        0x01, 0x02, 0x03, 0x04,
    };
    chat::protocol::Packet b{
        chat::protocol::MsgType::HeartbeatReq, 0x01020304u, ""
    };
    CHECK(encode_packet(b) == std::string(reinterpret_cast<const char*>(expected_empty),
    sizeof expected_empty));

    return g_failed == 0 ? 0 : 1;
}