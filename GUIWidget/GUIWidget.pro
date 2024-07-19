TEMPLATE    =   lib
CONFIG      +=  c++11
CONFIG      +=  qt
TARGET      =   GUIWidget
QT          +=  core widgets gui
DEFINES     += GUIWidget_API

unix:!mac{ QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN/\'" }

include(./occ.pri)
include(./vtk.pri)
include(./GUIWidget.pri)

win32{

    INCLUDEPATH    +=  ./   \
                       ../  \
                       
    Release:DESTDIR         = ../output/bin
    Release:MOC_DIR         = ../generate/GUIWidget/release/moc
    Release:RCC_DIR         = ../generate/GUIWidget/release/rcc
    Release:UI_DIR          = ../generate/GUIWidget/release/qui
    Release:OBJECTS_DIR     = ../generate/GUIWidget/release/obj
    Release:LIBS +=  \
        -L../output/bin  \
        -lFITKAppFramework \
        -lFITKPython \
        -lFITKCore \
        -lOperatorsInterface \
        -lFITKOCC2VTKGraphAdaptor \
        -lFITKVTKAlgorithm \
        -lFITKInterfaceGeometry \
        -lFITKInterfaceFlowOF \
        -lFITKInterfaceModel \
        
    Debug:CONFIG            +=  console
    Debug:DESTDIR         = ../output/bin_d
    Debug:MOC_DIR         = ../generate/GUIWidget/debug/moc
    Debug:RCC_DIR         = ../generate/GUIWidget/debug/rcc
    Debug:UI_DIR          = ../generate/GUIWidget/debug/qui
    Debug:OBJECTS_DIR     = ../generate/GUIWidget/debug/obj
    Debug:LIBS +=  \
        -L../output/bin_d \
        -lFITKAppFramework \
        -lFITKPython \
        -lFITKCore \
        -lOperatorsInterface \
        -lFITKOCC2VTKGraphAdaptor \
        -lFITKVTKAlgorithm \
        -lFITKInterfaceGeometry \
        -lFITKInterfaceFlowOF \
        -lFITKInterfaceModel \

    message("Windows GUIWidget generated")
}

unix{

    INCLUDEPATH    +=   ./  \
                        ../ \

    CONFIG          += console
    CONFIG          += plugin
    DESTDIR         = ../output/bin
    MOC_DIR         = ../generate/GUIWidget/release/moc
    UI_DIR          = ../generate/GUIWidget/release/qui
    RCC_DIR         = ../generate/GUIWidget/release/rcc
    OBJECTS_DIR     = ../generate/GUIWidget/release/obj
    LIBS += \
        -L../output/bin \
        -lFITKAppFramework \
        -lFITKPython \
        -lFITKCore \
        -lOperatorsInterface \
        -lFITKOCC2VTKGraphAdaptor \
        -lFITKVTKAlgorithm \
        -lFITKInterfaceGeometry \
        -lFITKInterfaceFlowOF \
        -lFITKInterfaceModel \
       

    message("Linux GUIWidget generated")
}

