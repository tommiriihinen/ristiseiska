import socket
import threading
import os
import sys
from enum import Enum

from tensorflow.keras import backend as K
import tensorflow as tf
from tensorflow import keras


HOST = "127.0.0.1"  # The server's hostname or IP address
PORT = 1234  # The port used by the server
DIR = "model"
SUB = ""

print(f"Ristiseiska NeuralClient running on Python {sys.version.split()[0]} and TensorFlow {tf.version.VERSION}\n")

nickname = "Mestari Tikku"

print("Directory " + DIR + " contents:")
for filename in os.scandir(DIR):
    if filename.is_file():
        print(filename.path)
modelfile = input("Which model to use: ")


def squared_error_masked(y_true, y_pred):
    """ Squared error of elements where y_true is not 0 """
    err = y_pred - (K.cast(y_true, y_pred.dtype) * 1)
    return K.sum(K.square(err) * K.cast(K.not_equal(y_true, 0),
                                        y_pred.dtype), axis=-1)


custom_objects = {"squared_error_masked": squared_error_masked}
with keras.utils.custom_object_scope(custom_objects):
    model = tf.keras.models.load_model(DIR + modelfile + SUB)
model.summary()

# Connecting To Server
client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client.connect(('127.0.0.1', 55555))


def binarize_card_id(card_id):
    suit = 0
    if card_id[0] == 'C':
        suit = 0
    elif card_id[0] == 'D':
        suit = 1
    elif card_id[0] == 'H':
        suit = 2
    elif card_id[0] == 'S':
        suit = 3

    if card_id[1] == 'A':
        rank = 1
    elif card_id[1] == 'X':
        rank = 10
    elif card_id[1] == 'J':
        rank = 11
    elif card_id[1] == 'Q':
        rank = 12
    elif card_id[1] == 'K':
        rank = 13
    else:
        rank = int(card_id[1])

    return (suit * 13 + rank) - 1  # 0-51


def de_binarize_card_id(card_id):
    suit = ' '
    rem = card_id // 13
    if rem == 0:
        suit = 'C'
    elif rem == 1:
        suit = 'D'
    elif rem == 2:
        suit = 'H'
    elif rem == 3:
        suit = 'S'

    rank = ' '
    mod = card_id % 13
    if mod == 1:
        rank = 'A'
    elif mod == 10:
        rank = 'X'
    elif mod == 11:
        rank = 'J'
    elif mod == 12:
        rank = 'Q'
    elif mod == 13:
        rank = 'K'
    else:
        rank = str(mod)

    return suit + rank  # C-S A-K


def normalize_id(id):
    if id < my_id:
        return id + 1
    else:
        return id


class State(Enum):
    play = 1,
    give = 2,
    wait = 3


state = State.wait
options = []

my_id = 0
current_player_id = 0

tables = (([0] * 52) for _ in range(7))
hand = [0] * 52
action = 0


# Listening to Server and Sending Nickname
def receive():
    global state, options, action, hand, tables, my_id, current_player_id
    while True:
        try:
            # Receive Message From Server
            # If 'NICK' Send Nickname
            messages = client.recv(1024).decode('UTF-8').split("*")

            while len(messages) > 1:
                message = messages.pop(0)

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
                    options = [0]*52

                elif cmd == 'STARTING_CARDS':
                    hand[binarize_card_id(content)] = 1

                elif cmd == 'CARD_PLAYED':
                    # Put card on the correct players table
                    player = normalize_id(current_player_id)
                    card = binarize_card_id(content)
                    tables[player][card] = 1

                elif cmd == 'CARD_GIVEN':
                    # Take card in hand
                    hand[binarize_card_id(content)] = 1

                elif cmd == 'TURN':
                    current_player_id = int(content)

                elif cmd == 'ID':
                    my_id = int(content)

                elif cmd == 'OPTION':
                    options[binarize_card_id(content)] = 1

                elif cmd == 'NICK':
                    client.send(nickname.encode('UTF-8'))

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
    global state, action, hand, tables, model
    waiting = True
    while True:

        if waiting and (state == State.play or state == State.give):

            # Construct model input
            model_input = [action]
            model_input += hand
            for table in tables:
                model_input += table
            # Decide
            model_output = model.predict(model_input)
            # be real!
            realization = []
            for want, possibility in zip(model_output, options):
                realization.append(want * possibility)
                # take best!
            decision = 0
            max = 0
            for i in range(52):
                if realization[i] > max:
                    max = realization[i]
                    decision = i

            # Apply to self
            hand[decision] = 0
            if state == State.play:
                tables[0][decision] = 1
            # Apply to world
            card = de_binarize_card_id(decision)
            print(" CA C2 C3 C4 C5 C6 C7 C8 C9 CX CJ CQ CK DA D2 D3 D4 D5 D6 D7 D8 D9 DX DJ DQ DK HA H2 H3 H4 H5 H6 H7 H8 H9 HX HJ HQ HK SA S2 S3 S4 S5 S6 S7 S8 S9 SX SJ SQ SK")
            print(model_output)
            print(realization)
            print(card)
            client.send(card.encode('UTF-8'))
            # Go into dormancy
            waiting = False

        if not waiting and state.wait:
            waiting = True


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



