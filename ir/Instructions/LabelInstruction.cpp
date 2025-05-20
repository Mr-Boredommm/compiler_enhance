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
    : Instruction(_func, IRInstOperator::IRINST_OP_LABEL, VoidType::getType())
{
    // 确保标签名格式一致（以点开头）
    // std::string formattedName = _labelName;
    // if (!formattedName.empty() && formattedName[0] != '.') {
    //     formattedName = "." + formattedName;
    // }
    // this->IRName = formattedName;    // 设置IRName为格式化后的标签名
    // this->labelName = formattedName; // 更新labelName
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
    std::string name;

    // 首先尝试使用labelName，如果为空则使用IRName
    if (!labelName.empty()) {
        name = labelName;
    } else if (!IRName.empty()) {
        name = IRName;
    } else {
        // 如果两者都为空，生成一个唯一的标签名
        static int uniqueLabelCount = 0;
        name = ".L" + std::to_string(uniqueLabelCount++);
    }

    // 确保标签以点开头
    if (!name.empty() && name[0] != '.') {
        name = "." + name;
    }

    // 在生成的IR中，标签行应该包含标签名和冒号
    str = name + ":";
}
