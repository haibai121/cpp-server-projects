# cpp-server-projects

C++ server-side projects, built while learning Linux network programming.

## Projects

### [epoll-chat-room](epoll-chat-room)

A multiplayer chat server for local networks, built with Linux epoll and a custom binary protocol.

- Non-blocking sockets and an epoll-based Reactor event loop
- 12-byte fixed header + variable-length body to handle TCP framing
- Registration, login, group chat, online user list, heartbeat (in progress)
- Main-sub Reactor and I/O thread pool (planned)

Status: under development. See [epoll-chat-room/README.md](epoll-chat-room/README.md).

## Tech stack

C++17 · CMake · Linux (epoll) · TCP · Git

## Repository layout

```
.
├── epoll-chat-room/    # first project
├── README.md
└── .gitignore
```

Older practice projects are preserved on the `archive/old-practice` branch.
