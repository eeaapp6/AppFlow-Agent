TEMPLATE = subdirs

SUBDIRS += \
    $$PWD/FITK_Kernel \ 
#    $$PWD/FITK_Interface \
#    $$PWD/FITK_Test \
#    $$PWD/FITK_GeneralComponent \
    $$PWD/FlowApp \
    $$PWD/GUIFrame \
    $$PWD/GUIWidget \
    $$PWD/GUIDialog \
    $$PWD/GraphDataProvider \
    $$PWD/OperatorsInterface \
    $$PWD/OperatorsModel \
    $$PWD/OperatorsGUI \
  
 

CONFIG += ordered
