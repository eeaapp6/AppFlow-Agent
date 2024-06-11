TEMPLATE    =   lib
CONFIG      +=  c++11
CONFIG      +=  qt
TARGET      =   GUIFrame
QT          +=  core widgets gui
DEFINES     += GUIFrame_API

unix:!mac{ QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN/\'" }

include(./GUIFrame.pri)
#include(./vtk.pri)
 

win32{

    INCLUDEPATH    +=  ./   \
                       ../  \

    Release:DESTDIR         = ../output/bin
    Release:MOC_DIR         = ../generate/GUIFrame/release/moc
    Release:RCC_DIR         = ../generate/GUIFrame/release/rcc
    Release:UI_DIR          = ../generate/GUIFrame/release/qui
    Release:OBJECTS_DIR     = ../generate/GUIFrame/release/obj
    Release:LIBS +=  \
        -L../output/bin  \
        -lFITKAppFramework \
        -lFITKCore \
        -lFITKPython \
        -lFITKWidget \
        -lGUIWidget \


    Debug:CONFIG            +=  console
    Debug:DESTDIR         = ../output/bin_d
    Debug:MOC_DIR         = ../generate/GUIFrame/debug/moc
    Debug:RCC_DIR         = ../generate/GUIFrame/debug/rcc
    Debug:UI_DIR          = ../generate/GUIFrame/debug/qui
    Debug:OBJECTS_DIR     = ../generate/GUIFrame/debug/obj
    Debug:LIBS +=  \
        -L../output/bin_d \
        -lFITKAppFramework \
        -lFITKCore \
        -lFITKPython \
        -lFITKWidget \
        -lGUIWidget \

    message("Windows GUIFrame generated")
}

unix{
    CONFIG          += console
    CONFIG          += plugin
    DESTDIR         = ../output/bin
    MOC_DIR         = ../generate/GUIFrame/release/moc
    UI_DIR          = ../generate/GUIFrame/release/qui
    RCC_DIR         = ../generate/GUIFrame/release/rcc
    OBJECTS_DIR     = ../generate/GUIFrame/release/obj
    INCLUDEPATH    +=   ./  \
                        ../ \
    LIBS += \
        -L../output/bin \
        -lFITKAppFramework \
        -lFITKCore \
        -lFITKPython \
        -lFITKWidget \
        -lGUIWidget \

    message("Linux GUIFrame generated")
}

