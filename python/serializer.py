"""

"""
import os.path

from bitarray import bitarray
import bitarray.util as bu
import numpy as np
import inspect


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
        print(arr)

        arr.tofile(self.__file)

    def close(self):
        self.__file.close()


class Parser:

    def __init__(self, filepath=None):
        if filepath:
            self.__file_size = os.path.getsize(filepath)

            with open(filepath, "rb") as file:
                print("Reading binary")
                self.__data = bitarray()
                self.__data.fromfile(file, -1)
                print("Read")

    def __len__(self):
        return self.__file_size // ROW_BYTES

    def __str__(self):
        pass

    def parse_batch(self, idx, batch_size):

        x_batch = np.empty((batch_size, 105), dtype="b")
        y_batch = np.empty((batch_size, 52), dtype="b")

        for i in range(idx):
            arr = self.__data[i * ROW_BYTES : (i+1) * ROW_BYTES + 1]

            x_batch[i] = extract(arr, **ML_PARSE['x'])
            y_batch[i] = extract(arr, **ML_PARSE['y'])

        return x_batch, y_batch

    def parse_single(self, i: int):
        idx = (1*90 + 10)//100
        a = idx
        start = a * 112
        b = idx+1
        end = b * 112
        assert start < end, f"start {start}, end {end}"
        diff = end - start
        arr = self.__data[start:end]
        assert len(arr) == 112, f"len: {len(self)}, idx: {idx}, start {start}, end {end}, diff {diff}, arr {len(arr)}, a {a}, b {b}"
        return extract(arr, **ML_PARSE['x']), extract(arr, **ML_PARSE['y'])

    def get_file_size(self):
        return self.__file_size


def parse(data, idx):
    start = idx * 112
    end = (idx+1) * 112
    assert start < end, f"start {start}, end {end}"
    diff = end - start
    arr = data[start:end]
    assert len(arr) == 112, f"len: {len(data)}, idx: {idx}, start {start}, end {end}, diff {diff}, arr {len(arr)}, a {a}, b {b}"
    return extract(arr, **ML_PARSE['x']), extract(arr, **ML_PARSE['y'])


def protocol():
    s = f"Parser info:\n" \
        f" - Serial: {SERIAL}\n" \
        f" - Parse:  {ML_PARSE}\n"

    for var in ML_PARSE.values():
        s += "\n" + inspect.getsource(var['operation'])

    return s


def main():
    np.set_printoptions(linewidth=1000, formatter={'all': lambda x: f"{x: }"}, threshold=2**32)
    legend = " CA C2 C3 C4 C5 C6 C7 C8 C9 CX CJ CQ CK DA D2 D3 D4 D5 D6 D7 D8 D9 DX DJ DQ DK HA H2 H3 H4 H5 H6 H7 H8 H9 HX HJ HQ HK SA S2 S3 S4 S5 S6 S7 S8 S9 SX SJ SQ SK"
    x_legend = " " + legend + legend + " A"
    y_legend = " " + legend

    file = "data/parsed/conf.bin"
    parser = Parser(file)
    for i in range(0, len(parser)):
        x_batch, y_batch = (parser.parse_single(i))
        print(x_batch)
        print(x_legend)
        print(y_batch)
        print(y_legend)
    print("len", len(parser))


if __name__ == "__main__":
    main()