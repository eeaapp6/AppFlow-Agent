/**
 *
 * @file OperatorsModelAPI.h
 * @brief  声明接口
 * @author LiBaojun (libaojunqd@foxmail.com)
 * @date 2024-04-14
 *
 */
#ifndef _OPERATORMODELS___API_H___
#define _OPERATORMODELS___API_H___

#include <QtCore/QtGlobal>

#ifdef OperatorsModel_API
#define OperatorsModelAPI Q_DECL_EXPORT
#else
#define OperatorsModelAPI Q_DECL_IMPORT
#endif
#endif
