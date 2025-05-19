///
/// @file LabelInstruction.cpp
/// @brief Label指令
/// @author zenglj (zenglj@live.com)
/// @version 1.0
/// @date 2024-09-29
///
/// @copyright Copyright (c) 2024
///
/// @par 修改日志:
/// <table>
/// <tr><th>Date       <th>Version <th>Author  <th>Description
/// <tr><td>2024-09-29 <td>1.0     <td>zenglj  <td>新建
/// </table>
///
#include "VoidType.h"

#include "LabelInstruction.h"

///
/// @brief 构造函数
/// @param _func 所属函数
///
LabelInstruction::LabelInstruction(Function * _func)
    : Instruction(_func, IRInstOperator::IRINST_OP_LABEL, VoidType::getType()), labelName("")
{}

///
/// @brief 带标签名的构造函数
/// @param _func 所属函数
/// @param _labelName 标签名
///
LabelInstruction::LabelInstruction(Function * _func, const std::string & _labelName)
    : Instruction(_func, IRInstOperator::IRINST_OP_LABEL, VoidType::getType()), labelName(_labelName)
{
    this->IRName = _labelName; // 设置IRName为标签名
}

///
/// @brief 获取标签名
/// @return 标签名
///
std::string LabelInstruction::getLabelName() const
{
    return labelName;
}

///
/// @brief 设置标签名
/// @param _labelName 标签名
///
void LabelInstruction::setLabelName(const std::string & _labelName)
{
    labelName = _labelName;
    this->IRName = _labelName; // 同时更新IRName
}

/// @brief 转换成字符串
/// @param str 返回指令字符串
void LabelInstruction::toString(std::string & str)
{
    if (!labelName.empty()) {
        str = labelName + ":";
    } else {
        str = IRName + ":";
    }
}
