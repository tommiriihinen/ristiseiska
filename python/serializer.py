"""

"""
import os.path

from bitarray import bitarray
import bitarray.util as bu
import numpy as np
import inspect
import dill


# This is a contract between the Serializer and the Parser
PARSED = {'Hand': np.ndarray, 'Table': np.ndarray, 'Action': bool, 'Card': int, 'Score': bool}
SERIAL = {'Hand': 52,         'Table': 52,         'Action': 1,    'Card': 6,   'Score': 1}

ROW_BYTES = sum(SERIAL.values()) // 8


def combine(barr):
    return np.frombuffer(barr.unpack(), dtype='b')


def produce_feedback_array(barr):
    farr = np.zeros(52, dtype='b')
    farr[bu.ba2int(barr[:-1])] = 1 if barr[-1] else -1
    return farr


ML_PARSE = {'x': {'bits': (0,   105), 'operation': combine},
            'y': {'bits': (105, 112), 'operation': produce_feedback_array}}


def extract(arr, bits, operation):
    return operation(arr[slice(*bits)])


def add_to_arr(arr, value, key):
    value_length = SERIAL[key]
    value_type = PARSED[key]

    if value_type == np.ndarray:
        assert len(value) == value_length
        arr.extend(value)

    elif value_type == bool:
        arr.append(value)

    elif value_type == int:
        arr += bu.int2ba(value, value_length)


class Serializer:

    def __init__(self, filepath):
        self.__file = open(filepath, "wb")

    def serialize_to_file(self, hand, table, action, card, winning):
        arr = bitarray()

        add_to_arr(arr, hand, 'Hand')
        add_to_arr(arr, table, 'Table')
        add_to_arr(arr, action, 'Action')
        add_to_arr(arr, card, 'Card')
        add_to_arr(arr, winning, 'Score')

        assert len(arr) // 8 == ROW_BYTES

        arr.tofile(self.__file)

    def close(self):
        self.__file.close()


class Parser:

    def __init__(self, filepath=None):
        self.__filepath = filepath
        if filepath:
            self.__file_size = os.path.getsize(self.__filepath)

    def __len__(self):
        return self.__file_size // ROW_BYTES

    def __str__(self):
        s = f"Parser info:\n" \
            f" - Serial: {SERIAL}\n" \
            f" - Parse:  {ML_PARSE}\n"

        for var in ML_PARSE.values():
            s += "\n" + inspect.getsource(var['operation'])

        return s

    def parse_batch(self, idx, batch_size):

        x_batch = np.empty((batch_size, 105), dtype="b")
        y_batch = np.empty((batch_size, 52), dtype="b")

        with open(self.__filepath, "rb") as file:

            for i in range(0, batch_size):
                arr = bitarray()
                arr.fromfile(file, ROW_BYTES)

                x_batch[i] = extract(arr, **ML_PARSE['x'])
                y_batch[i] = extract(arr, **ML_PARSE['y'])

        return x_batch, y_batch

    def get_file_size(self):
        return self.__file_size


# print(dill.detect.baditems(Parser("data/parsed/test.bin")))
