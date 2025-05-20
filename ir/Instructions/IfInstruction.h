///
/// @file IfInstruction.h
/// @brief 条件跳转指令
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

///
/// @brief 条件跳转指令
///
class IfInstruction : public Instruction {

private:
    /// @brief 目标标签
    Instruction * targetLabel;

    /// @brief 是否是false条件跳转
    bool isNotCondition;

public:
    /// @brief 构造函数
    /// @param _func 所在函数
    /// @param _op 操作符（IRINST_OP_IF或IRINST_OP_IFNOT）
    /// @param _condition 条件值
    /// @param _targetLabel 目标标签
    IfInstruction(Function * _func, IRInstOperator _op, Value * _condition, Instruction * _targetLabel)
        : Instruction(_func, _op, _condition->getType()), targetLabel(_targetLabel),
          isNotCondition(_op == IRInstOperator::IRINST_OP_IFNOT)
    {
        addOperand(_condition);
    }

    /// @brief 获取条件值
    /// @return 条件值
    Value * getCondition()
    {
        return getOperand(0);
    }

    /// @brief 获取目标标签
    /// @return 目标标签
    Instruction * getTargetLabel()
    {
        return targetLabel;
    }

    /// @brief 是否是false条件跳转
    /// @return true为false条件跳转，false为true条件跳转
    bool isIfNot()
    {
        return isNotCondition;
    }

    /// @brief 转换成字符串
    void toString(std::string & str) override
    {
        // 不再生成自己的IR格式，而是直接使用"if"或"ifnot"
        if (isNotCondition) {
            str += "ifnot ";
        } else {
            str += "if ";
        }

        // 获取操作数信息
        Value * op = getOperand(0);
        str += op->getValueID();

        // 获取目标标签名称
        auto * labelInst = dynamic_cast<LabelInstruction *>(targetLabel);
        if (labelInst) {
            // 获取标签ID，不应该包含冒号
            std::string labelID = labelInst->getValueID();
            str += " goto " + labelID;
        } else {
            // 非LabelInstruction类型，这是一个严重的错误
            str += " goto Unknown";
        }
    }
};
