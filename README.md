# UDPchat

## What is it?
UDPchat is a CLI based messenger that I wrote for a weekend project. I don't have any plans of actively maintaining it but if I get bored I might add some features. Feel free to contribute if you feel there is a need to do so. It should befairly easy to compile

## Technical Details
### UI
The UI of UDPchat is written purely in Lib NCurses and is aimed at being as simple as possible. It consists of an input bar in the bottom row of the screen and the rest of the screen shows the buffer of messages

### Communication
The communication backend of UDPchat is UDP sockets (hence the name) which allows UDPchat to be sessionless and run without a connection process.

The packet structure is as follows:
```<user>USERNAME</user><type>MESSAGE TYPE</type><body>MESSAGE BODY</body>```

An example of this is:
```<user>TDay</user><type>0</type><body>hello world</body>```

__Username - `<user>`__ <br />

The username header defines the username of the sender and should typically be put at the start of the packet (for readability purposes). It should be printed in unicode between the `<user>` and `</user>` tag.

__Message Type - `<type>`__ <br />

The type header defines the type of message the is being send. The message type will always be an int data type and so far the only types are as follows:
* `0` - Message
* `1` - Joining the chat

In future more may be added but for now this is it. the type header should be enclosed between the `<type>` and `</type>` tags.

__Message body - `<body>`__ <br />

The `<body>` tag indicates the body of the packet. The body tag does not need to be populated for every message type, but the opening and closing body tags must remain for consistency