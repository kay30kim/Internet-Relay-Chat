## About this project

The objective of this project is to reproduce the functioning of an IRC(Internet Relay Chat) server. IRC is designed for real-time message exchange, primarily for group communication in discussion forums (channels) but also supporting private messaging and data exchange, including file sharing.

<img width="1149" alt="Image" src="https://github.com/user-attachments/assets/23667b4c-c61d-477d-ae91-5238c9f46f57" />

### Overview

#### Multi-Client Server Model
It follows a client-server architecture, where multiple clients connect to a centralized IRC server to exchange messages. The server handles client authentication, message routing, and channel management.

#### Multithreaded Server Implementation
The server utilizes multithreading to efficiently manage multiple simultaneous client connections. Each client connection is handled in a separate thread, allowing concurrent message processing and minimizing latency.

#### Scalability & Performance
Efficient handling of multiple connections using non-blocking sockets and thread synchronization mechanisms. Designed to support large-scale messaging while ensuring responsiveness.

#### Server
The `Server` class represents the core of the `IRC server`. It handles client connections, message processing, and user management. The server listens for incoming connections, establishes data sockets with clients, and responds to IRC commands. The server uses the poll(or equivalent) system call for efficient event handling and non-blocking I/O so we can serve multiple clients at once.

#### Channel
The `Channel` class represents an `IRC channel`, where users can join and exchange messages. The class manages the users in the channel, broadcasts messages to all users, and handles channel-specific commands.

#### User
The `User` class represents an `IRC user` connected to the server. It stores user-specific information, such as nickname and connection status. Users can join channels, send and receive messages, and interact with other users on the server.

<img width="671" alt="Image" src="https://github.com/user-attachments/assets/0f688d50-93f1-4654-aec4-3d58aedb73d0" />

## Ressources

- **Regarding the client/server connection** : [Beej's Guide to network programming](https://beej.us/guide/bgnet/pdf/bgnet_a4_c_1.pdf). This is super super helpful to better understand what is a socket, what the `poll()` function entails, which system calls we should use or in which order, and so on.

<img width="417" alt="Image" src="https://github.com/user-attachments/assets/04e5b633-9927-4c7a-b84c-9a2d2261afb5" />

- **A link to the IRC Client Protocol with a good formatting** : [Modern IRC Client Protocol](https://modern.ircdocs.horse/). You will find there a description of all the commands with correct syntax, expected Numerical Replies,...

