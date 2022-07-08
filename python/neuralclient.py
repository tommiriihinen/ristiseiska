# import required module
import os
from enum import Enum

directory = 'data'

print("Directory \\" + directory + "\\ contents:")
for filename in os.scandir(directory):
    if filename.is_file():
        print(filename.path)
filename = input("which file to train from: ")


class Turn:
    handSizes = []  # n * n
    opponentsHistory = []  # n * [52]
    ownHand = []  # [52]
    action = 1  # 1 = Play, 0 Give
    decision = []  # [52]


def binarize_card_id(card_id):
    suit = 0
    match card_id[0]:
        case 'C':
            suit = 0
        case 'D':
            suit = 1
        case 'H':
            suit = 2
        case 'S':
            suit = 3
    match card_id[1]:
        case 'A':
            rank = 1
        case 'X':
            rank = 10
        case 'J':
            rank = 11
        case 'Q':
            rank = 12
        case 'K':
            rank = 13
        case _:
            rank = int(card_id[1])
    return (suit * 13 + rank) - 1  # 0-51


class Game:
    players = 0  # 3-7
    winner = 0
    turns = []


# parts      [0]        [1]               [2]          [3]
# game_text: (n players);n*(startingHand,);m*(action,);(winner)

def parse_game(game_text, winner=True):
    game = Game()
    game_parts = game_text.split(";")
    game.players = int(game_parts[0])
    game.winner = int(game_parts[3])

    # save hands
    hand_data = game_parts[1].split(",")  # P1 C2 C7 DA ... ,P2 CA C3 D4 ...
    player_hand_dict = {}
    for player in range(0, game.players):
        starting_hand = [False] * 52
        card_ids = hand_data[player].strip().split(" ")  # P1 C2 C7 DA ...
        player_id = card_ids.pop(0)  # P[1]

        for card_id in card_ids:
            starting_hand[binarize_card_id(card_id)] = True

        player_hand_dict[player_id] = starting_hand
        starting_hands.append(starting_hand)

    for hand in starting_hands:
        print(hand)


file = open(directory + "\\" + filename)
data = ""
for line in file:
    data += line

games = data.split("/")
metadata = games[0]

print("game[1]:", games[1])

parse_game(games[1])



