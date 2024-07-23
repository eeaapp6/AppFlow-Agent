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

#include "FITK_Kernel/FITKCore/FITKOperatorRepo.h"

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
         * @param   dataObjId：数据对象ID
         * @param   fitView：是否重置视角[缺省]
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        void updateGraph(int dataObjId, bool fitView = false) override;

        /**
         * @brief   根据数据对象ID获取模型可视化对象。（没有则不创建）[重写]
         * @param   dataObjId：数据对象ID
         * @return  可视化对象
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-23
         */
        Exchange::FITKOCC2VTKGraphObjectShape* getModelGraphObjectByDataId(int dataObjId) override;

    };

    Register2FITKOPeratorRepo(GraphPreprocess, OperGraphPreprocess);

}  // namespace GUIOper

#endif
