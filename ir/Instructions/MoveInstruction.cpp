///
/// @file MoveInstruction.cpp
/// @brief Move指令，也就是DragonIR的Asssign指令
///
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

#include "MoveInstruction.h"

///
/// @brief 构造函数
/// @param _func 所属的函数
/// @param result 结构操作数
/// @param srcVal1 源操作数
///
MoveInstruction::MoveInstruction(Function * _func, Value * _result, Value * _srcVal1)
    : Instruction(_func, IRInstOperator::IRINST_OP_ASSIGN, VoidType::getType()), arrayAccessType(NOT_ARRAY_ACCESS)
{
    addOperand(_result);
    addOperand(_srcVal1);
}

///
/// @brief 构造函数（带数组访问类型）
/// @param _func 所属的函数
/// @param result 结构操作数
/// @param srcVal1 源操作数
/// @param accessType 数组访问类型
///
MoveInstruction::MoveInstruction(Function * _func, Value * _result, Value * _srcVal1, ArrayAccessType accessType)
    : Instruction(_func, IRInstOperator::IRINST_OP_ASSIGN, VoidType::getType()), arrayAccessType(accessType)
{
    addOperand(_result);
    addOperand(_srcVal1);
}

///
/// @brief 构造函数（兼容旧接口）
/// @param _func 所属的函数
/// @param result 结构操作数
/// @param srcVal1 源操作数
/// @param isArrayAccess 是否是数组元素访问
///
MoveInstruction::MoveInstruction(Function * _func, Value * _result, Value * _srcVal1, bool isArrayAccess)
    : Instruction(_func, IRInstOperator::IRINST_OP_ASSIGN, VoidType::getType()),
      arrayAccessType(isArrayAccess ? ARRAY_WRITE : NOT_ARRAY_ACCESS)
{
    addOperand(_result);
    addOperand(_srcVal1);
}

/// @brief 转换成字符串显示
/// @param str 转换后的字符串
void MoveInstruction::toString(std::string & str)
{
    Value *dstVal = getOperand(0), *srcVal = getOperand(1);

    switch (arrayAccessType) {
        case ARRAY_WRITE:
            // 数组元素写入: *addr = value
            str = "*" + dstVal->getIRName() + " = " + srcVal->getIRName();
            break;
        case ARRAY_READ:
            // 数组元素读取: value = *addr
            str = dstVal->getIRName() + " = *" + srcVal->getIRName();
            break;
        default:
            // 普通变量赋值
            str = dstVal->getIRName() + " = " + srcVal->getIRName();
            break;
    }
}