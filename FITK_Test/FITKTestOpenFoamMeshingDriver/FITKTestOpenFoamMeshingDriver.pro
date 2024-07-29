TEMPLATE    =  app
CONFIG     +=  c++11
CONFIG     +=  qt
TARGET      =  FITKTestOpenFoamMeshingDriver
QT         +=  core widgets gui testlib
DEFINES    +=  FITKTestOpenFoamMeshingDriver_API

unix:!mac{ QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN/\'" }

include(./FITKTestOpenFoamMeshingDriver.pri)

win32{

    INCLUDEPATH    +=  ./   \
                       ../  \
                       ../../ \

    Release:DESTDIR         = ../../output/bin
    Release:MOC_DIR         = ../../generate/FITKTestOpenFoamMeshingDriver/release/moc
    Release:RCC_DIR         = ../../generate/FITKTestOpenFoamMeshingDriver/release/rcc
    Release:UI_DIR          = ../../generate/FITKTestOpenFoamMeshingDriver/release/qui
    Release:OBJECTS_DIR     = ../../generate/FITKTestOpenFoamMeshingDriver/release/obj
    Release:LIBS +=  \
        -L../../output/bin  \
        -lFITKCore \
        -lFITKAppFramework \
        -lFITKOFDriver \


    Debug:CONFIG            +=  console
    Debug:DESTDIR         = ../../output/bin_d
    Debug:MOC_DIR         = ../../generate/FITKTestOpenFoamMeshingDriver/debug/moc
    Debug:RCC_DIR         = ../../generate/FITKTestOpenFoamMeshingDriver/debug/rcc
    Debug:UI_DIR          = ../../generate/FITKTestOpenFoamMeshingDriver/debug/qui
    Debug:OBJECTS_DIR     = ../../generate/FITKTestOpenFoamMeshingDriver/debug/obj
    Debug:LIBS +=  \
        -L../../output/bin_d \
        -lFITKCore \
        -lFITKAppFramework \
        -lFITKOFDriver \
     

    message("Windows FITKTestOpenFoamMeshingDriver generated")
}

unix{

    INCLUDEPATH    +=   ./  \
                        ../ \
                        ../../ \

    CONFIG          += console
    CONFIG          += plugin
    DESTDIR         = ../../output/bin
    MOC_DIR         = ../../generate/FITKTestOpenFoamMeshingDriver/release/moc
    UI_DIR          = ../../generate/FITKTestOpenFoamMeshingDriver/release/qui
    RCC_DIR         = ../../generate/FITKTestOpenFoamMeshingDriver/release/rcc
    OBJECTS_DIR     = ../../generate/FITKTestOpenFoamMeshingDriver/release/obj
    LIBS += -L../../output/bin \
        -lFITKCore \
        -lFITKAppFramework \
        -lFITKOFDriver \
       

    message("Linux FITKTestOpenFoamMeshingDriver generated")
}

