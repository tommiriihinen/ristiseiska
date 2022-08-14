import socket
import threading
import os
import sys
from enum import Enum

os.system('color')

HOST = "127.0.0.1"  # The server's hostname or IP address
PORT = 1234  # The port used by the server

MSG_DELIMITER = "§"
MSG_DELIMITER_UTF8 = MSG_DELIMITER.encode('UTF-8')

print(f"Ristiseiska CandyClient running on Python {sys.version.split()[0]}\n")

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
    OPTIONCYAN = '\033[96m'
    NAMEGREEN = '\033[92m'
    WARNING = '\033[93m'
    CHANGERED = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'


def highlight(message, word="", color=Bcolors.OPTIONCYAN):
    if word == "":
        return color + message + Bcolors.ENDC
    return message.replace(word, color + word + Bcolors.ENDC)


def highlight_many(message, words, color=Bcolors.OPTIONCYAN):
    for word in words:
        message = highlight(message, word, color)
    return message


# Listening to Server and Sending Nickname
def listen():
    while True:
        try:
            messages = client.recv(1024).decode('UTF-8').split(MSG_DELIMITER)

            while len(messages) > 1:
                message = messages.pop(0)
                handle_message(message)

        except Exception as e:
            print(e)
            os._exit(1)


def handle_message(message):
    global state, highlightCard, options

    parts = message.split(';')
    cmd = parts[0]
    content = parts[1]

    # print("RECV: " + message)

    if cmd == 'PLAY':
        choice = input().upper()

        continues = False
        if len(choice) == 2 and (choice[1] == 'A' or choice[1] == 'K'):
            if input("Will you continue? (y/n):") == 'y':
                continues = True

        send(choice + ';' + str(int(continues)))

    elif cmd == 'GIVE':
        choice = input().upper()
        send(choice)

    elif cmd == 'OPTION':
        options.append(content)

    elif cmd == 'TURN':
        options.clear()
        highlightCard = "null"

    elif cmd in ['CARD_PLAYED', 'CARD_GIVEN']:
        highlightCard = content

    elif cmd == 'NICK':
        send(nickname)

    elif cmd in ['MSG', 'PROMPT']:
        if content.count("'s turn"):
            content = highlight(content, "", Bcolors.UNDERLINE)
            content = highlight(content, "", Bcolors.NAMEGREEN)
        content = highlight_many(content, options)
        content = highlight(content, highlightCard, Bcolors.CHANGERED)
        print(content)

    elif cmd == 'ERROR':
        content = highlight(content, "", Bcolors.WARNING)
        print(content)

    elif cmd in ['CARDS', 'STARTING_CARDS', 'ID', 'END']:
        pass

    else:
        print(message)


def send(message):
    client.send((message + MSG_DELIMITER).encode('UTF-8'))


listen()


"""
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    s.sendall(b"Hello, world")
    data = s.recv(1024)

print(f"Received {data!r}")
"""



