print("Loading libraries...")

import os
import socket
import sys
import codecs

import numpy as np
import tensorflow as tf
import keras
from keras import backend as K

np.set_printoptions(linewidth=80, precision=3)

HOST = "127.0.0.1"  # The server's hostname or IP address
PORT = 1234  # The port used by the server
DIR = "models"
nickname = "mestari-tikku"
DEFAULT_FILE = ""
DEBUG = True
SHOW_GAME = True
MAX_PLAYERS = 3

model = None
client = None

print(f"Ristiseiska NeuralClient running on Python {sys.version.split()[0]} and TensorFlow {tf.version.VERSION}\n")


def binarize_card(card_id):
    suit = 0
    if card_id[0] in ['C', '♣']:
        suit = 0
    elif card_id[0] in ['D', '♦']:
        suit = 1
    elif card_id[0] in ['H', '♥']:
        suit = 2
    elif card_id[0] in ['S', '♠']:
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
    card_index = (suit * 13 + rank) - 1  # 0-51
    return card_index


def de_binarize_card(card_index):
    suit = ' '
    rem = card_index // 13
    if rem == 0:
        suit = 'C'
    elif rem == 1:
        suit = 'D'
    elif rem == 2:
        suit = 'H'
    elif rem == 3:
        suit = 'S'

    rank = ' '
    mod = (card_index + 1) % 13
    if mod == 1:
        rank = 'A'
    elif mod == 10:
        rank = 'X'
    elif mod == 11:
        rank = 'J'
    elif mod == 12:
        rank = 'Q'
    elif mod == 0:
        rank = 'K'
    else:
        rank = str(mod)
    return suit + rank  # C-S A-K


def normalize_id(id):
    if id < my_id:
        return id + 1
    else:
        return id


my_id = 0
current_player_id = 0
tables = np.zeros((MAX_PLAYERS, 52))
table = np.zeros(52)
hand = np.zeros(52)
options = np.zeros(52)
action = 0


# Listening to Server and Sending Nickname
def receive():
    global options, action, hand, tables, table, my_id, current_player_id
    dec = codecs.getincrementaldecoder('utf8')()

    while True:
        received = client.recv(2048)
        messages = dec.decode(received).split("*")

        while len(messages) > 1:
            message = messages.pop(0).strip()

            parts = message.split(';')
            if len(parts) == 1:
                print("ERRONEOUS INPUT:", message, ":", received)
                continue

            cmd = parts[0]
            content = parts[1]

            if cmd == 'PLAY':
                action = 1
                decision = decide()
                send(decision)
                options = np.zeros(52)

            elif cmd == 'GIVE':
                action = 0
                decision = decide()
                send(decision)
                options = np.zeros(52)

            elif cmd == 'WAIT':
                pass

            elif cmd == 'STARTING_CARDS':
                hand[binarize_card(content)] = 1

            elif cmd == 'CARD_PLAYED':
                # Put card on the correct players table
                player = normalize_id(current_player_id)
                card = binarize_card(content)
                tables[player][card] = 1
                table[card] = 1

            elif cmd == 'CARD_GIVEN':
                # Take card in hand
                hand[binarize_card(content)] = 1

            elif cmd == 'TURN':
                current_player_id = int(content)

            elif cmd == 'ID':
                my_id = int(content)
                # Reset
                options = np.zeros(52)
                current_player_id = 0
                tables = np.zeros((MAX_PLAYERS, 52))
                table = np.zeros(52)
                hand = np.zeros(52)
                action = 0

            elif cmd == 'OPTION':
                if DEBUG:
                    print("Option:", content)
                options[binarize_card(content)] = 1

            elif cmd == 'NICK':
                send(nickname)

            elif cmd == 'MSG':
                if SHOW_GAME:
                    print(content)

            elif cmd == "END":
                pass

            else:
                print(message)


def send(message):

    #         print("Connection lost. Closing client")
    #         client.close()
    # os._exit(1)
    message += "*"
    try:
        client.send(message.encode('UTF-8'))
    except socket.error as e:
        print(e)


def decide():
    global action, hand, table, model

    # Pass of nothing fits
    if np.all(options == 0):
        print("Passing", options)
        return 'P'

    continues = False

    # Construct model input
    model_input = np.concatenate((action, hand, table.flatten()), axis=None)
    model_input = model_input.reshape((1, 105))
    # print("Model input:\n", model_input)
    # Decide
    model_output = model(model_input)
    if DEBUG:
        print("Model output:\n", model_output)
        print(model_output.dtype)
    # Realize
    # print("Options:", options)
    realization = np.multiply(model_output, options)
    if DEBUG:
        print("Realization:\n", realization)
    realization[realization == 0] = -np.inf
    decision = np.argmax(realization)

    # If everything fits, end game. Wasteful here, needs refactoring
    if np.array_equal(options, hand):
        for i in np.flatnonzero(options):
            if de_binarize_card(i)[1] in ['K', 'A']:
                decision = i
                continues = True
                break

    # Apply to self
    hand[decision] = 0
    if action == 1:  # Card played
        table[decision] = 1
    # Apply to world
    card = de_binarize_card(decision)
    if DEBUG:
        print("Decision", card)

    return card + ";" + str(int(continues))


def load_model():
    global nickname
    if DEFAULT_FILE:
        model_path = DIR + "/" + DEFAULT_FILE
    else:
        print("Directory " + DIR + " contents:")
        for filename in os.scandir(DIR):
            if not filename.is_file():
                print(f" {filename.name}")
        model_directory = input("Which model to use: ")
        nickname = model_directory
        model_path = DIR + "/" + model_directory
        print("Loading " + model_path)

    def agreeableness(y_true, y_pred):
        if K.sum(y_true) == 1:
            return K.argmax(y_true) == K.argmax(y_pred)
        else:
            return K.argmin(y_true) == K.argmax(y_pred)

    def squared_error_masked(y_true, y_pred):
        """ Squared error of elements where y_true is not 0 """
        err = y_pred - (K.cast(y_true, y_pred.dtype) * 1)
        return K.sum(K.square(err) * K.cast(K.not_equal(y_true, 0),
                                            y_pred.dtype), axis=-1)

    custom_objects = {"squared_error_masked": squared_error_masked,
                      "agreeableness": agreeableness}
    with keras.utils.custom_object_scope(custom_objects):
        model = tf.keras.models.load_model(model_path)
    model.summary()
    return model


def main():
    global model, client
    # Connecting To Server
    client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client.connect(('127.0.0.1', 55555))

    model = load_model()

    receive()
    # Starting Threads For Listening And Writing
    # receive_thread = threading.Thread(target=receive)
    # receive_thread.start()


if __name__ == "__main__":
    main()
