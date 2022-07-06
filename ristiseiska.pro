QT -= gui
QT += network

CONFIG += c++17 console
CONFIG -= app_bundle

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += main.cpp \
    src/application.cpp \
    src/games/board.cpp \
    src/games/game.cpp \
    src/games/humanplayer.cpp \
    src/games/machineplayer.cpp \
    src/games/player.cpp \
    src/games/playerfactory.cpp \
    src/games/socketplayer.cpp \
    src/logic_base/card.cpp \
    src/logic_base/dealer.cpp \
    src/logic_base/deck.cpp \
    src/networking/server.cpp \
    src/ui/ui.cpp \
    src/networking/connection.cpp


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += src/games/board.h \
    src/application.h \
    src/games/game.h \
    src/games/humanplayer.h \
    src/games/machineplayer.h \
    src/games/player.h \
    src/games/playerfactory.h \
    src/games/socketplayer.h \
    src/logic_base/card.h \
    src/logic_base/dealer.h \
    src/logic_base/deck.h \
    src/networking/server.h \
    src/ui/ui.h \
    src/networking/connection.h

RESOURCES += \
    resource.qrc

CONFIG(release, debug|release) {
    DESTDIR = release
} else {
    DESTDIR = debug
}

#for Windows
win32 {
    #in Windows, you can use & to separate commands
    copyfiles.commands += @echo NOW COPYING ADDITIONAL FILE(S) &
    copyfiles.commands += @call copy ..\\$${TARGET}\\python\\ $${DESTDIR}\\
}

QMAKE_EXTRA_TARGETS += copyfiles
POST_TARGETDEPS += copyfiles
