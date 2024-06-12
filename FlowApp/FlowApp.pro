TEMPLATE    =  app
CONFIG     +=  c++11
CONFIG     +=  qt
TARGET      =  FlowApp
QT         +=  core widgets gui 

unix:!mac{ QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN/\'" }

include(./FlowApp.pri)

win32{

    INCLUDEPATH    +=  ./   \
                       ../  \
                       ../../ \

    Release:DESTDIR         = ../output/bin
    Release:MOC_DIR         = ../generate/FlowApp/release/moc
    Release:RCC_DIR         = ../generate/FlowApp/release/rcc
    Release:UI_DIR          = ../generate/FlowApp/release/qui
    Release:OBJECTS_DIR     = ../generate/FlowApp/release/obj
    Release:LIBS +=  \
        -L../output/bin  \
        -lFITKAppFramework \
        -lFITKCore \
        -lFITKCompMessageWidget \
        -lFITKRenderWindowVTK \
        -lFITKInterfaceFlowOF \
        -lFITKInterfaceModel \
        -lFITKInterfaceOCC \
        -lOperatorsModel \
        -lOperatorsGUI \
        -lGUIFrame \
    
    Debug:CONFIG            +=  console
    Debug:DESTDIR         = ../output/bin_d
    Debug:MOC_DIR         = ../generate/FlowApp/debug/moc
    Debug:RCC_DIR         = ../generate/FlowApp/debug/rcc
    Debug:UI_DIR          = ../generate/FlowApp/debug/qui
    Debug:OBJECTS_DIR     = ../generate/FlowApp/debug/obj
    Debug:LIBS +=  \
        -L../output/bin_d  \
        -lFITKAppFramework \
        -lFITKCore \
        -lFITKCompMessageWidget \
        -lFITKRenderWindowVTK \
        -lFITKInterfaceFlowOF \
        -lFITKInterfaceModel \
        -lFITKInterfaceOCC \
        -lOperatorsModel \
        -lOperatorsGUI \
        -lGUIFrame \


    message("Windows FlowApp generated")
}

unix{

    INCLUDEPATH    +=   ./  \
                        ../ \
                        ../../ \

    CONFIG          += console
    CONFIG          += plugin
    DESTDIR         = ../output/bin
    MOC_DIR         = ../generate/FlowApp/release/moc
    UI_DIR          = ../generate/FlowApp/release/qui
    RCC_DIR         = ../generate/FlowApp/release/rcc
    OBJECTS_DIR     = ../generate/FlowApp/release/obj
    LIBS += \
        -L../output/bin \
        -lFITKAppFramework \
        -lFITKCore \
        -lFITKCompMessageWidget \
        -lFITKRenderWindowVTK \
        -lFITKInterfaceFlowOF \
        -lFITKInterfaceModel \
        -lFITKInterfaceOCC \
        -lOperatorsModel \
        -lOperatorsGUI \
        -lGUIFrame \
       

    message("Linux FlowApp generated")
}

