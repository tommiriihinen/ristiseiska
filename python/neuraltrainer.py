import os
import sys
import time
import logging
import random
import serializer
from serializer import Parser

import matplotlib.pyplot as plt
import numpy as np

# TensorFlow and tf.keras
import tensorflow as tf
from keras import backend as K

DATA_DIR = 'data/parsed'
SAVE_DIR = 'models'


def squared_error_masked(y_true, y_pred):
    """ Squared error of elements where y_true is not 0 """
    err = y_pred - (K.cast(y_true, y_pred.dtype) * 1)
    return K.sum(K.square(err) * K.cast(K.not_equal(y_true, 0),
                                        y_pred.dtype), axis=-1)


def agreeableness(y_true, y_pred):
    """ Measure of agreeing to training data decisions """
    if K.sum(y_true) == 1:
        return K.argmax(y_true) == K.argmax(y_pred)
    else:
        return K.argmin(y_true) == K.argmax(y_pred)


class DataGen(tf.keras.utils.Sequence):

    def __init__(self, filepath, batch_size, shuffle):
        self.__shuffle = shuffle
        self.__parser = Parser(filepath, batch_size)
        self.__batch_size = batch_size

        self.__yield_order = list(range(len(self.__parser)))
        if shuffle:
            random.shuffle(self.__yield_order)

    def on_epoch_end(self):
        pass

    def __getitem__(self, idx):
        fetch_idx = self.__yield_order[idx]
        x_batch, y_batch = self.__parser[fetch_idx]
        return x_batch, y_batch

    def get(self, idx):
        return self[idx]

    def __len__(self):
        return len(self.__parser) // self.__batch_size

    def get_examples(self):
        return len(self.__parser)


class NeuralTrainer:

    def __init__(self, model, name="test", save_dir=SAVE_DIR):
        self.__name = name
        self.__model = model
        self.__history = {'loss': [], 'agreeableness': [],
                          'val_loss': [], 'val_agreeableness': []}
        self.__callbacks = []

        # Create necessary directories
        os.makedirs(f"{SAVE_DIR}/{name}", exist_ok=True)
        self.__logs_dir = f"{save_dir}/{name}/logs"
        os.makedirs(self.__logs_dir, exist_ok=True)

        # Init logger
        logging.basicConfig(level=logging.INFO, format="%(levelname)s: %(message)s")
        self.__log = logging.getLogger("main")
        self.__log.setLevel(logging.INFO)

        # Logging to file with <savefile>.txt as name
        model_logger_fhandler = logging.FileHandler(f"{self.__logs_dir}/{name}.txt", mode="w")
        model_logger_fhandler.setLevel(logging.INFO)
        self.__log.addHandler(model_logger_fhandler)
        self.__log.info(f"Model name: {name}")
        self.__model.summary(print_fn=self.__log.info)
        self.__log.info(Parser.get_protocol())  # Log the parser in use

    def __create_histograms(self):
        # summarize history for agreeableness
        plt.plot(self.__history['agreeableness'])
        plt.plot(self.__history['val_agreeableness'])
        plt.title('model agreeableness')
        plt.ylabel('agreeableness')
        plt.xlabel('epoch')
        plt.legend(['train', 'test'], loc='upper left')
        plt.savefig(f"{self.__logs_dir}/agreeableness.png")
        plt.close()
        # summarize history for loss
        plt.plot(self.__history['loss'])
        plt.plot(self.__history['val_loss'])
        plt.title('model loss')
        plt.ylabel('loss')
        plt.xlabel('epoch')
        plt.legend(['train', 'test'], loc='upper left')
        plt.savefig(f"{self.__logs_dir}/loss.png")
        plt.close()

    def add_callback(self, callback):
        self.__callbacks.append(callback)

    def __create_dataset(self, train_data_path, batch_size, shuffle):

        data, elements = serializer.read(train_data_path)

        def func(i):
            i = i # prevent EagerTensor object from being propagated into processing
            x, y = serializer.parse_single(i, data)
            return x, y

        z = list(range(len(data)//14))
        dataset = tf.data.Dataset.from_generator(lambda: z, tf.uint32)

        dataset = dataset.map(lambda i: tf.py_function(func=func,
                                                       inp=[i],
                                                       Tout=[tf.bool,
                                                             tf.int8]),
                              num_parallel_calls=tf.data.AUTOTUNE)
        if shuffle:
            dataset.shuffle(100)

        dataset.batch(batch_size).prefetch(tf.data.AUTOTUNE)

        return dataset, elements

    def __load_data_generators(self, train_data_file, val_data_file, batch_size, shuffle):
        # Datafiles
        train_data_path = f"{DATA_DIR}/{train_data_file}"
        val_data_path = f"{DATA_DIR}/{val_data_file}"

        self.__log.info(f"Datafiles:\n"
                        f" - Training: {train_data_file} {os.path.getsize(train_data_path) / 1024 ** 2:.2f} MB\n"
                        f" - Validation: {val_data_file} {os.path.getsize(val_data_path) / 1024 ** 2:.2f} MB\n")

        # Training data
        train_data, train_elements = self.__create_dataset(train_data_path, batch_size, shuffle)

        self.__log.info(f"Training:\n"
                        f" - Total batches: {train_elements // batch_size}\n"
                        f" - Total examples: {train_elements}\n")

        # Validation data
        val_parser = serializer.Parser(val_data_path, -1)
        val_data = val_parser[0]
        val_elements = len(val_data[0])

        self.__log.info(f"Validation:\n"
                        f" - Total examples: {val_elements}\n"
                        f" - Ratio: {val_elements / train_elements * 100:.2f}%\n")

        return train_data, val_data, train_elements

    def train(self, train_data_file="test.bin", val_data_file="test.bin",
              epochs=15, epoch_divs=1, batch_size=10, learning_rate=0.01,
              patience=None, shuffle=False, multiprocessing=False, workers=0):

        # Log parameters
        self.__log.info(f"Parameters:\n"
                        f" - Epochs: {epochs}\n"
                        f" - Batch size: {batch_size}\n"
                        f" - Learning rate: {learning_rate}\n"
                        f" - Patience: {patience}\n"
                        f" - Shuffle: {shuffle}\n")

        # Load data into generators
        train_data, val_data, train_elements = self.__load_data_generators(train_data_file, val_data_file, batch_size, shuffle)
        batches = train_elements // batch_size

        # Config model
        opt = tf.keras.optimizers.Adam(learning_rate=learning_rate)
        self.__model.compile(
            optimizer=opt,
            loss=squared_error_masked,
            metrics=agreeableness)

        # Early stopping callback
        self.__callbacks.append(tf.keras.callbacks.EarlyStopping(
            monitor="val_loss",
            mode="min",
            patience=patience,
            restore_best_weights=True))

        # Training
        training_start_time = time.process_time()
        history = self.__model.fit(
            train_data,
            validation_data=val_data,
            epochs=epochs * epoch_divs,
            steps_per_epoch=batches // epoch_divs,
            callbacks=self.__callbacks,
            use_multiprocessing=multiprocessing,
            workers=workers)
        training_end_time = time.process_time()

        self.__log.info(f"Time elapsed:\n"
                        f" - Training: {time.strftime('%Hh%Mm%Ss', time.gmtime(training_end_time - training_start_time))}\n")

        self.__log.info("Training history:")
        for key, value in history.history.items():
            self.__history[key].extend(value)
            self.__log.info(f" - {key}:  {value}")

    def save(self):
        # Save model
        self.__create_histograms()
        self.__model.save(f"{SAVE_DIR}/{self.__name}")
        print(f"Model saved in '{SAVE_DIR}' as '{self.__name}'")


def main():
    print(f"Ristiseiska NeuralTrainer running on Python {sys.version.split()[0]} and TensorFlow {tf.version.VERSION}\n")
    # print(tf.reduce_sum(tf.random.normal([1000, 1000])))

    model = tf.keras.Sequential([
        tf.keras.layers.InputLayer(input_shape=105),
        tf.keras.layers.Dense(105, activation='elu'),
        tf.keras.layers.Dense(105, activation='elu'),
        tf.keras.layers.Dense(80, activation='elu'),
        tf.keras.layers.Dense(52, activation='tanh')
    ])

    trainer = NeuralTrainer(model, "Ruby", "models")
    trainer.train(train_data_file="3ggr50k_b.bin",
                  val_data_file="test.bin",
                  epochs=15,
                  epoch_divs=1,
                  batch_size=128,
                  learning_rate=0.0001,
                  patience=5,
                  shuffle=True,
                  multiprocessing=True,
                  workers=8)
    trainer.save()


if __name__ == "__main__":
    main()
