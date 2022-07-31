QT += network \
    gui

CONFIG += c++17 console
CONFIG -= app_bundle

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += main.cpp \
    src/application.cpp \
    src/game_core/board.cpp \
    src/game_core/game.cpp \
    src/game_core/card.cpp \
    src/game_core/dealer.cpp \
    src/game_core/deck.cpp \
    src/players/machineplayer.cpp \
    src/players/player.cpp \
    src/players/playerfactory.cpp \
    src/players/randomplayer.cpp \
    src/players/socketplayer.cpp \
    src/networking/server.cpp \
    src/networking/connection.cpp \
    src/ui/ui.cpp \
    src/util/benchmarker.cpp \
    src/util/datawriter.cpp \
    src/util/util.cpp


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += src/application.h \
    src/game_core/board.h \
    src/game_core/game.h \
    src/game_core/card.h \
    src/game_core/dealer.h \
    src/game_core/deck.h \
    src/players/machineplayer.h \
    src/players/player.h \
    src/players/playerfactory.h \
    src/players/randomplayer.h \
    src/players/socketplayer.h \
    src/networking/server.h \
    src/networking/connection.h \
    src/ui/ui.h \
    src/util/benchmarker.h \
    src/util/datawriter.h \
    src/util/util.h

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
