"""

"""
import os.path

from bitarray import bitarray
import bitarray.util as bu
import numpy as np
import inspect
from tqdm import tqdm


# This is a contract between the Serializer and the Parser
PARSED = {'Hand': np.ndarray, 'Table': np.ndarray, 'Action': bool, 'Card': int, 'Score': bool}
SERIAL = {'Hand': 52,         'Table': 52,         'Action': 1,    'Card': 6,   'Score': 1}

ROW_BYTES = sum(SERIAL.values()) // 8
ROW_BITS = ROW_BYTES * 8


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
        row = bitarray()

        add_to_arr(row, hand, 'Hand')
        add_to_arr(row, table, 'Table')
        add_to_arr(row, action, 'Action')
        add_to_arr(row, card, 'Card')
        add_to_arr(row, winning, 'Score')

        assert len(row) == ROW_BITS

        row.tofile(self.__file)

    def close(self):
        self.__file.close()


class Parser:

    def __init__(self, filepath, batch_size=1):
        self.__filepath = filepath
        self.__batch_size = batch_size

        self.__data_size = os.path.getsize(filepath)

        self.__buffer = bitarray()
        with open(filepath, "rb") as data_file:
            for i in tqdm(range(self.__data_size // ROW_BYTES)):
                arr = bitarray()
                arr.fromfile(data_file, ROW_BYTES)
                self.__buffer.extend(arr)

    def __len__(self):
        return self.__data_size // ROW_BYTES

    def __getitem__(self, batch_start):
        x_batch = np.empty((self.__batch_size, 105), dtype="b")
        y_batch = np.empty((self.__batch_size, 52), dtype="b")

        for i in range(self.__batch_size):
            row_start = (batch_start + i) * ROW_BITS
            row_end = (batch_start + i + 1) * ROW_BITS
            row = self.__buffer[row_start:row_end]

            x_batch[i] = extract(row, **ML_PARSE['x'])
            y_batch[i] = extract(row, **ML_PARSE['y'])

        assert len(x_batch[0]) == 105
        assert len(y_batch[0]) == 52
        assert len(x_batch) == self.__batch_size
        assert len(y_batch) == self.__batch_size
        assert not np.all((x_batch[-1] == 0))
        assert not np.all((y_batch[-1] == 0))

        return x_batch, y_batch

    def __call__(self):
        batches = self.__data_size//self.__batch_size
        for batch in range(batches):
            yield self[batch]

    @classmethod
    def get_protocol(cls):
        s = f"Parser info:\n" \
            f" - Serial: {SERIAL}\n" \
            f" - Parse:  {ML_PARSE}\n"

        for var in ML_PARSE.values():
            s += "\n" + inspect.getsource(var['operation'])

        return s

    def get_examples(self):
        return self.__data_size // ROW_BYTES

    def get_file_size(self):
        return self.__data_size


def main():
    np.set_printoptions(linewidth=1000, formatter={'all': lambda x: f"{x: }"}, threshold=2**32)
    legend = " CA C2 C3 C4 C5 C6 C7 C8 C9 CX CJ CQ CK DA D2 D3 D4 D5 D6 D7 D8 D9 DX DJ DQ DK HA H2 H3 H4 H5 H6 H7 H8 H9 HX HJ HQ HK SA S2 S3 S4 S5 S6 S7 S8 S9 SX SJ SQ SK"
    x_legend = "  " + legend + legend + " A"
    y_legend = "  " + legend

    file = "data/parsed/conf.bin"
    parser = Parser(file, 1)
    for i in range(0, len(parser)//10):
        x_batch, y_batch = parser[i]
        print(x_batch)
        print(x_legend)
        print(y_batch)
        print(y_legend)
    print("len", len(parser))


if __name__ == "__main__":
    main()