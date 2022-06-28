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
        src/logic_base/card.cpp \
        src/logic_base/dealer.cpp \
        src/logic_base/deck.cpp \


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += headers/games/*.h \
        headers/logic_base/*.h \

