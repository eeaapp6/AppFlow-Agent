TEMPLATE = subdirs

SUBDIRS += \
    $$PWD/FITKGeoCompOCC \
    $$PWD/FITKRenderWindowVTK \
    $$PWD/FITKMeshGenOF \
    $$PWD/FITKWidget \
    $$PWD/FITKCompMessageWidget \
    #$$PWD/FITKRenderWindowOCC \
    $$PWD/FITKOCC2VTKGraphAdaptor \
    $$PWD/FITKOFDictWriter \

CONFIG += ordered
