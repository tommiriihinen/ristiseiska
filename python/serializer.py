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
        self.__uniq = set()

    def serialize(self, hand, table, action, card, winning):
        ser = bitarray()

        add_to_arr(ser, hand, 'Hand')
        add_to_arr(ser, table, 'Table')
        add_to_arr(ser, action, 'Action')
        add_to_arr(ser, card, 'Card')
        add_to_arr(ser, winning, 'Score')

        assert len(ser) == ROW_BITS

        return ser

    def serialize_to_file(self, hand, table, action, card, winning):

        ser = self.serialize(hand, table, action, card, winning)

        # remove if duplicate
        if str(ser) not in self.__uniq:
            self.__uniq.add(str(ser))
            ser.tofile(self.__file)

    def close(self):
        self.__file.close()


def combine(barr):
    res = np.frombuffer(barr.unpack(), dtype='b')
    return res


def produce_feedback_array(barr):
    farr = np.zeros(52, dtype='b')
    card = bu.ba2int(barr[:-1])
    farr[card] = 1 if barr[-1] else -1
    return farr


def extract(arr, bits, operation):
    assert len(arr) == 112, f"len={len(arr)}, bits={bits}, op={operation}"
    return operation(arr[slice(*bits)])


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
        return self.__data_size // ROW_BYTES - self.__batch_size

    def __getitem__(self, batch_start):
        x_batch = np.empty((self.__batch_size, 105), dtype="b")
        y_batch = np.empty((self.__batch_size, 52), dtype="b")

        for i in range(self.__batch_size):
            row_start = (batch_start + i) * ROW_BITS
            row_end = (batch_start + i + 1) * ROW_BITS
            x = row_start + ROW_BITS
            row = self.__buffer[row_start:row_end]

            assert len(row) == 112, f"len={len(row)}, s={(batch_start + i) * ROW_BITS}, e={(batch_start + i + 1) * ROW_BITS}, x={x}"

            x_batch[i] = extract(row, (0,   105), combine)
            y_batch[i] = extract(row, (105, 112), produce_feedback_array)

        return x_batch, y_batch

    def __call__(self):
        batches = self.__data_size//self.__batch_size
        for batch in range(batches):
            yield self[batch]

    def get_unique(self):
        s = set()
        uniq = 0
        gen = []
        for i in tqdm(range(len(self))):
            row_start = i * ROW_BITS
            row_end = (i + 1) * ROW_BITS
            row = self.__buffer[row_start:row_end]
            row = row.to01()
            if row not in s:
                uniq += 1
                s.add(row)
            else:
                gen.append(row)
        print(uniq/len(self))
        return gen, uniq

    @classmethod
    def get_protocol(cls):
        s = f"Parser info:\n" \
            f" - Serial: {SERIAL}\n" \
            f" - Parse:  {PARSED}\n"

    def get_examples(self):
        return self.__data_size // ROW_BYTES

    def get_file_size(self):
        return self.__data_size


def main():
    np.set_printoptions(linewidth=1000, formatter={'all': lambda x: f"{x: }"}, threshold=2**32)
    legend = " CA C2 C3 C4 C5 C6 C7 C8 C9 CX CJ CQ CK DA D2 D3 D4 D5 D6 D7 D8 D9 DX DJ DQ DK HA H2 H3 H4 H5 H6 H7 H8 H9 HX HJ HQ HK SA S2 S3 S4 S5 S6 S7 S8 S9 SX SJ SQ SK"
    x_legend = "  " + legend + legend + " A"
    y_legend = "  " + legend

    file = "data/parsed/3ggr299k_bu.bin"
    parser = Parser(file)
    w = 10000
    l = 0
    for i in range(w, w + l):
        x_batch, y_batch = parser[i]
        print(x_batch)
        print(x_legend)
        print(y_batch)
        print(y_legend)
    print("len", len(parser))
    gen, uniq = parser.get_unique()

    print("Computed")
    print(uniq/len(parser))
    for x in gen:
        print(x)

if __name__ == "__main__":
    main()