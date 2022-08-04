print("Loading libraries...")

import os
import socket
import sys
import codecs
import logging
from timeit import default_timer as timer

import numpy as np
import tensorflow as tf
import keras
from keras import backend as K

"""
We are waiting for google to get their shit together with tensorflow.
There is a dependency conflict with protobuf

import googlecloudprofiler
# Profiler initialization. It starts a daemon thread which continuously
# collects and uploads profiles. Best done as early as possible.
try:
    # service and service_version can be automatically inferred when
    # running on App Engine. project_id must be set if not running
    # on GCP.
    googlecloudprofiler.start(verbose=3)
except (ValueError, NotImplementedError) as exc:
    print(exc)  # Handle errors here
"""

np.set_printoptions(linewidth=80, precision=3)
logging.basicConfig(level=logging.INFO,
                    format="%(levelname)s: %(message)s")

HOST = "127.0.0.1"  # The server's hostname or IP address
PORT = 1234  # The port used by the server
DIR = "models"
nickname = "mestari-tikku"
DEFAULT_FILE = ""
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

    logging.debug(f"binarized {card_id} to {card_index}")
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
    card_id = suit + rank
    logging.debug(f"debinarized {card_index} to {card_id}")
    return card_id  # C-S A-K


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
                logging.warning("ERRONEOUS INPUT:", message, ":", received)
                continue

            cmd = parts[0]
            content = parts[1]

            logging.debug(f"recv: {cmd}")

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
                logging.info(f"Option: {content}")
                options[binarize_card(content)] = 1

            elif cmd == 'NICK':
                send(nickname)

            elif cmd == 'MSG':
                logging.info(content)

            elif cmd == 'SETTINGS':
                name, value = content.split(":")
                if name == 'SHOW':
                    if int(value):
                        logging.warning("Setting logger to show info")
                        logging.getLogger().setLevel(logging.INFO)
                    else:
                        logging.warning("Setting logger to hide info")
                        logging.getLogger().setLevel(logging.WARNING)

            elif cmd == "END":
                pass

            else:
                logging.warning(f"UNCAUGHT MESSAGE: {message}")


def send(message):
    message += "*"
    try:
        client.send(message.encode('UTF-8'))
    except OSError as e:
        logging.error(e)
        if e.errno == 10054:
            os._exit()


def decide():
    global action, hand, table, model

    # Pass of nothing fits
    if np.all(options == 0):
        logging.info("Passing")
        return 'P'

    continues = False

    # Construct model input
    model_input = np.concatenate((action, hand, table.flatten()), axis=None)
    model_input = model_input.reshape((1, 105))
    # Decide
    model_output = model.predict_single(model_input)

    has = np.multiply(model_output, hand).flatten()
    logging.info(f"Hand scrore: {np.sum(has):.2f}")
    logging.info(f"Want's to play:  {debinarize_array(has)}")
    # Realize
    realization = np.multiply(model_output, options)
    logging.info(f"Can play:        {debinarize_array(realization)}")

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
    logging.info(f"Decision: {card}, continues: {continues}")

    return card + ";" + str(int(continues))


def debinarize_array(array: np.ndarray):
    array = array.flatten()
    return (" ".join(map(lambda card_index: de_binarize_card(card_index[0]),
                         sorted(np.argwhere(array),
                                key=lambda idx: array[idx]))))


class LiteModel:

    @classmethod
    def from_file(cls, model_path):
        return LiteModel(tf.lite.Interpreter(model_path=model_path))

    @classmethod
    def from_keras_model(cls, kmodel):
        converter = tf.lite.TFLiteConverter.from_keras_model(kmodel)
        tflite_model = converter.convert()
        return LiteModel(tf.lite.Interpreter(model_content=tflite_model))

    def __init__(self, interpreter):
        self.interpreter = interpreter
        self.interpreter.allocate_tensors()
        input_det = self.interpreter.get_input_details()[0]
        output_det = self.interpreter.get_output_details()[0]
        self.input_index = input_det["index"]
        self.output_index = output_det["index"]
        self.input_shape = input_det["shape"]
        self.output_shape = output_det["shape"]
        self.input_dtype = input_det["dtype"]
        self.output_dtype = output_det["dtype"]

    def predict(self, inp):
        inp = inp.astype(self.input_dtype)
        count = inp.shape[0]
        out = np.zeros((count, self.output_shape[1]), dtype=self.output_dtype)
        for i in range(count):
            self.interpreter.set_tensor(self.input_index, inp[i:i + 1])
            self.interpreter.invoke()
            out[i] = self.interpreter.get_tensor(self.output_index)[0]
        return out

    def predict_single(self, inp: np.ndarray):
        """ Like predict(), but only for a single record. The input data can be a Python list. """
        inp = inp.astype(self.input_dtype)
        self.interpreter.set_tensor(self.input_index, inp)
        self.interpreter.invoke()
        out = self.interpreter.get_tensor(self.output_index)
        return out[0]


def load_model():
    global nickname
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
    # Load model
    with keras.utils.custom_object_scope(custom_objects):
        model_keras = tf.keras.models.load_model(model_path)
    model_keras.summary()

    print("Converting to a TensorFlow Lite model")
    start_time = timer()
    lite_model = LiteModel.from_keras_model(model_keras)
    print(f'conversion time: {timer() - start_time:.6f} seconds')

    # Converting a tf.Keras model to a TensorFlow Lite model.
    converter = tf.lite.TFLiteConverter.from_keras_model(model_keras)
    tflite_model = converter.convert()

    return lite_model


def main():
    global model, client
    # Connecting To Server
    client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client.connect(('127.0.0.1', 55555))

    model = load_model()
    # Start main loop
    receive()


if __name__ == "__main__":
    main()
