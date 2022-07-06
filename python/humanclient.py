# echo-client.py

import socket
import threading
import os
from enum import Enum

HOST = "127.0.0.1"  # The server's hostname or IP address
PORT = 1234  # The port used by the server

nickname = input("Choose your nickname: ")

# Connecting To Server
client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client.connect(('127.0.0.1', 55555))


class State(Enum):
    play = 1,
    give = 2,
    wait = 3


state = State.wait


# Listening to Server and Sending Nickname
def receive():
    global state
    while True:
        try:
            # Receive Message From Server
            # If 'NICK' Send Nickname
            message = client.recv(1024).decode('UTF-8')
            parts = message.split(';')
            command = parts[0]
            content = parts[1]
            reply = 'null'

            #print("RECV: " + message)

            match command:
                case 'PLAY':
                    state = State.play
                case 'GIVE':
                    state = State.give
                case 'WAIT':
                    state = State.wait
                case 'CARDS':
                    pass
                case 'NICK':
                    client.send(nickname.encode('UTF-8'))
                case 'MSG':
                    print(content)
                case 'ERROR':
                    print(content)
                case _:
                    print(message)

        except Exception as e:
            try:
                # Close Connection When Error
                print("An error occured!")
                client.send(str(e).encode('UTF-8'))
            except:
                print("It was fatal!")
                client.close()
                os._exit(1)


# Sending Messages To Server
def write():
    global state
    while True:

        choice = input()
        choice.strip()
        match state:
            case State.play:

                continues = '0'
                if len(choice) == 2:
                    rank = choice[1]
                    if rank == 'A' or rank == 'K':
                        if input("Will you continue? (y/n):") == 'y':
                            continues = '1'

                elif choice == 'P':
                    message = 'P'

                message = choice + ';' + continues

            case State.give:
                message = choice
            case State.wait:
                message = choice

        #print("SEND: " + message)
        client.send(message.encode('UTF-8'))


# Starting Threads For Listening And Writing
receive_thread = threading.Thread(target=receive)
receive_thread.start()

write_thread = threading.Thread(target=write)
write_thread.start()


"""
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    s.sendall(b"Hello, world")
    data = s.recv(1024)

print(f"Received {data!r}")
"""