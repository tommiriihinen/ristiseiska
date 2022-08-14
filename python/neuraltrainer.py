import os
import sys
import csv
import time
import logging
from tqdm import tqdm
from serializer import Parser
import dill

import pyinputplus as pyip
import matplotlib.pyplot as plt
import numpy as np

# TensorFlow and tf.keras
import tensorflow as tf
from keras import backend as K
import keras

# Make numpy values easier to read.
np.set_printoptions(linewidth=1000, formatter={'all': lambda x: str(x) + ","}, threshold=sys.maxsize)


DATA_DIR = 'data/parsed'
SAVE_DIR = 'models'
LOGS_DIR = 'logs'
DEFAULT_TRAIN_DATA = "test.bin"
DEFAULT_VAL_DATA = "test.bin"
DEFAULT_SAVE = "def"

logging.basicConfig(level=logging.INFO, format="%(levelname)s: %(message)s")
log = logging.getLogger("main")
log.setLevel(logging.INFO)
# logging.basicConfig(filename="logs/logs.txt", level=logging.WARNING, format="%(asctime)s %(levelname)s %(message)s")

DEFAULT_STEPS_PER_EPOCH = 5
DEFAULT_EPOCHS = 10
DEFAULT_BATCH = 100
DEFAULT_LEARNING_RATE = 0.0001
PATIENCE = 5


def train(train_gen, val_gen, epochs, batch_size, learning_rate):

    # Create model
    model = tf.keras.Sequential([
        tf.keras.layers.InputLayer(input_shape=105),
        tf.keras.layers.Dense(105, activation='elu'),
        tf.keras.layers.Dense(105, activation='elu'),
        tf.keras.layers.Dense(80, activation='elu'),
        tf.keras.layers.Dense(52, activation='tanh')
    ])

    opt = keras.optimizers.Adam(learning_rate=learning_rate)

    model.compile(optimizer=opt,
                  loss=squared_error_masked,
                  metrics=agreeableness)

    # Training callbacks
    checkpoint_path = "/".join([SAVE_DIR, "cp.ckpt"])
    cp_callback = tf.keras.callbacks.ModelCheckpoint(filepath=checkpoint_path,
                                                     save_weights_only=True,
                                                     verbose=1)

    earlystopping = tf.keras.callbacks.EarlyStopping(monitor="val_loss",
                                                     mode="min", patience=PATIENCE,
                                                     restore_best_weights=True)

    # Training
    training_start_time = time.process_time()
    history = model.fit(train_gen,
                        validation_data=val_gen,
                        epochs=epochs,
                        callbacks=[cp_callback, earlystopping],
                        use_multiprocessing=True)

    return model, history, training_start_time


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

    def __init__(self, filepath, batch_size):
        self.__parser = Parser(filepath)
        self.__batch_size = batch_size

        self.__n = len(self.__parser)
        self.__timespent = 0

    def on_epoch_end(self):
        pass

    def __getitem__(self, index):
        start = time.process_time()

        self.__parser.seek(index * self.__batch_size)

        x_batch = np.empty((self.__batch_size, 105), dtype="b")
        y_batch = np.empty((self.__batch_size, 52), dtype="b")
        for i in range(0, self.__batch_size):
            try:
                x_batch[i], y_batch[i] = self.__parser.parse_next()
            except EOFError as e:
                print(index, i, e)

        end = time.process_time()
        self.__timespent += end - start
        return x_batch, y_batch

    def __len__(self):
        return self.__n // self.__batch_size

    def get_parser(self):
        return self.__parser

    def get_time_spent(self):
        return self.__timespent

    def get_n(self):
        return self.__n


def main():
    print(f"Ristiseiska NeuralTrainer running on Python {sys.version.split()[0]} and TensorFlow {tf.version.VERSION}\n")
    # print(tf.reduce_sum(tf.random.normal([1000, 1000])))

    # Show available data
    print("Available datasets:")
    for filename in os.scandir(DATA_DIR):
        if filename.is_file():
            print(f" {filename.name}")

    # Ask for training parameters
    train_data_file = pyip.inputStr("Training data: ", default=DEFAULT_TRAIN_DATA, limit=1)
    val_data_file = pyip.inputStr("Validation data: ", default=DEFAULT_VAL_DATA, limit=1)
    savefile = pyip.inputStr("Savefile: ", default=DEFAULT_SAVE, limit=1)
    epochs = pyip.inputInt("Epochs: ", default=DEFAULT_EPOCHS, limit=1)
    batch_size = pyip.inputInt("Batch size: ", default=DEFAULT_BATCH, limit=1)
    learning_rate = pyip.inputFloat("Learning rate: ", default=DEFAULT_LEARNING_RATE, limit=1)

    process_start_time = time.process_time()

    # Parse input data
    train_gen = DataGen(f"{DATA_DIR}/{train_data_file}", batch_size)
    val_gen = DataGen(f"{DATA_DIR}/{val_data_file}", batch_size)

    print(dill.detect.baditems(train_gen))

    # Train model
    model, history, training_start_time = train(train_gen,
                                                val_gen,
                                                epochs,
                                                batch_size,
                                                learning_rate)
    training_end_time = time.process_time()

    # Saving model
    save_path = "/".join([SAVE_DIR, savefile])
    model.save(save_path)
    print(f"Model saved in '{SAVE_DIR}' as '{savefile}'")

    # Make directory for logs in model directory
    model_logs_dir = "/".join([SAVE_DIR, savefile, LOGS_DIR])
    try:
        os.mkdir(model_logs_dir)
    except OSError:
        pass

    # Logging to file with <savefile>.txt as name
    logs_path = "/".join([model_logs_dir, savefile + ".txt"])
    model_logger_fhandler = logging.FileHandler(logs_path, mode="w")
    model_logger_fhandler.setLevel(logging.INFO)
    log.addHandler(model_logger_fhandler)
    log.info(f"Model name: {savefile}")

    # Log model training
    model.summary(print_fn=log.info)

    log.info(train_gen.get_parser())
    log.info(f"Datafiles:\n"
             f" - Training: {train_data_file} {train_gen.get_parser().get_file_size()/1024**2:.2f} MB\n"
             f" - Validation: {val_data_file} {train_gen.get_parser().get_file_size()/1024**2:.2f} MB\n")
    log.info(f"Parameters:\n"
             f" - Epochs: {epochs}\n"
             f" - Batch size: {batch_size}\n"
             f" - Learning rate: {learning_rate}\n"
             f" - Patience: {PATIENCE}\n")
    log.info(f"Training:\n"
             f" - Total batches: {len(train_gen)}\n"
             f" - Total examples: {train_gen.get_n()}\n")
    log.info(f"Validation:\n"
             f" - Total batches: {len(val_gen)}\n"
             f" - Total examples: {val_gen.get_n()}\n"
             f" - Ratio: {val_gen.get_n()/train_gen.get_n()}\n")
    log.info(f"Time elapsed:\n"
             f" - Training: {time.strftime('%Hh%Mm%Ss', time.gmtime(training_end_time - training_start_time))}\n"
             f" - Parsing:  {time.strftime('%Hh%Mm%Ss', time.gmtime(train_gen.get_time_spent() + val_gen.get_time_spent()))}\n"
             f" - Total:    {time.strftime('%Hh%Mm%Ss', time.gmtime(training_end_time - process_start_time))}\n")

    model_logger_fhandler.close()
    logging.shutdown()

    # Plot training history
    agreeableness_plot_path = "/".join([model_logs_dir, "agreeableness.png"])
    loss_plot_path = "/".join([model_logs_dir, "loss.png"])
    # summarize history for agreeableness
    plt.plot(history.history['agreeableness'])
    plt.plot(history.history['val_agreeableness'])
    plt.title('model agreeableness')
    plt.ylabel('agreeableness')
    plt.xlabel('epoch')
    plt.legend(['train', 'test'], loc='upper left')
    plt.savefig(agreeableness_plot_path)
    plt.close()
    # summarize history for loss
    plt.plot(history.history['loss'])
    plt.plot(history.history['val_loss'])
    plt.title('model loss')
    plt.ylabel('loss')
    plt.xlabel('epoch')
    plt.legend(['train', 'test'], loc='upper left')
    plt.savefig(loss_plot_path)
    plt.show()


if __name__ == "__main__":
    main()
