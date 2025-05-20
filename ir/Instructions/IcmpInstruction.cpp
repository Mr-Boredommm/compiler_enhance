///
/// @file IcmpInstruction.cpp
/// @brief 条件比较指令
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
#include "IcmpInstruction.h"

/// @brief 转换成字符串
void IcmpInstruction::toString(std::string & str)
{
    Value * left = getLeft();
    Value * right = getRight();

    str = getValueID() + " = icmp " + cmpType + " " + left->getIRName() + ", " + right->getIRName();
}
