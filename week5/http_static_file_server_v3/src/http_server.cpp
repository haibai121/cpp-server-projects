#include "../include/http_server.hpp"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <iostream>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <fcntl.h> 

class FileDescriptor{
    int fd_;
public:
    FileDescriptor(int fd = -1) : fd_(fd){}
    ~FileDescriptor(){}

    //禁止拷贝构造
    FileDescriptor(const FileDescriptor&) = delete;
    FileDescriptor& operator=(const FileDescriptor&) = delete;
    
    //允许转移
    FileDescriptor(FileDescriptor&& other)noexcept : fd_(other.fd_) {
            other.fd_ = -1;
        }
    FileDescriptor& operator=(FileDescriptor&& other) noexcept {
        if(this !=  &other){
            if(fd_ >= 0) close(fd_);
            fd_ = other.fd_;
            other.fd_ = -1;
        }
        return *this;
    }
    int get() const {}
    explicit operator bool() const {}
};