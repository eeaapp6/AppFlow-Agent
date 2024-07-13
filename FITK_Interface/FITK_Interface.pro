TEMPLATE = subdirs

SUBDIRS += \
    $$PWD/FITKInterfaceMesh \ 
    $$PWD/FITKInterfaceModel \ 
    $$PWD/FITKInterfacePhysics \
    $$PWD/FITKInterfaceIO \
    $$PWD/FITKInterfaceFlowOF \
#    $$PWD/FITKInterfaceMeshGen \
    $$PWD/FITKInterfaceGeometry \
    $$PWD/FITKVTKAlgorithm \

CONFIG += ordered
