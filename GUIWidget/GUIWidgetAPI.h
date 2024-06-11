/**
 * @file GUIWidgetAPI.h
 * @brief 接口宏声明
 * @author YanZhiHui (chanyuantiandao@126.com)
 * @date 2024-03-05
 */
#ifndef __GUIWIDGETAPI_H__
#define __GUIWIDGETAPI_H__

#include <QtCore/QtGlobal>

#ifdef GUIWidget_API
#define GUIWIDGETAPI Q_DECL_EXPORT
#else
#define GUIWIDGETAPI Q_DECL_IMPORT
#endif

#endif // __GUIWIDGETAPI_H__
