/**
 *
 * @file GUIGeometryDialogAPI.h
 * @brief 几何相关对话框接口声明
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-04-19
 *
 */
#ifndef __GUIGeometryDialogAPI_H
#define __GUIGeometryDialogAPI_H

#include <QtCore/QtGlobal>

#if defined( GUIGeometryDialog_API )
#    define GUIGeometryDialogAPI Q_DECL_EXPORT
#else
#    define GUIGeometryDialogAPI Q_DECL_IMPORT
#endif

#endif // __GUIWIDGETAPI_H__
