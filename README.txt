Command line
my_channel.exe 6342 6343 60 12345
my_receiver.exe 127.0.0.1 6343 output.txt
my_sender.exe 127.0.0.1 6342 hello.txt

Order of exectuable
my_channel
my_receiver
my_sender

Files Content

channel\main.c - handling the input arguments and calling main function
channel\server.c - main fucntion that handle the channel functionality
channel\SocketSendRecvTools.c - handling sending and receiving data from sockets

receiver\main.c - handling the input arguments and calling main function
receiver\client.c - main fucntion that handle the channel functioality
receiver\code_functions.c - contain the ecc calculations
receiver\SocketSendRecvTools.c - handling sending and receiving data from sockets

sender\main.c - handling the input arguments and calling main function
sender\sender.c - main fucntion that handle the channel functioality
sender\code_functions.c - contain the ecc calculations
sender\SocketSendRecvTools.c - handling sending and receiving data from sockets