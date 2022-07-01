QT -= gui

CONFIG += c++17 console
CONFIG -= app_bundle

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += main.cpp \
    src/games/board.cpp \
    src/games/game.cpp \
    src/games/humanplayer.cpp \
    src/games/machineplayer.cpp \
    src/games/player.cpp \
    #src/games/socketplayer.cpp \
    src/logic_base/card.cpp \
    src/logic_base/dealer.cpp \
    src/logic_base/deck.cpp \
    src/ui/ui.cpp


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += headers/games/board.h \
    headers/games/game.h \
    headers/games/humanplayer.h \
    headers/games/machineplayer.h \
    headers/games/player.h \
    #src/games/socketplayer.h \
    headers/logic_base/card.h \
    headers/logic_base/dealer.h \
    headers/logic_base/deck.h \
    headers/ui/ui.h

