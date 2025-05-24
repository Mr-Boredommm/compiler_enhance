///
/// @file BcInstruction.cpp
/// @brief 条件分支指令
///
/// @author zenglj (zenglj@live.com)
/// @version 1.0
/// @date 2024-05-19
///
/// @copyright Copyright (c) 2024
///
/// @par 修改日志:
/// <table>
/// <tr><th>Date       <th>Version <th>Author  <th>Description
/// <tr><td>2024-05-19 <td>1.0     <td>zenglj  <td>新建
/// </table>
///
#include "BcInstruction.h"

/// @brief 转换成字符串
void BcInstruction::toString(std::string & str)
{
    Value * condition = getCondition();

    // 获取标签名称
    auto * trueLabelInst = dynamic_cast<LabelInstruction *>(trueLabel);
    auto * falseLabelInst = dynamic_cast<LabelInstruction *>(falseLabel);

    str = "bc " + condition->getIRName();

    if (trueLabelInst) {
        str += ", label " + trueLabelInst->getValueID();
    } else {
        str += ", label Unknown";
    }

    if (falseLabelInst) {
        str += ", label " + falseLabelInst->getValueID();
    } else {
        str += ", label Unknown";
    }
}
