/**
 * @file   GUIFrameAPI.h
 * @brief  接口宏声明
 * @author YanZhiHui (chanyuantiandao@126.com)
 * @date   2024-03-19
 */
#ifndef __GUIFRAMEAPI_H__
#define __GUIFRAMEAPI_H__

#include <QtCore/QtGlobal>

#ifdef GUIFrame_API
#define GUIFRAMEAPI Q_DECL_EXPORT
#else
#define GUIFRAMEAPI Q_DECL_IMPORT
#endif

#endif // __GUIFRAMEAPI_H__
