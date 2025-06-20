///
/// @file MoveInstruction.h
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
#pragma once

#include <string>

#include "Value.h"
#include "Instruction.h"

class Function;

// 数组访问类型枚举
enum ArrayAccessType {
    NOT_ARRAY_ACCESS, // 不是数组访问
    ARRAY_WRITE,      // 数组元素写入 (a[i] = x)
    ARRAY_READ        // 数组元素读取 (x = a[i])
};

///
/// @brief 复制指令
///
class MoveInstruction : public Instruction {

public:
    ///
    /// @brief 构造函数
    /// @param _func 所属的函数
    /// @param result 结构操作数
    /// @param srcVal1 源操作数
    ///
    MoveInstruction(Function * _func, Value * result, Value * srcVal1);

    ///
    /// @brief 构造函数（带数组标志）
    /// @param _func 所属的函数
    /// @param result 结构操作数
    /// @param srcVal1 源操作数
    /// @param accessType 数组访问类型
    ///
    MoveInstruction(Function * _func, Value * result, Value * srcVal1, ArrayAccessType accessType);

    ///
    /// @brief 构造函数（兼容旧接口）
    /// @param _func 所属的函数
    /// @param result 结构操作数
    /// @param srcVal1 源操作数
    /// @param isArrayAccess 是否是数组元素访问
    ///
    MoveInstruction(Function * _func, Value * result, Value * srcVal1, bool isArrayAccess);

    /// @brief 转换成字符串
    void toString(std::string & str) override;

private:
    /// @brief 数组访问类型
    ArrayAccessType arrayAccessType;
};
