import os
import sys
import time
import logging
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


class DataGen:

    def __init__(self, filepath, batch_size, shuffle=True):
        self.__parser = Parser(filepath)
        self.__batch_size = batch_size
        self.__shuffle = shuffle

        self.__n = len(self.__parser)
        self.__timespent = 0

    def __len__(self):
        return self.__n // self.__batch_size

    def __getitem__(self, idx):
        start = time.process_time()
        x_batch, y_batch = self.__parser.parse_batch(idx * self.__batch_size,
                                                     self.__batch_size)
        self.__timespent += time.process_time() - start
        return x_batch, y_batch

    def __call__(self):
        for i in range(len(self)):
            yield self[i]

            if i == len(self)-1:
                self.on_epoch_end()

    def on_epoch_end(self):
        pass

    def get_parser(self):
        return self.__parser

    def get_time_spent(self):
        return self.__timespent

    def get_n(self):
        return self.__n

    def to_ds(self):
        out_sign = tf.TensorSpec([None, 105]), tf.TensorSpec([None, 52])
        ds = tf.data.Dataset.from_generator(self, output_signature=out_sign)
        ds = ds.prefetch(tf.data.AUTOTUNE)
        return ds


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
        self.__log.info(Parser())  # Log the parser in use

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

    def train(self,
              train_data_file="test.bin",
              val_data_file="test.bin",
              epochs=5,
              batch_size=1000,
              learning_rate=0.001,
              patience=5,
              shuffle=True,
              multiprocessing=True,
              workers=1):

        # Create data generators
        train_gen = DataGen(f"{DATA_DIR}/{train_data_file}", batch_size, shuffle)
        val_gen = DataGen(f"{DATA_DIR}/{val_data_file}", batch_size, shuffle)
        # Wrap generators in tf.Dataset
        train_ds = train_gen.to_ds()
        val_ds = val_gen.to_ds()

        # Log
        self.__log.info(f"Datafiles:\n"
                        f" - Training: {train_data_file} {train_gen.get_parser().get_file_size() / 1024 ** 2:.2f} MB\n"
                        f" - Validation: {val_data_file} {val_gen.get_parser().get_file_size() / 1024 ** 2:.2f} MB\n")
        self.__log.info(f"Parameters:\n"
                        f" - Epochs: {epochs}\n"
                        f" - Batch size: {batch_size}\n"
                        f" - Learning rate: {learning_rate}\n"
                        f" - Patience: {patience}\n")
        self.__log.info(f"Training:\n"
                        f" - Total batches: {len(train_gen)}\n"
                        f" - Total examples: {train_gen.get_n()}\n")
        self.__log.info(f"Validation:\n"
                        f" - Total batches: {len(val_gen)}\n"
                        f" - Total examples: {val_gen.get_n()}\n"
                        f" - Ratio: {val_gen.get_n() / train_gen.get_n()}\n")

        # Config model
        opt = tf.keras.optimizers.Adam(learning_rate=learning_rate)

        self.__model.compile(optimizer=opt,
                             loss=squared_error_masked,
                             metrics=agreeableness)

        earlystopping = tf.keras.callbacks.EarlyStopping(monitor="val_loss",
                                                         mode="min",
                                                         patience=patience,
                                                         restore_best_weights=True)
        self.__callbacks.append(earlystopping)

        # Training
        training_start_time = time.process_time()
        history = self.__model.fit(train_ds,
                                   validation_data=val_ds,
                                   epochs=epochs,
                                   callbacks=self.__callbacks,
                                   use_multiprocessing=multiprocessing,
                                   workers=workers,
                                   verbose=2)
        training_end_time = time.process_time()

        self.__log.info("Training history:")
        for key, value in history.history.items():
            self.__history[key].extend(value)
            self.__log.info(f" - {key}: {value}")

        self.__log.info(f"Time elapsed:\n"
                        f" - Training: {time.strftime('%Hh%Mm%Ss', time.gmtime(training_end_time - training_start_time))}\n"
                        f" - Parsing:  {time.strftime('%Hh%Mm%Ss', time.gmtime(train_gen.get_time_spent() + val_gen.get_time_spent()))}\n")

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
    trainer.train(train_data_file="3ggr1M.bin",
                  val_data_file="3ggr50k.bin",
                  epochs=20,
                  batch_size=1024,
                  learning_rate=0.0001,
                  patience=5,
                  shuffle=True,
                  multiprocessing=True,
                  workers=8)
    trainer.save()


if __name__ == "__main__":
    main()
