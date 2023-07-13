# Beasts-game

## Description

Beast Game is an multiplayer game developed in C using the Ncurses library. It allows up to four players to connect and play together via a server-client connection using sockets.

In the game, players will explore a labyrinth filled with coins to collect and dangerous beasts to avoid. The objective is to gather as many coins as possible while staying away from the beasts' reach. Be careful! If a player is caught by a beast, they will lose their collected coins.

The game offers real-time, synchronized gameplay, allowing players to compete against each other.

## Features

- Multiplayer Gameplay: Up to 4 players can connect to the game server and compete against each other.
- Server-Client Connection: The game uses sockets for communication between the server and clients, enabling multiplayer functionality.
- Labyrinth Generation: The game generates a labyrinth based on text file mapa.txt.
- Player Movement: Players can navigate through the labyrinth using arrow keys.
- Beast AI: Beasts are AI-controlled entities that hunt down players. Players must avoid them to stay alive and collect coins.
- Coin Collection: Players must collect coins scattered throughout the labyrinth and bring them to a designated base to earn points.
- Death and Respawn: If a player is caught by a beast, they die, and their collected coins are dropped. Players respawn at a designated location.
- Synchronized Gameplay: The game employs multi-threading to ensure synchronized gameplay across all connected players, providing a smooth and immersive experience.

## Compile

### 1. Compile the server code:
- Open the terminal and navigate to the game directory.
- Enter the following command: gcc server.c -o server -lncurses -pthread
### 2. Start the server:
- Run the server by executing the following command: ./server
### 3. Compile the client code:
- Open a new terminal window.
- Navigate to the game directory.
- Enter the following command: gcc client.c -o client -lncurses
### 4. Start a client:
- Run the client by executing the following command: ./client
- Repeat this step for each additional player (up to four players).
  
The server and clients will establish a connection, and the game will begin. 

## Map
To create a custom map, just edit mapa.txt file.

## Controls

### Players: 
- Use arrow keys (↑, ↓, ←, →) to move.
- 'q' or 'Q' to quit.
### Server:
- 'c' to spawn a coin.
- 't' to spawn a small treasure.
- 'T' to spawn a large treasue.
- 'b' or 'B' to spawn a beast.
- 'q' or 'Q' to quit.
## Screenshots
1. Server view
   :![Server](https://github.com/Wikcis/Beasts-game/assets/130648148/123e0d78-f4d4-4f1e-9717-a610b3833819)
2. Player view:
   
     ![2](https://github.com/Wikcis/Beasts-game/assets/130648148/a7094480-d500-4722-8a0c-c7520f9c7c32)

   
