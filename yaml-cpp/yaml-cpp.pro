#-------------------------------------------------
#
# Project created by QtCreator 2019-11-26T13:54:23
#
#-------------------------------------------------

QT       -= gui

TARGET = yaml-cpp
TEMPLATE = lib

DEFINES += YAML_CPP_DLL

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += $$PWD \
                $$PWD/inc \
                $$PWD/inc/yaml-cpp \
                $$PWD/inc/yaml-cpp\node \
                $$PWD/inc/yaml-cpp\node\detail \
                $$PWD/src \

SOURCES += \
    src/binary.cpp \
    src/convert.cpp \
    src/directives.cpp \
    src/emit.cpp \
    src/emitfromevents.cpp \
    src/emitter.cpp \
    src/emitterstate.cpp \
    src/emitterutils.cpp \
    src/exceptions.cpp \
    src/exp.cpp \
    src/memory.cpp \
    src/node.cpp \
    src/node_data.cpp \
    src/nodebuilder.cpp \
    src/nodeevents.cpp \
    src/null.cpp \
    src/ostream_wrapper.cpp \
    src/parse.cpp \
    src/parser.cpp \
    src/regex_yaml.cpp \
    src/scanner.cpp \
    src/scanscalar.cpp \
    src/scantag.cpp \
    src/scantoken.cpp \
    src/simplekey.cpp \
    src/singledocparser.cpp \
    src/stream.cpp \
    src/tag.cpp

HEADERS +=\
    inc/yaml-cpp/node/detail/impl.h \
    inc/yaml-cpp/node/detail/iterator.h \
    inc/yaml-cpp/node/detail/iterator_fwd.h \
    inc/yaml-cpp/node/detail/memory.h \
    inc/yaml-cpp/node/detail/node.h \
    inc/yaml-cpp/node/detail/node_data.h \
    inc/yaml-cpp/node/detail/node_iterator.h \
    inc/yaml-cpp/node/detail/node_ref.h \
    inc/yaml-cpp/node/convert.h \
    inc/yaml-cpp/node/emit.h \
    inc/yaml-cpp/node/impl.h \
    inc/yaml-cpp/node/iterator.h \
    inc/yaml-cpp/node/node.h \
    inc/yaml-cpp/node/parse.h \
    inc/yaml-cpp/node/ptr.h \
    inc/yaml-cpp/node/type.h \
    inc/yaml-cpp/anchor.h \
    inc/yaml-cpp/binary.h \
    inc/yaml-cpp/dll.h \
    inc/yaml-cpp/emitfromevents.h \
    inc/yaml-cpp/emitter.h \
    inc/yaml-cpp/emitterdef.h \
    inc/yaml-cpp/emittermanip.h \
    inc/yaml-cpp/emitterstyle.h \
    inc/yaml-cpp/eventhandler.h \
    inc/yaml-cpp/exceptions.h \
    inc/yaml-cpp/mark.h \
    inc/yaml-cpp/null.h \
    inc/yaml-cpp/ostream_wrapper.h \
    inc/yaml-cpp/parser.h \
    inc/yaml-cpp/stlemitter.h \
    inc/yaml-cpp/traits.h \
    inc/yaml-cpp/yaml.h \
    src/collectionstack.h \
    src/directives.h \
    src/emitterstate.h \
    src/emitterutils.h \
    src/exp.h \
    src/indentation.h \
    src/nodebuilder.h \
    src/nodeevents.h \
    src/ptr_vector.h \
    src/regex_yaml.h \
    src/regeximpl.h \
    src/scanner.h \
    src/scanscalar.h \
    src/scantag.h \
    src/setting.h \
    src/singledocparser.h \
    src/stream.h \
    src/streamcharsource.h \
    src/stringsource.h \
    src/tag.h \
    src/token.h \
    inc/yaml-cpp_global.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
