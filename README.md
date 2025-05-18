# 42_IRC
![License](https://img.shields.io/badge/license-MIT-blue.svg) ![C++98](https://img.shields.io/badge/C%2B%2B-98-blue.svg)

A robust Internet Relay Chat (IRC) server implementation in C++98, developed as part of the 42 curriculum. This server provides core IRC functionality with support for multiple clients, channels, and various IRC commands.

## Features

### Standard Version
- Full implementation of essential IRC commands:
  - `PING`: Server connectivity check
  - `JOIN`: Channel joining capabilities
  - `PASS`: Server password authentication
  - `NICK`: Nickname management
  - `PRIVMSG`: Private and channel messaging
  - `TOPIC`: Channel topic management
  - `USER`: User registration
  - `QUIT`: Graceful disconnection
  - `INVITE`: Channel invitation system
  - `KICK`: User removal from channels
  - `MODE`: Channel and user mode management

### Bonus Features
The bonus version includes a bot feature that enhances the server with automated interactions. The bot responds to specific commands and provides additional functionality to the chat environment.

## Technical Requirements

- C++98 compliant compiler
- Supports port range: 1024-65535
- Command-line format: `./ircserv <port> <password>`

## Installation

### Getting Started
```bash
# Clone the repository
git clone https://github.com/Bruno0798/42_IRC.git
cd 42_IRC
```

### Prerequisites
- C++ compiler with C++98 support
- Make build system

### Building

Standard version:
```bash
make all        # Build the standard server
make clean      # Remove object files
make fclean     # Remove object files and executable
make re         # Rebuild the server
```

Bonus version:
```bash
make bonus      # Build the server with bonus features
```

## Usage

### Running the Server

Standard version:
```bash
./ircserv <port> <password>
# Example:
./ircserv 6667 serverpass
```

Bonus version:
```bash
./ircserv_bonus <port> <password>
```

### Debugging and Testing

Memory leak testing:
```bash
make leaks      # Run memory leak test
```

## Project Structure

### Core Components
- `Server`: Main server implementation handling connections and client management
- `Client`: Client session management and command processing
- `Channel`: Channel operations and user management

### Command Implementation
- Modular command system in the `Commands/` directory
- Each command implemented as a separate module
- Utility functions for command processing

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Authors

- Bruno Miguel - https://github.com/Bruno0798
- Diogo Pinheiro - https://github.com/DiogoFSPinheiro
- Miguel Cardoso - https://github.com/Biltes

- Project developed at 42 School
