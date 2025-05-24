///
/// @file BcInstruction.h
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
#pragma once

#include "Instruction.h"
#include "LabelInstruction.h"
#include "../Types/VoidType.h"
#include "../Types/VoidType.h"

///
/// @brief 条件分支指令
///
class BcInstruction : public Instruction {

private:
    /// @brief 条件为真时跳转的标签
    Instruction * trueLabel;

    /// @brief 条件为假时跳转的标签
    Instruction * falseLabel;

public:
    /// @brief 构造函数
    /// @param _func 所在函数
    /// @param _condition 条件值
    /// @param _trueLabel 条件为真时跳转的标签
    /// @param _falseLabel 条件为假时跳转的标签
    BcInstruction(Function * _func, Value * _condition, Instruction * _trueLabel, Instruction * _falseLabel)
        : Instruction(_func, IRInstOperator::IRINST_OP_BC, VoidType::getType()), trueLabel(_trueLabel),
          falseLabel(_falseLabel)
    {
        addOperand(_condition);
    }

    /// @brief 获取条件值
    /// @return 条件值
    Value * getCondition()
    {
        return getOperand(0);
    }

    /// @brief 获取真分支标签
    /// @return 真分支标签
    Instruction * getTrueLabel()
    {
        return trueLabel;
    }

    /// @brief 获取假分支标签
    /// @return 假分支标签
    Instruction * getFalseLabel()
    {
        return falseLabel;
    }

    /// @brief 转换成字符串
    void toString(std::string & str) override;
};
