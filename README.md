<p align="center">
  <img src="pics/Highboy_repo.png" alt="HighBoy Banner" width="1000"/>
</p>

---

# 📡 HighBoy Firmware (Beta)

> 🌍 **Languages**:  [🇺🇸 English](README.md) | [🇧🇷 Português](README.pt.md) 

This repository contains a **firmware in development** for the **HighBoy** platform.
**Warning:** this firmware is in its **beta phase** and is **still incomplete**.



## Officially Supported Targets

| ESP32-S3 |
| -------- |



## Firmware Structure

Unlike basic examples with a single `main.c`, this project uses a modular structure organized into **components**, which are divided as follows:

- **Drives** – Handles hardware drivers and interfaces.
- **Services** – Implements support functionalities and auxiliary logic.
- **Core** – Contains the system's central logic and main managers.
- **Applications** – Specific applications that use the previous modules.

This division facilitates scalability, code reuse, and firmware organization.

📷 See the general project architecture:
[![Firmware Architecture](pics/architecture)](pics/architecture.png)

---

## How to use this project

We recommend that this project serves as a basis for custom projects with ESP32-S3.
To start a new project with ESP-IDF, follow the official guide:
🔗 [ESP-IDF Documentation - Create a new project](https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html#start-a-new-project)



## Initial project structure

Despite the modular structure, the project still maintains an organization compatible with the ESP-IDF build system (CMake).

Example layout:

```bash
├── CMakeLists.txt
├── components
│   ├── Drives
│   ├── Services
│   ├── Core
│   └── Applications
├── main
│   ├── CMakeLists.txt
│   └── main.c
└── README.md
```

- The project is in its **beta** phase, subject to frequent changes.
- Contributions and feedback are welcome to evolve the project.



## 🤝 Our Supporters

Special thanks to the partners supporting this project:

[![PCBWay](pics/PCBway.png)](https://www.pcbway.com)



## License
This project is licensed under the [Apache License, Version 2.0](LICENSE).
