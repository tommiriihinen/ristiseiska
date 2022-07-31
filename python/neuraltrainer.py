# import required module
import copy
import os

import pandas as pd
import numpy as np

# Make numpy values easier to read.
np.set_printoptions(precision=3, suppress=True)

# TensorFlow and tf.keras
import tensorflow as tf
from tensorflow.keras import backend as K
from tensorflow.keras import layers

DIR = 'data'
# Helper libraries
print(f"Ristiseiska NeuralTrainer running on Python {sys.version.split()[0]} and TensorFlow {tf.version.VERSION}\n")

print(tf.reduce_sum(tf.random.normal([1000, 1000])))


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


class Turn:  # A Decision unit
    player = 0
    tables = []  # n * [52]
    hand = []  # [52]
    action = 1  # 1 = Play, 0 Give
    decision = [0] * 52  # [52]

    def __repr__(self):
        return "PLAYER:  " + str(self.player) + "\n" \
               "TABLES: " + str(self.tables) + "\n" \
               "HAND:    " + str(self.hand) + "\n"\
               "ACTION:  " + str(self.action) + "\n"\
               "DECISION:" + str(self.decision) + "\n" \
               "LEGEND:   CA C2 C3 C4 C5 C6 C7 C8 C9 CX CJ CQ CK DA D2 D3 D4 D5 D6 D7 D8 D9 DX DJ DQ DK HA H2 H3 H4 H5 H6 H7 H8 H9 HX HJ HQ HK SA S2 S3 S4 S5 S6 S7 S8 S9 SX SJ SQ SK\n"


class Game:  # Series of decision units tied to a single reward
    players = 0  # 3-7
    winner = 0
    turns = []


# parts      [0]        [1]               [2]          [3]
# game_text: (n players);n*(startingHand,);m*(action,);(winner)

def parse_game(game_text, training_inputs, training_outputs):
    game_parts = game_text.split(";")
    players = int(game_parts[0])
    winner = int(game_parts[3])

    # Binarize starting hands
    starting_hands = []
    hand_data = game_parts[1].split(",")  # P1 C2 C7 DA ... ,P2 CA C3 D4 ...
    for player in range(0, players):
        starting_hand = [0] * 52
        card_ids = hand_data[player].strip().split(" ")  # P1 C2 C7 DA ...
        player_id = card_ids.pop(0)  # P[1]

        for card_id in card_ids:
            starting_hand[binarize_card_id(card_id)] = 1
        starting_hands.append(starting_hand)

    # starting hands
    running_hands = starting_hands.copy()
    # empty table
    running_tables = [([0] * 52) for _ in range(7)]

    # start parsing
    turn_data = game_parts[2].split(",")
    for unit in turn_data[:-1]:
        contents = unit.split(" ")
        player = int(contents[1])
        action = 1 if (contents[0] == 'P') else 0
        card = binarize_card_id(contents[2])

        inputs = [action]
        inputs += running_hands[player]
        table_copy = copy.deepcopy(running_tables)
        inputs += table_copy.pop(0)
        for table in table_copy:
            inputs += table

        outputs = [0] * 52
        outputs[card] = 1 if winner == player else -1

        training_inputs.append(inputs)
        training_outputs.append(outputs)

        if action:  # is play
            running_hands[player][card] = 0
            running_tables[player][card] = 1
        else:       # is give
            next_player = (player + 1) % players
            running_hands[player][card] = 0
            running_hands[next_player][card] = 1

        # hands = sum(sum(x) for x in running_hands)
        # table = sum(sum(x) for x in running_tables)
        # print(hands, table, hands + table)




print("Directory: " + DIR + ", contents:")
for filename in os.scandir(DIR):
    if filename.is_file():
        print(filename.path)
filename = input("which file to train from: ")
savefile = input("what to save the model as: ")
file = open(DIR + "\\" + filename)
data = ""
for line in file:
    data += line

games = data.split("/")
metadata = games.pop(0)

training_features = []
training_labels = []
step = 0
length = len(games)
print("Parsing data")
for game in games:
    step += 1
    parse_game(game, training_features, training_labels)
    if (step % 1000 == 2):
        print(str(step/length*100) + "% complete")
print("Parsing complete")

training_features = np.array(training_features)
training_labels = np.array(training_labels)
print("Training data ready")

def squared_error_masked(y_true, y_pred):
    """ Squared error of elements where y_true is not 0 """
    err = y_pred - (K.cast(y_true, y_pred.dtype) * 1)
    return K.sum(K.square(err) * K.cast(K.not_equal(y_true, 0),
                                        y_pred.dtype), axis=-1)


def create_model():
    model = tf.keras.Sequential([
        tf.keras.layers.InputLayer(input_shape=417),
        tf.keras.layers.Dense(512, activation='elu'),
        tf.keras.layers.Dense(256, activation='elu'),
        tf.keras.layers.Dense(128, activation='elu'),
        tf.keras.layers.Dense(52, activation='tanh')
    ])

    model.compile(optimizer='adam',
                  loss=squared_error_masked,
                  metrics=['accuracy'])
    print("Model compiled")
    model.summary()
    return model


model = create_model()

print(training_features.size, len(training_features), training_features[0].size)
print(training_labels.size, len(training_labels), training_labels[0].size)

checkpoint_path = "model/cp.ckpt"
checkpoint_dir = os.path.dirname(checkpoint_path)

# Create a callback that saves the model's weights
cp_callback = tf.keras.callbacks.ModelCheckpoint(filepath=checkpoint_path,
                                                 save_weights_only=True,
                                                 verbose=1)

# Train the model with the new callback
model.fit(training_features,
          training_labels,
          epochs=10,
          callbacks=[cp_callback])  # Pass callback to training

model.save("model/" + savefile)
print("Model saved")

