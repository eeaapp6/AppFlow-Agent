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

        /**
         * @brief   根据提供的数据对象ID更新对应可视化对象。（没有则创建）[重写]
         * @param   dataId：数据对象ID
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        void updateGraph(int dataId) override;

    };

    Register2FITKOPeratorRepo(GraphPreprocess, OperGraphPreprocess);

}  // namespace GUIOper

#endif
