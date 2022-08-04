# import required module
import os
import sys
import csv
import time
import logging
from datetime import timedelta
from tqdm import tqdm
import pyinputplus as pyip
import matplotlib.pyplot as plt
import numpy as np

# TensorFlow and tf.keras
import tensorflow as tf
from keras import backend as K
import keras

# Make numpy values easier to read.
np.set_printoptions(linewidth=1000, formatter={'all': lambda x: str(x) + ","}, threshold=sys.maxsize)


DATA_DIR = 'data'
SAVE_DIR = 'models'
LOGS_DIR = 'logs'
DEFAULT_DATA = "test.csv"
DEFAULT_SAVE = "test"

logging.basicConfig(level=logging.INFO)
# logging.basicConfig(filename="logs/logs.txt", level=logging.WARNING, format="%(asctime)s %(levelname)s %(message)s")

DEFAULT_EPOCHS = 50
DEFAULT_BATCH = 10
LEARNING_RATE = 0.001
VALIDATION_SPLIT = 0.2
PATIENCE = 3


def train(features, labels, epochs, batch_size):

    # Create model
    model = tf.keras.Sequential([
        tf.keras.layers.InputLayer(input_shape=105),
        tf.keras.layers.Dense(105, activation='elu'),
        tf.keras.layers.Dense(80, activation='elu'),
        tf.keras.layers.Dense(52, activation='tanh')
    ])

    opt = keras.optimizers.Adam(learning_rate=LEARNING_RATE)

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
    history = model.fit(features,
                        labels,
                        epochs=epochs,
                        batch_size=batch_size,
                        validation_split=VALIDATION_SPLIT,
                        callbacks=[cp_callback, earlystopping])

    return model, history


def binarize_card_id(card_id):
    suit = 0
    if card_id[0] == 'C':
        suit = 0
    elif card_id[0] == 'D':
        suit = 1
    elif card_id[0] == 'H':
        suit = 2
    elif card_id[0] == 'S':
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

    return (suit * 13 + rank) - 1  # 0-51


def parse_game(game, training_inputs, training_outputs):
    winning_examples = 0
    losing_examples = 0

    players = int(game['Player Count'])
    winner = int(game['Winner'])

    # Binarize starting hands
    starting_hands = np.zeros((players, 52), dtype="b")
    hand_data = game['Starting Hands'].split(";")  # P1 C2 C7 DA ... ,P2 CA C3 D4 ...

    for player in range(0, players):
        card_ids = hand_data[player].strip().split(" ")  # P1 C2 C7 DA ...
        player_id = card_ids.pop(0)  # P[1]

        for card_id in card_ids:
            starting_hands[player][binarize_card_id(card_id)] = 1

    # Init running hands and table
    running_hands = starting_hands.copy()
    running_table = np.zeros(52, dtype="b")

    # Start parsing
    turn_data = game['Actions'].split(";")
    for unit in turn_data[:-1]:
        contents = unit.split(" ")
        player = int(contents[1])
        action = 1 if (contents[0] == 'P') else 0
        card = binarize_card_id(contents[2])
        hand = running_hands[player]

        inputs = np.concatenate((action, hand, running_table.flatten()), dtype="b", axis=None)
        outputs = np.zeros(52, dtype="b")

        if winner == player:
            outputs[card] = 1
        else:
            outputs[card] = -1

        training_inputs.append(inputs)
        training_outputs.append(outputs)

        if action:  # is play
            running_hands[player][card] = 0
            running_table[card] = 1
        else:  # is give
            last_player = (player - 1) % players
            running_hands[player][card] = 0
            running_hands[last_player][card] = 1


def decomment(csvfile):
    for row in csvfile:
        raw = row.split('#')[0].strip()
        if raw: yield raw


def load_data(filename):
    features = []
    labels = []

    path = "/".join([DATA_DIR, filename])

    # Find file length
    line_count = -1
    with open(path) as csvfile:
        for _ in csv.reader(decomment(csvfile)):
            line_count += 1

    with open(path) as csvfile:
        reader = csv.DictReader(decomment(csvfile))
        for i, row in enumerate(tqdm(reader, total=line_count)):
            parse_game(row, features, labels)

    return np.array(features, dtype="b"), np.array(labels, dtype="b")


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


def main():
    print(f"Ristiseiska NeuralTrainer running on Python {sys.version.split()[0]} and TensorFlow {tf.version.VERSION}\n")
    # print(tf.reduce_sum(tf.random.normal([1000, 1000])))

    # Create logger
    log = logging.getLogger("main")
    log.setLevel(logging.INFO)

    # Show available data
    print("Available datasets:")
    for filename in os.scandir(DATA_DIR):
        if filename.is_file():
            print(f" {filename.name}")

    # Ask for training parameters
    datafile = pyip.inputStr("Datafile: ", default=DEFAULT_DATA, limit=1)
    savefile = pyip.inputStr("Savefile: ", default=DEFAULT_SAVE, limit=1)
    epochs = pyip.inputInt("Epochs: ", default=DEFAULT_EPOCHS, limit=1)
    batch_size = pyip.inputInt("Batch size: ", default=DEFAULT_BATCH, limit=1)

    # Parse input data
    print("Parsing data:")
    parsing_start_time = time.process_time()
    training_features, training_labels = load_data(datafile)
    parsing_end_time = time.process_time()

    # Log input data
    log.info(f"Features size:      {training_labels.size}")
    log.info(f"Labels size:        {training_labels.size}")
    log.info(f"Data length:        {len(training_features)}")
    log.info(f"Feature unit size:  {training_features[0].size}")
    log.info(f"Label unit size:    {training_labels[0].size}")
    log.info(f"Feature datatype:   {training_features.dtype}")
    log.info(f"Label datatype:     {training_labels.dtype}")

    # Train model
    training_start_time = time.process_time()
    model, history = train(training_features,
                           training_labels,
                           epochs,
                           batch_size)
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

    # Log model training
    log.info(f"Name: {savefile}")
    model.summary(print_fn=log.info)

    log.info(f"Datafile: {datafile}")
    log.info(f"Parameters:")
    log.info(f"  Epochs: {epochs}")
    log.info(f"  Batch size: {batch_size}")
    log.info(f"  Validation split: {VALIDATION_SPLIT}")
    log.info(f"  Learning rate: {LEARNING_RATE}")
    log.info(f"  Patience: {PATIENCE}")
    log.info(f"Time elapsed:")
    log.info(f"  Parsing:  {time.strftime('%Hh%Mm%Ss', time.gmtime(parsing_end_time - parsing_start_time))}")
    log.info(f"  Training: {time.strftime('%Hh%Mm%Ss', time.gmtime(training_end_time - training_start_time))}")

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
    plt.show()
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
