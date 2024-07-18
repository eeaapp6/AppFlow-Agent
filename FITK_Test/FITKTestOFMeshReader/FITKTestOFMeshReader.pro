TEMPLATE    =  app
CONFIG     +=  c++11
CONFIG     +=  qt
TARGET      =  FITKTestOFMeshReader
QT         +=  core widgets gui testlib
DEFINES    +=  FITKTestOFMeshReader_API

unix:!mac{ QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN/\'" }

include(./FITKTestOFMeshReader.pri)

win32{

    INCLUDEPATH    +=  ./   \
                       ../  \
                       ../../ \

    Release:DESTDIR         = ../../output/bin
    Release:MOC_DIR         = ../../generate/FITKTestOFMeshReader/release/moc
    Release:RCC_DIR         = ../../generate/FITKTestOFMeshReader/release/rcc
    Release:UI_DIR          = ../../generate/FITKTestOFMeshReader/release/qui
    Release:OBJECTS_DIR     = ../../generate/FITKTestOFMeshReader/release/obj
    Release:LIBS +=  \
        -L../../output/bin  \
        -lFITKCore \
        -lFITKInterfaceIO \
        -lFITKInterfaceModel \
        -lFITKOFMesh \


    Debug:CONFIG            +=  console
    Debug:DESTDIR         = ../../output/bin_d
    Debug:MOC_DIR         = ../../generate/FITKTestOFMeshReader/debug/moc
    Debug:RCC_DIR         = ../../generate/FITKTestOFMeshReader/debug/rcc
    Debug:UI_DIR          = ../../generate/FITKTestOFMeshReader/debug/qui
    Debug:OBJECTS_DIR     = ../../generate/FITKTestOFMeshReader/debug/obj
    Debug:LIBS +=  \
        -L../../output/bin_d \
        -lFITKCore \
        -lFITKInterfaceIO \
        -lFITKInterfaceModel \
        -lFITKOFMesh \
     

    message("Windows FITKTestOFMeshReader generated")
}

unix{

    INCLUDEPATH    +=   ./  \
                        ../ \
                        ../../ \

    CONFIG          += console
    CONFIG          += plugin
    DESTDIR         = ../../output/bin
    MOC_DIR         = ../../generate/FITKTestOFMeshReader/release/moc
    UI_DIR          = ../../generate/FITKTestOFMeshReader/release/qui
    RCC_DIR         = ../../generate/FITKTestOFMeshReader/release/rcc
    OBJECTS_DIR     = ../../generate/FITKTestOFMeshReader/release/obj
    LIBS += \
        -L../../output/bin \
        -lFITKCore \
        -lFITKInterfaceIO \
        -lFITKInterfaceModel \
        -lFITKOFMesh \
       

    message("Linux FITKTestOFMeshReader generated")
}

