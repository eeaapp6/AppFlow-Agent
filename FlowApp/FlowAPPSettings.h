/**
 *
 * @file FlowAPPSettings.h
 * @brief 软件设置
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-04-06
 *
 */
#ifndef __STRUCTURAL_SETTINGS_H__
#define __STRUCTURAL_SETTINGS_H__

#include "FITK_Kernel/FITKAppFramework/FITKAppSettings.h"

/**
 * @brief 软件设置
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-04-06
 */
class FlowAPPSettings : public AppFrame::FITKAppSettings
{
public:
    /**
     * @brief Construct a new Structural Settings object
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-04-06
     */
    explicit FlowAPPSettings() = default;
    /**
     * @brief Destroy the Structural Settings object
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-04-06
     */
    virtual ~FlowAPPSettings() = default;
    /**
     * @brief 读取ini文件
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-04-06
     */
    void read() override;


};


#endif
