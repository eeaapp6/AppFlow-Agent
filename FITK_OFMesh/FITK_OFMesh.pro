TEMPLATE    =  lib
CONFIG     +=  c++11
CONFIG     +=  qt
TARGET      =  FITKOFMesh
QT         +=  core
DEFINES    +=  FITKOFMesh_API

unix:!mac{ QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN/\'" }

include(./FITK_OFMesh.pri)

win32{

    INCLUDEPATH    +=  ./   \
                       ../  \

    Release:DESTDIR         = ../output/bin
    Release:MOC_DIR         = ../generate/FITKOFMesh/release/moc
    Release:RCC_DIR         = ../generate/FITKOFMesh/release/rcc
    Release:UI_DIR          = ../generate/FITKOFMesh/release/qui
    Release:OBJECTS_DIR     = ../generate/FITKOFMesh/release/obj
    Release:LIBS +=  \
        -L../output/bin  \
        -lFITKCore \
        -lFITKInterfaceIO \
        -lFITKInterfaceModel


    Debug:CONFIG            +=  console
    Debug:DESTDIR         = ../output/bin_d
    Debug:MOC_DIR         = ../generate/FITKOFMesh/debug/moc
    Debug:RCC_DIR         = ../generate/FITKOFMesh/debug/rcc
    Debug:UI_DIR          = ../generate/FITKOFMesh/debug/qui
    Debug:OBJECTS_DIR     = ../generate/FITKOFMesh/debug/obj
    Debug:LIBS +=  \
        -L../output/bin_d \
        -lFITKCore \
        -lFITKInterfaceIO \
        -lFITKInterfaceModel




    message("Windows FITKOFMesh generated")
}

unix{

    INCLUDEPATH    +=   ./  \
                        ../ \

    CONFIG          += console
    CONFIG          += plugin
    DESTDIR         = ../output/bin
    MOC_DIR         = ../generate/FITKOFMesh/release/moc
    UI_DIR          = ../generate/FITKOFMesh/release/qui
    RCC_DIR         = ../generate/FITKOFMesh/release/rcc
    OBJECTS_DIR     = ../generate/FITKOFMesh/release/obj
    LIBS += \
        -L../output/bin \
        -lFITKCore \
        -lFITKInterfaceIO \
        -lFITKInterfaceModel


    message("Linux FITKOFMesh generated")
}

