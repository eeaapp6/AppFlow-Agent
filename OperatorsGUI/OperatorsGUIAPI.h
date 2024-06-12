/*****************************************************************//**
 * @file    OperatorsGUIAPI.h
 * @brief   For export macro.
 *  
 * @author  ChengHaotian (yeguangbaozi@foxmail.com)
 * @date    2024-06-12
 *********************************************************************/

#ifndef _OperatorsGUIAPI_H_
#define _OperatorsGUIAPI_H_

#include <QtCore/QtGlobal>

#include <QDebug>

#if defined(OperatorsGUI_API)
#define OperatorsGUIAPI Q_DECL_EXPORT
#else
#define OperatorsGUIAPI Q_DECL_IMPORT
#endif

#endif
