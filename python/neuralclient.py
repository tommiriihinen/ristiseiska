print("Loading libraries...")

import os
import socket
import sys
import codecs
import logging
import logging.handlers
from timeit import default_timer as timer
import argparse

print("Loading numpy")
import numpy as np
print("Loading tensorflow")
import tensorflow as tf
print("Loading keras")
import keras
print("Loading keras backend")
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

logging.basicConfig(filename="logs/neuralclient.txt",
                    format="%(asctime)s %(levelname)s %(message)s")


ui_handler = logging.StreamHandler(sys.stdout)
ui_handler.setFormatter(logging.Formatter("%(message)s"))

ui = logging.getLogger("ui")
ui.setLevel(logging.INFO)
ui.addHandler(ui_handler)

HOST = "127.0.0.1"  # The server's hostname or IP address
PORT = 1234  # The port used by the server
MSG_DELIMITER = "§"
MSG_DELIMITER_UTF8 = MSG_DELIMITER.encode('UTF-8')
DIR = "models"
MAX_PLAYERS = 3


def set_show_level(level):
    if level == 0:  # No UI
        ui.setLevel(logging.CRITICAL)
    elif level == 1:  # Small UI
        ui.setLevel(logging.INFO)
    elif level == 2:  # Full UI
        ui.setLevel(logging.DEBUG)


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


def debinarize_card(card_index):
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
    return card_id  # C-S A-K


def normalize_id(id, my_id):
    if id < my_id:
        return id + 1
    else:
        return id


def debinarize_array(array: np.ndarray):
    array = array.flatten()
    return (" ".join(map(lambda card_index: debinarize_card(card_index[0]),
                         sorted(np.argwhere(array),
                                key=lambda idx: array[idx]))))


class LiteModel:

    @classmethod
    def from_flatbuffer_file(cls, model_path):
        return LiteModel(tf.lite.Interpreter(model_path=model_path))

    @classmethod
    def from_keras_model_file(cls, model_path):

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
            kmodel = tf.keras.models.load_model(model_path)
        kmodel.summary()

        converter = tf.lite.TFLiteConverter.from_keras_model(kmodel)
        tflite_model = converter.convert()
        return LiteModel(tf.lite.Interpreter(model_content=tflite_model))

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


class NeuralPlayer:

    def __init__(self, model: LiteModel):
        self.model = model
        self.hand = np.zeros(52)
        self.options = np.zeros(52)

    def add_card(self, card):
        self.hand[binarize_card(card)] = 1

    def reset_cards(self):
        self.hand = np.zeros(52)

    def play(self, table, options):

        # Pass if nothing fits.
        if np.all(options == 0):
            return 'P;'

        # If can end, end fast.
        if np.array_equal(options, self.hand):
            card_ids = np.flatnonzero(options)
            if len(card_ids) == 1:
                # sweet victory
                return f"{debinarize_card(card_ids[0])};0"
            for card_id in card_ids:
                if debinarize_card(card_id)[1] in ['K', 'A']:
                    ui.info(f"Ending quick")
                    self.hand[card_id] = 0
                    table[card_id] = 1
                    return f"{debinarize_card(card_id)};1"

        # Consult the model
        card_id = self.act(1, table, options)

        self.hand[card_id] = 0
        table[card_id] = 1

        card = debinarize_card(card_id)
        return f"{card};0"

    def give(self, table):
        card_id = self.act(0, table, self.hand)

        self.hand[card_id] = 0

        card = debinarize_card(card_id)
        return f"{card};"

    def act(self, action: int, table: np.ndarray, options: np.ndarray):
        # Construct model input
        model_input = np.concatenate((action, self.hand, table.flatten()), axis=None)
        model_input = model_input.reshape((1, 105))

        # Use model to intuit
        model_output = self.model.predict_single(model_input)

        has = np.multiply(model_output, self.hand).flatten()
        ui.debug(f"Hand score: {np.sum(has):.2f}")
        ui.debug(f"Want's to play:  {debinarize_array(has)}")

        # Filter unfeasible moves
        realization = np.multiply(model_output, options)
        ui.debug(f"Can play:        {debinarize_array(realization)}")

        realization[realization == 0] = -np.inf
        decision = np.argmax(realization)

        return decision


class NeuralClient:

    def __init__(self, nickname, model):
        self.nickname = nickname
        self.__tcp_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.__tcp_socket.connect(('127.0.0.1', 55555))

        self.player = NeuralPlayer(model)

        self.table = np.zeros(52)
        self.options = np.zeros(52)

        self.id = None
        self.current_player_id = None

    def listen(self):
        dec = codecs.getincrementaldecoder('utf8')()
        while True:
            try:
                received = self.__tcp_socket.recv(2048)
                messages = dec.decode(received).split(MSG_DELIMITER)
                for message in messages:
                    if message != '':
                        self.handle_message(message)
            except OSError:
                print(e)
                os._exit()

    def handle_message(self, message):
        try:
            cmd, content = message.split(";")
        except ValueError:
            logging.warning(f"ERRONEOUS INPUT: '{message}'")
            return

        if cmd in ['WAIT', 'PROMPT', 'END']:
            pass

        elif cmd == 'PLAY':
            # Play a card
            card = self.player.play(self.table, self.options)
            self.send(card)
            self.options = np.zeros(52)
            ui.info(f"Playing: {card}")

        elif cmd == 'GIVE':

            card = self.player.give(self.table)
            self.send(card)
            self.options = np.zeros(52)
            ui.info(f"Giving: {card}")

        elif cmd == 'STARTING_CARDS':
            # Take starting card to hand
            self.player.add_card(content)

        elif cmd == 'CARD_PLAYED':
            # Put card on the table
            self.table[binarize_card(content)] = 1

        elif cmd == 'CARD_GIVEN':
            # Take card in hand
            self.player.add_card(content)

        elif cmd == 'TURN':
            current_player_id = int(content)

        elif cmd == 'ID':
            self.id = int(content)
            # Reset
            self.table = np.zeros(52)
            self.player.reset_cards()

        elif cmd == 'OPTION':
            ui.info(f"Option: {content}")
            self.options[binarize_card(content)] = 1

        elif cmd == 'NICK':
            self.send(self.nickname)

        elif cmd == 'MSG':
            ui.info(content)

        elif cmd == 'ERROR':
            logging.error(content)

        elif cmd == "KILL":
            os._exit()

        elif cmd == 'SETTINGS':
            field, value = content.split(":")
            if field == 'SHOW':
                set_show_level(int(value))

        else:
            logging.error(f"UNCAUGHT MESSAGE: {message}")

    def send(self, message):
        try:
            self.__tcp_socket.send((message + MSG_DELIMITER).encode('UTF-8'))
        except OSError as e:
            print(e)
            os._exit()


def main():
    print(f"Ristiseiska NeuralClient running on Python {sys.version.split()[0]} and TensorFlow {tf.version.VERSION}\n")

    parser = argparse.ArgumentParser(description='Create ANN powered Ristiseiska Client')
    parser.add_argument('-output', help="Output level", type=int, choices=[0, 1, 2], default=1)
    parser.add_argument('-model', help="Model")
    args = parser.parse_args()

    set_show_level(int(args.output))

    model_directory = args.model
    if not model_directory:

        print("Directory " + DIR + " contents:")
        for filename in os.scandir(DIR):
            if not filename.is_file():
                print(f" {filename.name}")
        model_directory = input("Which model to use: ")

    model_path = DIR + "/" + str(model_directory)
    print("Loading " + model_path)
    model = LiteModel.from_keras_model_file(model_path)

    nickname = model_directory

    client = NeuralClient(nickname, model)
    client.listen()


if __name__ == "__main__":
    main()
