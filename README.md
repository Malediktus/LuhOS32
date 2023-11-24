# LuhOS - Lightweight Unix Hobby Operating System

![LuhOS Logo](Logo.png)

LuhOS (Lightweight Unix Hobby Operating System) is a minimalistic, Unix-like hobby kernel and operating system designed for educational and experimental purposes. It provides a platform for learning about operating system development, low-level programming, and system architecture. LuhOS is inspired by the principles of simplicity, modularity, and elegance.

## Table of Contents

- [Features](#features)
- [Getting Started](#getting-started)
- [System Requirements](#system-requirements)
- [Building LuhOS](#building-luhos)
- [Running LuhOS](#running-luhos)
- [Contributing](#contributing)
- [License](#license)

## Features

- **Kernel:** A minimalistic kernel that supports multitasking and basic system calls.
- **File System:** A simple file system for storing and retrieving data.
- **Shell:** A basic shell for interacting with the operating system.
- **Drivers:** Drivers for keyboard input, VGA display, and basic hardware interaction.
- **Processes:** Support for creating and managing multiple user and system processes.
- **Memory Management:** Basic memory management including virtual memory and paging.
- **Networking (future feature):** Planned support for networking functionalities.
- **User-space Programs (future feature):** Framework for running user-space programs.
- **Documentation:** Comprehensive documentation to aid users and developers.

## Getting Started

Before you begin, make sure you have the necessary development tools and a basic understanding of operating system development. You should also be comfortable with low-level programming in C and assembly.

## System Requirements

LuhOS is designed to be run on x86-based architectures. To build and run LuhOS, you will need:

- A modern x86-compatible CPU.
- A development environment that includes GCC (cross-compiler), Binutils (cross-binutils), GNU Make, and the Limine bootloader (already included).
- Any version of GNU mtools.
- Git for version control.
- Optional: QEMU emulator for testing.
- Optional: Doxygen for building documentation.

## Building LuhOS

1. Clone the LuhOS repository to your local machine:

   ```bash
   git clone https://github.com/yourusername/LuhOS.git
   ```

2. Change to the LuhOS directory:

   ```bash
   cd LuhOS
   ```

3. Build the kernel and user-space components using the provided build script:

   ```bash
   ./build.sh
   ```

## Running LuhOS

LuhOS can be run on a real machine or in an emulator such as QEMU or Bochs. The exact process may vary depending on your setup. Refer to the documentation for detailed instructions on running LuhOS on your system.

## Contributing

LuhOS is an open-source project released under the GNU License, and we welcome contributions from the community. If you're interested in contributing to LuhOS, please follow our [contributing guidelines](CONTRIBUTING.md) to get started.

## License

LuhOS is released under the GNU License. See the [License](LICENSE) file for details.

###

Enjoy exploring and experimenting with LuhOS! If you have any questions or encounter issues, feel free to open an issue on our GitHub repository. We hope you find this project educational and inspiring as you delve into the fascinating world of operating system development.
