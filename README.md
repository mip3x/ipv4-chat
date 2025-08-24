# ipv4-chat
Simple IPv4 chat on LAN running via UDP broadcast. Each participant has equal rights. Each participant can see the messages of others.

## Installation
### Prerequisites
Ensure you have the following dependencies installed:
- GCC (or Clang)
- GNU Make

### Build
To compile the chat, run:
```sh
make
```

This will generate an executable named `ipv4-chat`

## Running the Sniffer
To start the chat, use:
```sh
./ipv4-chat -a `IPv4_ADDR` -p `PORT`
```

## Startup flags
`[m]` - mandatory flag; `[o]` - optional flag
- `a`[m] `IPv4_ADDR`: set IPv4 address of listening host
- `p`[m] `PORT`: set port of listening host
- `v`[o]: (verbose) debugging information output
- `h`[o]: (help) output of auxiliary information

## Auxiliary script
The script will launch the chat instance and substitute default values if desired by the user
```sh 
./run_ipv4_chat.sh
```
