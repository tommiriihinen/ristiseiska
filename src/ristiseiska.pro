QT += network \
    gui

CONFIG += c++17 console
CONFIG -= app_bundle

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += main.cpp \
    application.cpp \
    game_core/board.cpp \
    game_core/game.cpp \
    game_core/card.cpp \
    game_core/dealer.cpp \
    game_core/deck.cpp \
    players/machineplayer.cpp \
    players/player.cpp \
    players/playerfactory.cpp \
    players/randomplayer.cpp \
    players/socketplayer.cpp \
    ui/ui.cpp \
    util/benchmarker.cpp \
    util/datawriter.cpp \
    util/util.cpp


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += application.h \
    game_core/board.h \
    game_core/game.h \
    game_core/card.h \
    game_core/dealer.h \
    game_core/deck.h \
    players/machineplayer.h \
    players/player.h \
    players/playerfactory.h \
    players/randomplayer.h \
    players/socketplayer.h \
    ui/ui.h \
    util/benchmarker.h \
    util/datawriter.h \
    util/util.h


CONFIG(release, debug|release) {
    DESTDIR = release
} else {
    DESTDIR = debug
}

#for Windows
win32 {
    #in Windows, you can use & to separate commands
    copyfiles.commands += @echo NOW COPYING PYTHON FILES &
    copyfiles.commands += @call copy ..\\$${TARGET}\\..\\python\\ $${DESTDIR}\\ &
    copyfiles.commands += @echo NOW COPYING MODELS &
    copyfiles.commands += @call dir ..\\$${TARGET}\\..\\python &
    copyfiles.commands += @call xcopy ..\\$${TARGET}\\..\\python\\models\\ $${DESTDIR}\\models\\ /E /Y
}

QMAKE_EXTRA_TARGETS += copyfiles
POST_TARGETDEPS += copyfiles


CONFIG(release, debug|release): DEFINES += QT_NO_DEBUG_OUTPUT
CONFIG(release, debug|release): DEFINES += QT_NO_INFO_OUTPUT
CONFIG(release, debug|release): DEFINES += QT_NO_WARNING_OUTPUT

# Use gprof profiler
QMAKE_CXXFLAGS_DEBUG += -pg
QMAKE_LFLAGS_DEBUG += -pg


