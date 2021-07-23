<p align="center" width="100%">
    <img width="60%" src="https://github.com/Nabeel110/Client-Srever-Model/blob/main/Screenshots/ClientSideCommands.png" id="dsc-logo"> 
    <br>
</p>

<p align="center"> Welcome to Client-Server Model<br/>
                       By Nabeel Ahmed
    </p>
   


# Client-Srever-Model
Interaction of Client And Server over numerous commands

## Server-Side Commands:
<p>On server-Side, I have implemented 7 commands which are taken from user which are as follows:</p>

  - `connlist`: It displays list of all active connections i.e., active client Handlers. The list gets updated once any client handler terminates.
  - `list`: It displays list of all active processes of all active client Handlers.
  - `list` <ClientID>: If after list command client id is also passed it will return active processes of that particular client only iff the client id is valid. Example: list <c0>. I have assigned every client Handler a unique identifier like c0, c1...etc.
  - BONUS: `listall`: It displays list of all processes (active and inactive both) of all active client Handlers.
  - BONUS: `listall` <ClientID>: If after `listall` command client id is also passed it will return all processes of that particular client only iff the client id is valid. Example: `listall <c0>`. I have assigned every client Handler a unique identifier like c0, c1...etc. Client ID is validated every time.
  - Writing first "HELLO WORLD" Program.
  - `print <message>`: This command will send the message to all active client Handlers whose connection hasn’t been terminated yet.
  - `print <ClientID> <message> `: This command will send the message to only that client whose id we have specified provided the id is valid and that client Handler is also active.

## Client-Side Commands:
<p>On client-side, I have implemented the following commands::</p>

  - `add <list of int numbers>`: This command accepts list of integer numbers space separated to perform additions, e.g., add 3 4 45 5 will return 57.
  - `sub <list of int numbers> `: This command accepts list of integer numbers space separated to perform subtraction e.g., sub 3 4 will return -1.
  - `mult <list of int numbers>`: This command accepts list of integer numbers space separated to perform multiplication e.g., mult 3 4 will return 12.
  - `div <list of int numbers>`: This command accepts list of integer numbers space separated to perform division e.g., mult 3 4 will return 0.75.
  - `run <program exe name>`: This command will run the executable passed in iff the executable exist and is valid.
  - `exit`: This command will gracefully terminate client and all its spawned processes.
  - `list` or `list all`: If only list token is passed it will print only active processes list, however if `all` token after list is found it will print all active and inactive processes.
  - `kill <name/pid>`: This command will kill the already running process you have created if it is still running. It can kill the process by name or if you Wanna kill specific instance of that process you can kill by its pid.
  
  ## Client-Server Architecture Details:
  
  <p align="center" width="100%">
    <img width="60%" src="https://github.com/Nabeel110/Client-Srever-Model/blob/main/Screenshots/client-server-architecture-diagram.png" id="dsc-logo"> 
    <br>
</p>

# Client-Side Architecture
- On client-side I have implemented threads in order to provide parallelism so that my program doesn’t get freeze when one of the reads get blocked. This will allow simultaneously to take command from user from one thread and display output from server on another thread. Multi-threaded Client also allow us to send multiple commands at a time without waiting for server to respond to one command and then we deliver next command.
<hr/>

# Server-Side Architecture
- On server side, I have a connection process in one thread while in other thread I take input command from user. The connection process does nothing but accepts and establishes new connection and maintain its entry in connection list. The other thread just takes commands from the user and process it accordingly.
- In client Handler also we create threads in order to provide parallelism to our program and to prevent our program from freezing otherwise we might be either stuck on read from pipe or on read from socket. Therefore, I made one thread for read from pipe and displaying output on server while other thread communicates with client and read commands sent from client. This way we can perform multiple tasks without getting block on any read.
- Another architecture level detail includes that I have initialized and created pipes for Inter process communication (IPC) between client handler and connection process. Every client handler has its own 2-way pipes with thread in connection process. Thereby having different communications channels for every Client handler. This allows to take input from server in its thread, then sent it via pipe to Client Handler and finally writing it to their respective socket.
<hr/>

# Limitations of my Project:
- The very obvious limitation of my project so far is although I am sending multiple commands from client using multithreaded architecture on client side, the server still process one command at a time which is exactly one of the limitations to my project that server can't process multiple commands at a time since it is not designed to be multithreaded.
- My project can only process integer numbers for arithmetic operations, it cannot process fractional or decimal numbers.

## Some Screenshots of My Process table 

|Connlist|Process Table|
|--|--|
| ![Connlist](https://github.com/Nabeel110/Client-Srever-Model/blob/main/Screenshots/connlist.png)|  ![Process Table](https://github.com/Nabeel110/Client-Srever-Model/blob/main/Screenshots/prettyPrinting.png)| 

|Client-Server Commands|
|--|
|![Client-Server Commands](https://github.com/Nabeel110/Client-Srever-Model/blob/main/Screenshots/Client-server-cpmmands.png)|

