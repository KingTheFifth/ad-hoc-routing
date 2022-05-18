QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    host/host.cpp \
    host/GPSRHost.cpp \
    host/DSDVHost.cpp \
    host/DSRHost.cpp \
    host/routingTable.cpp \
    link.cpp \
    packet/packet.cpp \
    packet/GPSRPacket.cpp \
    packet/DSRPacket.cpp \
    packet/DSDVPacket.cpp \
    point.cpp \
    events/EventHandler.cpp \

HEADERS += \
    host/host.h \
    host/GPSRHost.h \
    host/DSDVHost.h \
    host/DSRHost.h \
    host/DSRRoute.h \
    host/routingTable.h \
    link.h \
    packet/packet.h \
    packet/GPSRPacket.h \
    packet/DSDVPacket.h \
    packet/DSRPacket.h \
    point.h \
    StatisticsHandler.h \
    events/Event.h \
    events/EventHandler.h

FORMS +=

unix {
    QMAKE_POST_LINK += $$quote(cp $$PWD/events/events.txt $$OUT_PWD$$escape_expand(\n\t))
    QMAKE_POST_LINK += $$quote(cp $$PWD/events/pres_events.txt $$OUT_PWD$$escape_expand(\n\t))
    QMAKE_POST_LINK += $$quote(cp $$PWD/events/events_small_sparse.txt $$OUT_PWD$$escape_expand(\n\t))

    QMAKE_POST_LINK += $$quote(cp $$PWD/events/ev_large_dense.txt $$OUT_PWD$$escape_expand(\n\t))
    QMAKE_POST_LINK += $$quote(cp $$PWD/events/ev_large_sparse.txt $$OUT_PWD$$escape_expand(\n\t))
    QMAKE_POST_LINK += $$quote(cp $$PWD/events/ev_small_dense.txt $$OUT_PWD$$escape_expand(\n\t))
    QMAKE_POST_LINK += $$quote(cp $$PWD/events/ev_small_sparse.txt $$OUT_PWD$$escape_expand(\n\t))
    
    QMAKE_POST_LINK += $$quote(cp $$PWD/topologies/small_sparse.txt $$OUT_PWD$$escape_expand(\n\t))
    QMAKE_POST_LINK += $$quote(cp $$PWD/topologies/small_dense.txt $$OUT_PWD$$escape_expand(\n\t))
    QMAKE_POST_LINK += $$quote(cp $$PWD/topologies/large_sparse.txt $$OUT_PWD$$escape_expand(\n\t))
    QMAKE_POST_LINK += $$quote(cp $$PWD/topologies/large_dense.txt $$OUT_PWD$$escape_expand(\n\t))

    QMAKE_POST_LINK += $$quote(cp $$PWD/events/ev_move_test.txt $$OUT_PWD$$escape_expand(\n\t))

    QMAKE_POST_LINK += $$quote(cp $$PWD/events/ev_small_dense_move.txt $$OUT_PWD$$escape_expand(\n\t))
}
win32 {
    QMAKE_POST_LINK += $$quote(cmd /c copy /y $$PWD\events\events.txt $$OUT_PWD$$escape_expand(\n\t))
    
    QMAKE_POST_LINK += $$quote(cmd /c copy /y $$PWD\topologies\small_sparse.txt $$OUT_PWD$$escape_expand(\n\t))
    QMAKE_POST_LINK += $$quote(cmd /c copy /y $$PWD\topologies\small_dense.txt $$OUT_PWD$$escape_expand(\n\t))
    QMAKE_POST_LINK += $$quote(cmd /c copy /y $$PWD\topologies\large_sparse.txt $$OUT_PWD$$escape_expand(\n\t))
    QMAKE_POST_LINK += $$quote(cmd /c copy /y $$PWD\topologies\large_dense.txt $$OUT_PWD$$escape_expand(\n\t))
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
