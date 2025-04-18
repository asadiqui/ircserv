ircserv is an IRC server developped in C++ 98. The server is capable of handling multiple clients at the same time and never hangs.

> [!NOTE]
> This is not a client, and it doesn't handle server-to-server communication.

The executable runs as follows:
```
./ircserv <port> <password>
```
* port: The port number on which your IRC server will be listening to for incoming IRC connections.
* password: The connection password. It will be needed by any IRC client that tries to connect to your server.

This is a list of implemented features:

- [x] Authentication
- [x] Setting a nickname
- [x] Setting a username
- [x] Joinning a channel
- [x] Sending and receiving private messages using an IRC client
- [x] All the messages sent from one client to a channel are forwarded to every other client that joined the channel
- [x] Supporting operators and regular users.

The implemented commands that are specific to channel operators:

command | description
--------|------------
KICK | Eject a client from the channel
INVITE | Invite a client to a channel
TOPIC | Change or view the channel topic
MODE | Change the channel’s mode (needs a flag)
MODE -i | Set/remove Invite-only channel
MODE -t | Set/remove the restrictions of the TOPIC command to channel operators
MODE -k | Set/remove the channel key (password)
MODE -o | Give/take channel operator privilege
MODE -l | Set/remove the user limit to channel
