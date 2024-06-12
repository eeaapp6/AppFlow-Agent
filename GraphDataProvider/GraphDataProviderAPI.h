/*****************************************************************//**
 * @file    GraphDataProviderAPI.h
 * @brief   接口导出宏。
 *  
 * @author  ChengHaotian (yeguangbaozi@foxmail.com)
 * @date    2024-03-27
 *********************************************************************/

#ifndef __GRAPHDATAPROVIDERAPI_H__
#define __GRAPHDATAPROVIDERAPI_H__

#include <QtCore/QtGlobal>

#include <QDebug>

#if defined( GRAPHDATAPROVIDER_API )
#    define GRAPHDATAPROVIDERAPI Q_DECL_EXPORT
#else
#    define GRAPHDATAPROVIDERAPI Q_DECL_IMPORT
#endif

#endif // __GRAPHDATAPROVIDERAPI_H__
