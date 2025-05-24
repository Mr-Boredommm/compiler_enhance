///
/// @file BoolType.h
/// @brief 布尔类型
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

#include "Type.h"

///
/// @brief 布尔类型
///
class BoolType : public Type {

private:
    /// @brief 构造函数
    BoolType();

    /// @brief 布尔类型的单例
    static BoolType * boolType;

public:
    /// @brief 获取布尔类型单例
    /// @return 布尔类型单例
    static BoolType * getType();

    /// @brief 获取类型字符串表示
    /// @return 类型字符串
    [[nodiscard]] std::string toString() const override;
};
