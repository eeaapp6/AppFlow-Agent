#ifndef __OPERMODELMANAGER_H__
#define __OPERMODELMANAGER_H__

#include "FITK_Kernel/FITKCore/FITKActionOperator.h"
#include "FITK_Kernel/FITKCore/FITKOperatorRepo.h"

namespace Oper
{
	class OperModelManager : public Core::FITKActionOperator
	{
	public:
		/**
		 * @brief 构造函数
		 * @author YanZhiHui (chanyuantiandao@126.com)
		 * @date 2024-06-12
		 */
		explicit OperModelManager() = default;
		/**
		 * @brief 析构函数
		 * @author YanZhiHui (chanyuantiandao@126.com)
		 * @date 2024-06-12
		 */
		~OperModelManager() = default;

	private:
		/**
		 * @brief 界面逻辑，生成UI交互
		 * @return 执行成功返回true，否则返回false
		 * @author yanzhihui (chanyuantiandao@126.com)
		 * @date 2024-06-12
		 */
		bool execGUI() override;
		/**
		 * @brief 业务处理逻辑，在execGUI后执行
		 * @return 执行成功返回true，否则返回false
		 * @author yanzhihui (chanyuantiandao@126.com)
		 * @date 2024-06-12
		 */
		bool execProfession() override;

	};

	// 按钮注册相关操作
	Register2FITKOPeratorRepo(OperModelManager, OperModelManager);
	Register2FITKOPeratorRepo(actionCreateBox, OperModelManager);
	Register2FITKOPeratorRepo(actionCreateCylinder, OperModelManager);
	Register2FITKOPeratorRepo(actionCreateSphere, OperModelManager);
}

#endif // !__OPERMODELMANAGER_H__