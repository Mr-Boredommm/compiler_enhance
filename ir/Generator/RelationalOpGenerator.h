///
/// @file RelationalOpGenerator.h
/// @brief 关系运算符和控制流生成器头文件
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

#include "AST.h"
#include "Module.h"

// 关系运算符生成器命名空间
namespace RelationalOpGenerator {

    /// @brief 处理小于关系运算符
    /// @param node AST节点
    /// @param module 模块
    /// @return 处理结果，true：成功，false：失败
    bool rel_lt(ast_node * node, Module * module);

    /// @brief 处理小于等于关系运算符
    /// @param node AST节点
    /// @param module 模块
    /// @return 处理结果，true：成功，false：失败
    bool rel_le(ast_node * node, Module * module);

    /// @brief 处理大于关系运算符
    /// @param node AST节点
    /// @param module 模块
    /// @return 处理结果，true：成功，false：失败
    bool rel_gt(ast_node * node, Module * module);

    /// @brief 处理大于等于关系运算符
    /// @param node AST节点
    /// @param module 模块
    /// @return 处理结果，true：成功，false：失败
    bool rel_ge(ast_node * node, Module * module);

    /// @brief 处理等于关系运算符
    /// @param node AST节点
    /// @param module 模块
    /// @return 处理结果，true：成功，false：失败
    bool rel_eq(ast_node * node, Module * module);

    /// @brief 处理不等于关系运算符
    /// @param node AST节点
    /// @param module 模块
    /// @return 处理结果，true：成功，false：失败
    bool rel_ne(ast_node * node, Module * module);

    /// @brief 逻辑与 && 翻译成线性中间IR，实现短路求值
    /// @param node AST节点
    /// @param module 模块
    /// @return 翻译是否成功，true：成功，false：失败
    bool rel_logical_and(ast_node * node, Module * module);

    /// @brief 逻辑或 || 翻译成线性中间IR，实现短路求值
    /// @param node AST节点
    /// @param module 模块
    /// @return 翻译是否成功，true：成功，false：失败
    bool rel_logical_or(ast_node * node, Module * module);

    /// @brief 逻辑非 ! 翻译成线性中间IR
    /// @param node AST节点
    /// @param module 模块
    /// @return 翻译是否成功，true：成功，false：失败
    bool rel_logical_not(ast_node * node, Module * module);

    /// @brief if语句翻译成线性中间IR
    /// @param node AST节点
    /// @param module 模块
    /// @return 翻译是否成功，true：成功，false：失败
    bool rel_if(ast_node * node, Module * module);

    /// @brief if-else语句翻译成线性中间IR
    /// @param node AST节点
    /// @param module 模块
    /// @return 翻译是否成功，true：成功，false：失败
    bool rel_if_else(ast_node * node, Module * module);

    /// @brief while循环语句翻译成线性中间IR
    /// @param node AST节点
    /// @param module 模块
    /// @return 翻译是否成功，true：成功，false：失败
    bool rel_while(ast_node * node, Module * module);

    /// @brief break语句翻译成线性中间IR
    /// @param node AST节点
    /// @param module 模块
    /// @return 翻译是否成功，true：成功，false：失败
    bool rel_break(ast_node * node, Module * module);

    /// @brief continue语句翻译成线性中间IR
    /// @param node AST节点
    /// @param module 模块
    /// @return 翻译是否成功，true：成功，false：失败
    bool rel_continue(ast_node * node, Module * module);

} // namespace RelationalOpGenerator
