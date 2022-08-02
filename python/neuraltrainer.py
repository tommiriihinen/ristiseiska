# import required module
import copy
import os
import sys
import csv
from tqdm import tqdm

import matplotlib.pyplot as plt
import pandas as pd
import numpy as np

# Make numpy values easier to read.
np.set_printoptions(linewidth=1000, formatter={'all': lambda x: str(x) + ","}, threshold=sys.maxsize)

# TensorFlow and tf.keras
import tensorflow as tf
from keras import backend as K
import keras

DATA_DIR = 'data'
SAVE_DIR = 'models'
DEFAULT_DATA = "test.csv"
DEFAULT_SAVE = "test"
DEFAULT_EPOCHS = 50
DEFAULT_BATCH = 10
ALLOWED_PLAYERS = [3]

"""
Reducing overfitting
    1. Holdout
        80/20
        
    2. Cross-validation
        Not implemented
        
    3. Data augmentation
        Enough data already
        
    4. Feature selection
        N/A: monolithic data
        
    5. L1 / L2 reqularization
        Not implemented
        
    6. Remove layers / units per layer
        Researching
        
    7. Dropout
        0.2
        
    8. Early stopping
        Implemented
"""


def create_model():
    input_size = 1 + 52 + (len(ALLOWED_PLAYERS) + 2) * 52
    model = tf.keras.Sequential([
        tf.keras.layers.InputLayer(input_shape=input_size),
        tf.keras.layers.Dense(384, activation='elu'),
        tf.keras.layers.Dropout(0.2),
        tf.keras.layers.Dense(384, activation='elu'),
        tf.keras.layers.Dropout(0.2),
        tf.keras.layers.Dense(256, activation='elu'),
        tf.keras.layers.Dropout(0.2),
        tf.keras.layers.Dense(128, activation='elu'),
        tf.keras.layers.Dropout(0.2),
        tf.keras.layers.Dense(52, activation='tanh')
    ])

    def accuracy(y_true, y_pred):
        if K.sum(y_true) == 1:
            return K.argmax(y_true) == K.argmax(y_pred)
        else:
            return K.argmin(y_true) == K.argmin(y_pred)

    opt = keras.optimizers.Adam(learning_rate=0.0001)
    model.compile(optimizer=opt,
                  loss=squared_error_masked,
                  metrics=accuracy)

    model.summary()
    return model


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


# parts      [0]        [1]               [2]          [3]
# game_text: (n players);n*(startingHand,);m*(action,);(winner)

def parse_game(game, accepted_sizes, training_inputs, training_outputs):
    winning_examples = 0
    losing_examples = 0

    # print(game)

    players = int(game['Player Count'])
    winner = int(game['Winner'])

    if players not in accepted_sizes:
        return

    # Binarize starting hands
    starting_hands = np.zeros((players, 52), dtype="b")
    hand_data = game['Starting Hands'].split(";")  # P1 C2 C7 DA ... ,P2 CA C3 D4 ...

    for player in range(0, players):
        card_ids = hand_data[player].strip().split(" ")  # P1 C2 C7 DA ...
        player_id = card_ids.pop(0)  # P[1]

        for card_id in card_ids:
            starting_hands[player][binarize_card_id(card_id)] = 1

    # starting hands
    running_hands = starting_hands.copy()
    # empty table
    running_tables = np.zeros((len(ALLOWED_PLAYERS) + 2, 52), dtype="b")

    # start parsing
    turn_data = game['Actions'].split(";")
    for unit in turn_data[:-1]:
        contents = unit.split(" ")
        player = int(contents[1])
        action = 1 if (contents[0] == 'P') else 0
        card = binarize_card_id(contents[2])
        hand = running_hands[player]

        inputs = np.concatenate((action, hand, running_tables.flatten()), dtype="b", axis=None)
        outputs = np.zeros(52, dtype="b")

        if winner == player:
            outputs[card] = 1
            training_inputs.append(inputs)
            training_outputs.append(outputs)
            winning_examples += 1
        else:
            if winning_examples > losing_examples:
                outputs[card] = -1
                training_inputs.append(inputs)
                training_outputs.append(outputs)
                losing_examples += 1

        # print("action: ", unit)
        # print("inputs: ", inputs)
        # print("outputs:", outputs)
        # print("legend:   A,", 7*"CA C2 C3 C4 C5 C6 C7 C8 C9 CX CJ CQ CK DA D2 D3 D4 D5 D6 D7 D8 D9 DX DJ DQ DK HA H2 H3 H4 H5 H6 H7 H8 H9 HX HJ HQ HK SA S2 S3 S4 S5 S6 S7 S8 S9 SX SJ SQ SK ")

        if action:  # is play
            running_hands[player][card] = 0
            running_tables[player][card] = 1
        else:  # is give
            last_player = (player - 1) % players
            running_hands[player][card] = 0
            running_hands[last_player][card] = 1

        # hands = sum(sum(x) for x in running_hands)
        # table = sum(sum(x) for x in running_tables)
        # print(hands, table, hands + table)


def decomment(csvfile):
    for row in csvfile:
        raw = row.split('#')[0].strip()
        if raw: yield raw


def load_data(filename):
    allowed_players = ALLOWED_PLAYERS

    features = []
    labels = []

    path = "/".join([DATA_DIR, filename])

    # Find file length
    with open(path) as csvfile:
        lines = len(csvfile.readlines())

    with open(path) as csvfile:
        reader = csv.DictReader(decomment(csvfile))
        for i, row in enumerate(tqdm(reader, total=lines)):
            parse_game(row, allowed_players, features, labels)

    return np.array(features, dtype="b"), np.array(labels, dtype="b")


def squared_error_masked(y_true, y_pred):
    """ Squared error of elements where y_true is not 0 """
    err = y_pred - (K.cast(y_true, y_pred.dtype) * 1)
    return K.sum(K.square(err) * K.cast(K.not_equal(y_true, 0),
                                        y_pred.dtype), axis=-1)


def plot(history):
    print(history.history.keys())
    # summarize history for loss
    plt.plot(history.history['loss'])
    plt.plot(history.history['val_loss'])
    plt.title('model loss')
    plt.ylabel('loss')
    plt.xlabel('epoch')
    plt.legend(['train', 'test'], loc='upper left')
    plt.show()


def batch_generator(Train_df, batch_size, steps):
    idx = 1
    while True:
        yield load_data(Train_df, idx - 1, batch_size)  ## Yields data
        if idx < steps:
            idx += 1
        else:
            idx = 1


def main():
    print(f"Ristiseiska NeuralTrainer running on Python {sys.version.split()[0]} and TensorFlow {tf.version.VERSION}\n")
    # print(tf.reduce_sum(tf.random.normal([1000, 1000])))

    print("Available datasets:")
    for filename in os.scandir(DATA_DIR):
        if filename.is_file():
            print(f" {filename.name}")

    datafile = input("which file to train from: ")
    savefile = input("what to save the model as: ")
    try:
        epochs = int(input("How many epochs to train:"))
    except ValueError:
        epochs = DEFAULT_EPOCHS
    try:
        batch_size = int(input("How large will the batch size be:"))
    except ValueError:
        batch_size = DEFAULT_BATCH
    if not datafile:
        datafile = DEFAULT_DATA
    if not savefile:
        savefile = DEFAULT_SAVE

    print("Parsing data")
    training_features, training_labels = load_data(datafile)
    print("Parsing complete")

    model = create_model()
    print("Model compiled")

    print("Size of training features: ", training_features.size)
    print("Size if training labels:   ", training_labels.size)
    print("Training data length:      ", len(training_features))
    print("Training feature unit size:", training_features[0].size)
    print("Training label unit size:  ", training_labels[0].size)
    print("Feature datatype:          ", training_features.dtype)
    print("Label   datatype:          ", training_labels.dtype)

    checkpoint_path = "models/cp.ckpt"
    checkpoint_dir = os.path.dirname(checkpoint_path)

    # Create a callback that saves the model's weights
    cp_callback = tf.keras.callbacks.ModelCheckpoint(filepath=checkpoint_path,
                                                     save_weights_only=True,
                                                     verbose=1)

    earlystopping = tf.keras.callbacks.EarlyStopping(monitor="val_loss",
                                                     mode="min", patience=2,
                                                     restore_best_weights=True)

    # Train the model with the new callback
    history = model.fit(training_features,
                        training_labels,
                        epochs=epochs,
                        batch_size=batch_size,
                        validation_split=0.10,
                        callbacks=[cp_callback, earlystopping])

    save_path = "/".join([SAVE_DIR, savefile])
    model.save(save_path)
    print(f"Model saved in '{SAVE_DIR}' as '{savefile}'")

    plot(history)


if __name__ == "__main__":
    main()
