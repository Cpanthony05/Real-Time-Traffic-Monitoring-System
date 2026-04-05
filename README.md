Real-Time Traffic Monitoring System

This project is a C-based client-server application designed to monitor road traffic in real-time. It utilizes a central server to coordinate communication with multiple vehicle clients, providing live updates on speeds and road incidents.
## Core Features

    Reliable Communication: Uses the TCP protocol to ensure an ordered, lossless flow of critical data such as speed limits and incident reports.

    I/O Multiplexing: The server employs the select() function to manage multiple concurrent client connections efficiently without blocking.

    Graph-Based Map: The road network is modeled as a graph, where nodes represent streets and edges represent physical connections.

    Dynamic Traffic Logic:

        Automated Updates: Clients transmit their current speed to the server every 60 seconds.

        Incident Management: Reporting an accident triggers a broadcast notification to all drivers and reduces the street's speed limit to one-third.

        Road Blocks: If a street's speed limit reaches 0 due to multiple incidents, the node is disconnected from the map.

    Subscription Service: Users can subscribe via extra_info to receive supplementary data (weather, sports, or fuel prices) relevant to their current location.

## System Architecture

The application follows a classic Client-Server model:
### The Server

    Listens on Port 3124.

    Initializes the map from a configuration file provided as a command-line argument.

    Validates vehicle movements based on street adjacency.

    Handles sudden client disconnections gracefully using FD_CLR.

### The Client

    Connects by providing an initial street and speed.

    Receives real-time warnings if the current speed exceeds the street limit.

    Supports interactive commands to navigate the map or report hazards.

## Protocol Commands
    where_to? - Requests a list of adjacent streets from the current position.
    go_to <street> - Moves the vehicle to a connected street and updates the local speed limit.
    speed <value> - Manually updates the vehicle speed.
    incident - Reports an accident on the current street to all participants.
    extra_info - Toggles subscriptions for location-based info (weather/gas prices).
    quit - Terminates the connection.
## Installation & Usage
### Prerequisites

    GCC Compiler

    Linux Environment (POSIX compliant) 

### Compilation
Bash

gcc server.c -o server
gcc client.c -o client

### Running the Application

    Start the Server:
    Bash

    ./server map_config.txt

    Start a Client:
    Bash

    ./client <server_ip> 3124

## Development & Testing

    Author: Cobzaru Paul-Antonio 

    Institution: UAIC, Faculty of Computer Science Iași 

    Validation: Network traffic and packet sequences were verified using Wireshark to ensure protocol integrity.
