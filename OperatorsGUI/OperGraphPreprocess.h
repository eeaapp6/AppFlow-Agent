/*****************************************************************//**
 * @file    OperGraphPreprocess.h
 * @brief   前处理可视化事件操作器。
 *  
 * @author  ChengHaotian (yeguangbaozi@foxmail.com)
 * @date    2024-06-12
 *********************************************************************/

#ifndef _OPERGRAPHPREPROCESS_H__
#define _OPERGRAPHPREPROCESS_H__

#include "OperGraphEvent3D.h"

#include "FITK_Kernal/FITKCore/FITKOperatorRepo.h"

namespace Comp
{
    class FITKGraph3DWindowVTK;
    class FITKGraphObjectVTK;
}

namespace GUIOper
{
    /**
     * @brief   前处理可视化事件处理。
     * @author  ChengHaotian (yeguangbaozi@foxmail.com)
     * @date    2024-06-12
     */
    class OperGraphPreprocess : public OperGraphEvent3D
    {
        Q_OBJECT

    public:
        /**
         * @brief   构造函数。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        explicit OperGraphPreprocess() = default;

        /**
         * @brief   析构函数。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        ~OperGraphPreprocess() = default;

    };

    Register2FITKOPeratorRepo(GraphPreprocess, OperGraphPreprocess);

}  // namespace GUIOper

#endif
