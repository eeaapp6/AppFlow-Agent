/**
 *
 * @file GUIMeshDialogAPI.h
 * @brief 几何相关对话框接口声明
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-04-19
 *
 */
#ifndef _GUIMeshDialogAPI_H
#define _GUIMeshDialogAPI_H

#include <QtCore/QtGlobal>

#if defined( GUIMeshDialog_API )
#    define GUIMeshDialogAPI Q_DECL_EXPORT
#else
#    define GUIMeshDialogAPI Q_DECL_IMPORT
#endif

#endif // __GUIWIDGETAPI_H__
