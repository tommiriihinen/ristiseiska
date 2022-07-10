# echo-client.py

import socket
import threading
import os
import sys
from enum import Enum

os.system('color')

HOST = "127.0.0.1"  # The server's hostname or IP address
PORT = 1234  # The port used by the server

print("Ristiseiska Client running on Python 3.10\n")

nickname = input("Choose your nickname: ")
print("Welcome " + nickname + "!\n")
print("Waiting for the server to start the game")

# Connecting To Server
client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client.connect(('127.0.0.1', 55555))


class State(Enum):
    play = 1,
    give = 2,
    wait = 3


state = State.wait
highlightCard = "NULL"
options = []


class Bcolors:
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'


def highlight(message, word="", color=Bcolors.OKCYAN):
    if word == "":
        return color + message + Bcolors.ENDC
    return message.replace(word, color + word + Bcolors.ENDC)


def highlight_many(message, words, color=Bcolors.OKCYAN):
    for word in words:
        message = highlight(message, word, color)
    return message


# Listening to Server and Sending Nickname
def receive():
    global state, highlightCard, options
    while True:
        try:
            # Receive Message From Server
            # If 'NICK' Send Nickname
            messages = client.recv(1024).decode('UTF-8').split("*")

            while len(messages) > 1:
                message = messages.pop(0);

                parts = message.split(';')
                cmd = parts[0]
                content = parts[1]

                # print("RECV: " + message)

                if cmd == 'PLAY':
                    state = State.play

                elif cmd == 'GIVE':
                    state = State.give

                elif cmd == 'WAIT':
                    state = State.wait
                    options.clear()

                elif cmd == 'CARDS':
                    pass

                elif cmd == 'OPTION':
                    options.append(content)

                elif cmd == 'CARD':
                    highlightCard = content

                elif cmd == 'NICK':
                    client.send(nickname.encode('UTF-8'))

                elif cmd == 'MSG':
                    if content.count("'s turn"):
                        content = highlight(content, "", Bcolors.UNDERLINE)
                        content = highlight(content, "", Bcolors.OKGREEN)
                    content = highlight(content, highlightCard, Bcolors.FAIL)
                    content = highlight_many(content, options)
                    print(content)
                    highlightCard = "NULL"

                elif cmd == 'ERROR':
                    content = highlight(content, "", Bcolors.WARNING)
                    print(content)

                else:
                    print(message)

        except Exception as e:
            try:
                print(e)
                client.send(str(e).encode('UTF-8'))
            # Cursed double-except
            except:
                print("Connection lost. Closing client")
                client.close()
                os._exit(1)


# Sending Messages To Server
def write():
    global state
    while True:

        choice = input()
        choice.strip()

        if state == State.play:

            continues = False
            if len(choice) == 2 and (choice[1].upper() == 'A' or choice[1].upper() == 'K'):
                if input("Will you continue? (y/n):") == 'y':
                    continues = True

            elif choice == 'P':
                message = 'P'

            message = choice + ';' + str(int(continues))
        else:
            message = choice
        # print("SEND: " + message)
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



