/**
 * @file GUIDialogAPI.h
 * @brief 接口宏声明
 * @author YanZhiHui (chanyuantiandao@126.com)
 * @date 2024-03-05
 */
#ifndef __GUIDIALOGAPI_H__
#define __GUIDIALOGAPI_H__

#include <QtCore/QtGlobal>

#ifdef GUIDIALOG_API
#    define GUIDIALOGAPI Q_DECL_EXPORT
#else
#    define GUIDIALOGAPI Q_DECL_IMPORT
#endif

#endif // __GUIDIALOGAPI_H__
