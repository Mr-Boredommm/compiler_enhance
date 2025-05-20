///
/// @file IcmpInstruction.h
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
#pragma once

#include "Instruction.h"
#include "../Types/BoolType.h"

///
/// @brief 条件比较指令
///
class IcmpInstruction : public Instruction {

private:
    /// @brief 比较类型 (gt, lt, ge, le, eq, ne)
    std::string cmpType;

public:
    /// @brief 构造函数
    /// @param _func 所在函数
    /// @param _op 操作符
    /// @param _left 左操作数
    /// @param _right 右操作数
    /// @param _cmpType 比较类型 (gt, lt, ge, le, eq, ne)
    IcmpInstruction(Function * _func, IRInstOperator _op, Value * _left, Value * _right, const std::string & _cmpType)
        : Instruction(_func, _op, BoolType::getType()), cmpType(_cmpType)
    {
        addOperand(_left);
        addOperand(_right);
    }

    /// @brief 获取左操作数
    /// @return 左操作数
    Value * getLeft()
    {
        return getOperand(0);
    }

    /// @brief 获取右操作数
    /// @return 右操作数
    Value * getRight()
    {
        return getOperand(1);
    }

    /// @brief 获取比较类型
    /// @return 比较类型
    std::string getCmpType()
    {
        return cmpType;
    }

    /// @brief 转换成字符串
    void toString(std::string & str) override;
};
