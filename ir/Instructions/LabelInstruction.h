///
/// @file LabelInstruction.h
/// @brief Label指令
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
#pragma once

#include <string>

#include "Instruction.h"

class Function;

///
/// @brief Label指令
///
class LabelInstruction : public Instruction {
private:
    /// @brief 标签名称
    std::string labelName;

public:
    ///
    /// @brief 构造函数
    /// @param _func 所属函数
    ///
    explicit LabelInstruction(Function * _func);

    ///
    /// @brief 带标签名的构造函数
    /// @param _func 所属函数
    /// @param _labelName 标签名
    ///
    LabelInstruction(Function * _func, const std::string & _labelName);

    ///
    /// @brief 获取标签名
    /// @return 标签名
    ///
    std::string getLabelName() const;

    ///
    /// @brief 设置标签名
    /// @param _labelName 标签名
    ///
    void setLabelName(const std::string & _labelName);

    ///
    /// @brief 转换成字符串
    /// @param str 返回指令字符串
    ///
    void toString(std::string & str) override;

    ///
    /// @brief 返回不带冒号的标签名，用于在IfInstruction中引用
    /// @return 不带冒号的标签名
    ///
    [[nodiscard]] std::string getValueID() const override
    {
        std::string name;

        // 首先尝试使用labelName，如果为空则使用IRName
        if (!labelName.empty()) {
            name = labelName;
        } else if (!IRName.empty()) {
            name = IRName;
        } else {
            // 如果两者都为空，使用默认标签名
            name = ".L0"; // 应该不会发生，因为标签总是有名称的
        }

        // 移除末尾冒号（如果有）
        if (!name.empty() && name.back() == ':') {
            name = name.substr(0, name.length() - 1);
        }

        return name;
    }
};
