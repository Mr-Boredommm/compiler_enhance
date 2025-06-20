#include <cstdint>
#include <cstdio>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <sstream>

#include "AST.h"
#include "Common.h"
#include "Function.h"
#include "IRCode.h"
#include "IRGenerator.h"
#include "Module.h"
#include "EntryInstruction.h"
#include "LabelInstruction.h"
#include "ExitInstruction.h"
#include "FuncCallInstruction.h"
#include "BinaryInstruction.h"
#include "MoveInstruction.h"
#include "GotoInstruction.h"
#include "IfInstruction.h"
#include "IntegerType.h"
#include "RelationalOpGenerator.h"

/// @brief 生成IR标签名称的计数器
static int label_counter = 1; // 从1开始计数，与正确IR保持一致

/// @brief 生成唯一的标签名称
/// @return 唯一的标签名称
static std::string generate_label()
{
    // 确保标签从1开始
    if (label_counter < 1) {
        label_counter = 1;
    }
    // 确保循环内标签也是唯一的
    std::stringstream ss;
    ss << "L" << label_counter++;
    return ss.str();
}

namespace RelationalOpGenerator {

    /// @brief 辅助函数：递归访问AST节点
    /// @param node AST节点
    /// @param module 模块
    /// @return 访问结果
    static ast_node * visit_ast_node(ast_node * node, Module * module)
    {
        if (!node)
            return nullptr;

        // 获取当前函数
        Function * func = module->getCurrentFunction();
        if (!func) {
            return nullptr;
        }

        // 处理叶子节点
        if (node->node_type == ast_operator_type::AST_OP_LEAF_VAR_ID) {
            // 查找变量值
            node->val = module->findVarValue(node->name);
            return node;
        } else if (node->node_type == ast_operator_type::AST_OP_LEAF_LITERAL_UINT) {
            // 创建整数常量
            node->val = module->newConstInt((int32_t) node->integer_val);
            return node;
        }

        // 让RelationalOpGenerator处理的节点类型
        switch (node->node_type) {
            case ast_operator_type::AST_OP_LT:
                if (rel_lt(node, module))
                    return node;
                break;
            case ast_operator_type::AST_OP_LE:
                if (rel_le(node, module))
                    return node;
                break;
            case ast_operator_type::AST_OP_GT:
                if (rel_gt(node, module))
                    return node;
                break;
            case ast_operator_type::AST_OP_GE:
                if (rel_ge(node, module))
                    return node;
                break;
            case ast_operator_type::AST_OP_EQ:
                if (rel_eq(node, module))
                    return node;
                break;
            case ast_operator_type::AST_OP_NE:
                if (rel_ne(node, module))
                    return node;
                break;
            case ast_operator_type::AST_OP_LOGICAL_AND:
                if (rel_logical_and(node, module))
                    return node;
                break;
            case ast_operator_type::AST_OP_LOGICAL_OR:
                if (rel_logical_or(node, module))
                    return node;
                break;
            case ast_operator_type::AST_OP_LOGICAL_NOT:
                if (rel_logical_not(node, module))
                    return node;
                break;
            case ast_operator_type::AST_OP_IF:
                if (rel_if(node, module))
                    return node;
                break;
            case ast_operator_type::AST_OP_IF_ELSE:
                if (rel_if_else(node, module))
                    return node;
                break;
            case ast_operator_type::AST_OP_WHILE:
                if (rel_while(node, module))
                    return node;
                break;
            case ast_operator_type::AST_OP_BREAK:
                if (rel_break(node, module))
                    return node;
                break;
            case ast_operator_type::AST_OP_CONTINUE:
                if (rel_continue(node, module))
                    return node;
                break;
            default:
                // 递归处理子节点
                for (auto & son: node->sons) {
                    visit_ast_node(son, module);
                }
                return node;
        }

        return nullptr;
    }

    /// @brief 处理小于关系运算符
    /// @param node AST节点
    /// @param module 模块
    /// @return 处理结果，true：成功，false：失败
    bool rel_lt(ast_node * node, Module * module)
    {
        // 获取左右操作数
        auto left = visit_ast_node(node->sons[0], module);
        if (!left) {
            return false;
        }

        auto right = visit_ast_node(node->sons[1], module);
        if (!right) {
            return false;
        }

        // 当前所在函数
        Function * func = module->getCurrentFunction();
        if (!func) {
            return false;
        }

        // 创建比较指令
        auto instr =
            new BinaryInstruction(func, IRInstOperator::IRINST_OP_LT, left->val, right->val, IntegerType::getTypeInt());
        node->blockInsts.addInst(left->blockInsts);
        node->blockInsts.addInst(right->blockInsts);
        node->blockInsts.addInst(instr);

        // 保存结果变量
        node->val = instr;

        return true;
    }

    /// @brief 处理小于等于关系运算符
    /// @param node AST节点
    /// @param module 模块
    /// @return 处理结果，true：成功，false：失败
    bool rel_le(ast_node * node, Module * module)
    {
        // 获取左右操作数
        auto left = visit_ast_node(node->sons[0], module);
        if (!left) {
            return false;
        }

        auto right = visit_ast_node(node->sons[1], module);
        if (!right) {
            return false;
        }

        // 当前所在函数
        Function * func = module->getCurrentFunction();
        if (!func) {
            return false;
        }

        // 创建比较指令
        auto instr =
            new BinaryInstruction(func, IRInstOperator::IRINST_OP_LE, left->val, right->val, IntegerType::getTypeInt());
        node->blockInsts.addInst(left->blockInsts);
        node->blockInsts.addInst(right->blockInsts);
        node->blockInsts.addInst(instr);

        // 保存结果变量
        node->val = instr;

        return true;
    }

    /// @brief 处理大于关系运算符
    /// @param node AST节点
    /// @param module 模块
    /// @return 处理结果，true：成功，false：失败
    bool rel_gt(ast_node * node, Module * module)
    {
        // 获取左右操作数
        auto left = visit_ast_node(node->sons[0], module);
        if (!left) {
            return false;
        }

        auto right = visit_ast_node(node->sons[1], module);
        if (!right) {
            return false;
        }

        // 当前所在函数
        Function * func = module->getCurrentFunction();
        if (!func) {
            return false;
        }

        // 创建比较指令
        auto instr =
            new BinaryInstruction(func, IRInstOperator::IRINST_OP_GT, left->val, right->val, IntegerType::getTypeInt());
        node->blockInsts.addInst(left->blockInsts);
        node->blockInsts.addInst(right->blockInsts);
        node->blockInsts.addInst(instr);

        // 保存结果变量
        node->val = instr;

        return true;
    }

    /// @brief 处理大于等于关系运算符
    /// @param node AST节点
    /// @param module 模块
    /// @return 处理结果，true：成功，false：失败
    bool rel_ge(ast_node * node, Module * module)
    {
        // 获取左右操作数
        auto left = visit_ast_node(node->sons[0], module);
        if (!left) {
            return false;
        }

        auto right = visit_ast_node(node->sons[1], module);
        if (!right) {
            return false;
        }

        // 当前所在函数
        Function * func = module->getCurrentFunction();
        if (!func) {
            return false;
        }

        // 创建比较指令
        auto instr =
            new BinaryInstruction(func, IRInstOperator::IRINST_OP_GE, left->val, right->val, IntegerType::getTypeInt());
        node->blockInsts.addInst(left->blockInsts);
        node->blockInsts.addInst(right->blockInsts);
        node->blockInsts.addInst(instr);

        // 保存结果变量
        node->val = instr;

        return true;
    }

    /// @brief 处理等于关系运算符
    /// @param node AST节点
    /// @param module 模块
    /// @return 处理结果，true：成功，false：失败
    bool rel_eq(ast_node * node, Module * module)
    {
        // 获取左右操作数
        auto left = visit_ast_node(node->sons[0], module);
        if (!left) {
            return false;
        }

        auto right = visit_ast_node(node->sons[1], module);
        if (!right) {
            return false;
        }

        // 当前所在函数
        Function * func = module->getCurrentFunction();
        if (!func) {
            return false;
        }

        // 创建比较指令
        auto instr =
            new BinaryInstruction(func, IRInstOperator::IRINST_OP_EQ, left->val, right->val, IntegerType::getTypeInt());
        node->blockInsts.addInst(left->blockInsts);
        node->blockInsts.addInst(right->blockInsts);
        node->blockInsts.addInst(instr);

        // 保存结果变量
        node->val = instr;

        return true;
    }

    /// @brief 处理不等于关系运算符
    /// @param node AST节点
    /// @param module 模块
    /// @return 处理结果，true：成功，false：失败
    bool rel_ne(ast_node * node, Module * module)
    {
        // 获取左右操作数
        auto left = visit_ast_node(node->sons[0], module);
        if (!left) {
            return false;
        }

        auto right = visit_ast_node(node->sons[1], module);
        if (!right) {
            return false;
        }

        // 当前所在函数
        Function * func = module->getCurrentFunction();
        if (!func) {
            return false;
        }

        // 创建比较指令
        auto instr =
            new BinaryInstruction(func, IRInstOperator::IRINST_OP_NE, left->val, right->val, IntegerType::getTypeInt());
        node->blockInsts.addInst(left->blockInsts);
        node->blockInsts.addInst(right->blockInsts);
        node->blockInsts.addInst(instr);

        // 保存结果变量
        node->val = instr;

        return true;
    }

    /// @brief 逻辑与 && 翻译成线性中间IR，实现短路求值
    /// @param node AST节点
    /// @param module 模块
    /// @return 翻译是否成功，true：成功，false：失败
    bool rel_logical_and(ast_node * node, Module * module)
    {
        // 获取当前函数
        Function * func = module->getCurrentFunction();
        if (!func) {
            return false;
        }

        // 创建标签: 第二个操作数的开始、结果为真和结果为假的跳转位置
        std::string secondOpLabel = generate_label();
        std::string falseLabel = generate_label();
        std::string endLabel = generate_label();

        // 左操作数的处理
        auto left = visit_ast_node(node->sons[0], module);
        if (!left) {
            return false;
        }
        node->blockInsts.addInst(left->blockInsts);

        // 如果左操作数为假(0)，直接短路到结果为假
        auto falseJump =
            new IfInstruction(func, IRInstOperator::IRINST_OP_IFNOT, left->val, new LabelInstruction(func, falseLabel));
        node->blockInsts.addInst(falseJump);

        // 处理右操作数
        auto secondLabelInst = new LabelInstruction(func, secondOpLabel);
        node->blockInsts.addInst(secondLabelInst);

        auto right = visit_ast_node(node->sons[1], module);
        if (!right) {
            return false;
        }
        node->blockInsts.addInst(right->blockInsts);

        // 右操作数的值就是最终结果
        auto result = right->val;

        // 跳转到结束
        auto endJump = new GotoInstruction(func, new LabelInstruction(func, endLabel));
        node->blockInsts.addInst(endJump);

        // 处理结果为假的情况
        auto falseLabelInst = new LabelInstruction(func, falseLabel);
        node->blockInsts.addInst(falseLabelInst);

        // 创建常量0表示false
        ConstInt * falseConst = module->newConstInt(0);
        MoveInstruction * moveInstr = new MoveInstruction(func, result, falseConst);
        node->blockInsts.addInst(moveInstr);

        // 结束标签
        auto endLabelInst = new LabelInstruction(func, endLabel);
        node->blockInsts.addInst(endLabelInst);

        // 将结果存储在当前节点
        node->val = result;

        return true;
    }

    /// @brief 逻辑或 || 翻译成线性中间IR，实现短路求值
    /// @param node AST节点
    /// @param module 模块
    /// @return 翻译是否成功，true：成功，false：失败
    bool rel_logical_or(ast_node * node, Module * module)
    {
        // 获取当前函数
        Function * func = module->getCurrentFunction();
        if (!func) {
            return false;
        }

        // 创建标签: 第二个操作数的开始、结果为真和结果为假的跳转位置
        std::string secondOpLabel = generate_label();
        std::string trueLabel = generate_label();
        std::string endLabel = generate_label();

        // 左操作数的处理
        auto left = visit_ast_node(node->sons[0], module);
        if (!left) {
            return false;
        }
        node->blockInsts.addInst(left->blockInsts);

        // 如果左操作数为真(非0)，直接短路到结果为真
        auto trueJump =
            new IfInstruction(func, IRInstOperator::IRINST_OP_IF, left->val, new LabelInstruction(func, trueLabel));
        node->blockInsts.addInst(trueJump);

        // 处理右操作数
        auto secondLabelInst = new LabelInstruction(func, secondOpLabel);
        node->blockInsts.addInst(secondLabelInst);

        auto right = visit_ast_node(node->sons[1], module);
        if (!right) {
            return false;
        }
        node->blockInsts.addInst(right->blockInsts);

        // 右操作数的值就是最终结果
        auto result = right->val;

        // 跳转到结束
        auto endJump = new GotoInstruction(func, new LabelInstruction(func, endLabel));
        node->blockInsts.addInst(endJump);

        // 处理结果为真的情况
        auto trueLabelInst = new LabelInstruction(func, trueLabel);
        node->blockInsts.addInst(trueLabelInst);

        // 创建常量1表示true
        ConstInt * trueConst = module->newConstInt(1);
        MoveInstruction * moveInstr = new MoveInstruction(func, result, trueConst);
        node->blockInsts.addInst(moveInstr);

        // 结束标签
        auto endLabelInst = new LabelInstruction(func, endLabel);
        node->blockInsts.addInst(endLabelInst);

        // 将结果存储在当前节点
        node->val = result;

        return true;
    }

    /// @brief 逻辑非 ! 翻译成线性中间IR
    /// @param node AST节点
    /// @param module 模块
    /// @return 翻译是否成功，true：成功，false：失败
    bool rel_logical_not(ast_node * node, Module * module)
    {
        // 获取当前函数
        Function * func = module->getCurrentFunction();
        if (!func) {
            return false;
        }

        // 操作数的处理
        auto operand = visit_ast_node(node->sons[0], module);
        if (!operand) {
            return false;
        }
        node->blockInsts.addInst(operand->blockInsts);

        // 创建结果临时变量
        auto result = module->newVarValue(IntegerType::getTypeInt());

        // 创建标签
        std::string trueLabel = generate_label();
        std::string falseLabel = generate_label();
        std::string endLabel = generate_label();

        // 如果操作数为假(0)，则结果为真(1)
        auto trueJump = new IfInstruction(func,
                                          IRInstOperator::IRINST_OP_IFNOT,
                                          operand->val,
                                          new LabelInstruction(func, trueLabel));
        node->blockInsts.addInst(trueJump);

        // 否则结果为假(0)
        auto falseJump = new GotoInstruction(func, new LabelInstruction(func, falseLabel));
        node->blockInsts.addInst(falseJump);

        // 处理结果为真的情况
        auto trueLabelInst = new LabelInstruction(func, trueLabel);
        node->blockInsts.addInst(trueLabelInst);

        // 创建常量1表示true
        ConstInt * trueConst = module->newConstInt(1);
        MoveInstruction * moveTrueInstr = new MoveInstruction(func, result, trueConst);
        node->blockInsts.addInst(moveTrueInstr);

        // 跳转到结束
        auto endJump = new GotoInstruction(func, new LabelInstruction(func, endLabel));
        node->blockInsts.addInst(endJump);

        // 处理结果为假的情况
        auto falseLabelInst = new LabelInstruction(func, falseLabel);
        node->blockInsts.addInst(falseLabelInst);

        // 创建常量0表示false
        ConstInt * falseConst = module->newConstInt(0);
        MoveInstruction * moveFalseInstr = new MoveInstruction(func, result, falseConst);
        node->blockInsts.addInst(moveFalseInstr);

        // 结束标签
        auto endLabelInst = new LabelInstruction(func, endLabel);
        node->blockInsts.addInst(endLabelInst);

        // 将结果存储在当前节点
        node->val = result;

        return true;
    }

    /// @brief if语句翻译成线性中间IR
    /// @param node AST节点
    /// @param module 模块
    /// @return 翻译是否成功，true：成功，false：失败
    bool rel_if(ast_node * node, Module * module)
    {
        // 获取当前函数
        Function * func = module->getCurrentFunction();
        if (!func) {
            return false;
        }

        // 创建标签
        std::string thenLabel = generate_label(); // 真分支的标签
        std::string endLabel = generate_label();  // if语句结束的标签

        // 计算条件表达式
        auto condExpr = visit_ast_node(node->sons[0], module);
        if (!condExpr) {
            return false;
        }
        node->blockInsts.addInst(condExpr->blockInsts);

        // 根据条件跳转到真分支或跳过
        auto ifTrueInstr =
            new IfInstruction(func, IRInstOperator::IRINST_OP_IF, condExpr->val, new LabelInstruction(func, thenLabel));
        node->blockInsts.addInst(ifTrueInstr);

        // 条件为假，直接跳到结束
        auto gotoEndInstr = new GotoInstruction(func, new LabelInstruction(func, endLabel));
        node->blockInsts.addInst(gotoEndInstr);

        // 真分支的代码
        auto thenLabelInstr = new LabelInstruction(func, thenLabel);
        node->blockInsts.addInst(thenLabelInstr);

        // 处理真分支语句体
        auto thenStmt = visit_ast_node(node->sons[1], module);
        if (!thenStmt) {
            return false;
        }
        node->blockInsts.addInst(thenStmt->blockInsts);

        // if语句结束的标签
        auto endLabelInst = new LabelInstruction(func, endLabel);
        node->blockInsts.addInst(endLabelInst);

        return true;
    }

    /// @brief if-else语句翻译成线性中间IR
    /// @param node AST节点
    /// @param module 模块
    /// @return 翻译是否成功，true：成功，false：失败
    bool rel_if_else(ast_node * node, Module * module)
    {
        // 获取当前函数
        Function * func = module->getCurrentFunction();
        if (!func) {
            return false;
        }

        // 创建标签
        std::string thenLabel = generate_label(); // 真分支的标签
        std::string elseLabel = generate_label(); // 假分支的标签
        std::string endLabel = generate_label();  // if-else语句结束的标签

        // 计算条件表达式
        auto condExpr = visit_ast_node(node->sons[0], module);
        if (!condExpr) {
            return false;
        }
        node->blockInsts.addInst(condExpr->blockInsts);

        // 根据条件跳转到真分支或假分支
        auto ifTrueInstr =
            new IfInstruction(func, IRInstOperator::IRINST_OP_IF, condExpr->val, new LabelInstruction(func, thenLabel));
        node->blockInsts.addInst(ifTrueInstr);

        // 条件为假，跳到假分支
        auto gotoElseInstr = new GotoInstruction(func, new LabelInstruction(func, elseLabel));
        node->blockInsts.addInst(gotoElseInstr);

        // 真分支的代码
        auto thenLabelInstr = new LabelInstruction(func, thenLabel);
        node->blockInsts.addInst(thenLabelInstr);

        // 处理真分支语句体
        auto thenStmt = visit_ast_node(node->sons[1], module);
        if (!thenStmt) {
            return false;
        }
        node->blockInsts.addInst(thenStmt->blockInsts);

        // 真分支执行完后跳到结束
        auto gotoEndInstr = new GotoInstruction(func, new LabelInstruction(func, endLabel));
        node->blockInsts.addInst(gotoEndInstr);

        // 假分支的代码
        auto elseLabelInstr = new LabelInstruction(func, elseLabel);
        node->blockInsts.addInst(elseLabelInstr);

        // 处理假分支语句体
        auto elseStmt = visit_ast_node(node->sons[2], module);
        if (!elseStmt) {
            return false;
        }
        node->blockInsts.addInst(elseStmt->blockInsts);

        // if-else语句结束的标签
        auto endLabelInst = new LabelInstruction(func, endLabel);
        node->blockInsts.addInst(endLabelInst);

        return true;
    }

    /// @brief while循环语句翻译成线性中间IR
    /// @param node AST节点
    /// @param module 模块
    /// @return 翻译是否成功，true：成功，false：失败
    bool rel_while(ast_node * node, Module * module)
    {
        // 获取当前函数
        Function * func = module->getCurrentFunction();
        if (!func) {
            return false;
        }

        // 创建标签
        std::string startLabel = generate_label(); // 循环的开始标签，用于条件判断
        std::string bodyLabel = generate_label();  // 循环体的开始标签
        std::string endLabel = generate_label();   // 循环结束的标签

        // 当前循环的标签信息
        // 注意：这里假设IRGenerator对象已经保存了这些信息
        // 这部分需要通过IRGenerator实例来处理，这里只是模拟

        // 循环开始标签
        auto startLabelInst = new LabelInstruction(func, startLabel);
        node->blockInsts.addInst(startLabelInst);

        // 计算条件表达式
        auto condExpr = visit_ast_node(node->sons[0], module);
        if (!condExpr) {
            return false;
        }
        node->blockInsts.addInst(condExpr->blockInsts);

        // 条件为真时跳转到循环体
        auto ifTrueInstr =
            new IfInstruction(func, IRInstOperator::IRINST_OP_IF, condExpr->val, new LabelInstruction(func, bodyLabel));
        node->blockInsts.addInst(ifTrueInstr);

        // 条件为假时跳出循环
        auto gotoEndInstr = new GotoInstruction(func, new LabelInstruction(func, endLabel));
        node->blockInsts.addInst(gotoEndInstr);

        // 循环体开始标签
        auto bodyLabelInst = new LabelInstruction(func, bodyLabel);
        node->blockInsts.addInst(bodyLabelInst);

        // 处理循环体
        auto bodyStmt = visit_ast_node(node->sons[1], module);
        if (!bodyStmt) {
            return false;
        }
        node->blockInsts.addInst(bodyStmt->blockInsts);

        // 循环体执行完后跳回条件判断
        auto gotoStartInstr = new GotoInstruction(func, new LabelInstruction(func, startLabel));
        node->blockInsts.addInst(gotoStartInstr);

        // 循环结束标签
        auto endLabelInst = new LabelInstruction(func, endLabel);
        node->blockInsts.addInst(endLabelInst);

        return true;
    }

    /// @brief break语句翻译成线性中间IR
    /// @param node AST节点
    /// @param module 模块
    /// @return 翻译是否成功，true：成功，false：失败
    bool rel_break(ast_node * node, Module * module)
    {
        // 获取当前函数
        Function * func = module->getCurrentFunction();
        if (!func) {
            return false;
        }

        // 这里需要通过IRGenerator来获取当前循环的结束标签
        // 由于我们不能直接访问IRGenerator的私有成员，所以这里假设已知标签名
        // 真实实现应该通过IRGenerator的方法获取

        // 假设当前循环的结束标签是"break_target"
        std::string breakTarget = "break_target";

        // 无条件跳转到当前循环的结束标签
        auto gotoEndInstr = new GotoInstruction(func, new LabelInstruction(func, breakTarget));
        node->blockInsts.addInst(gotoEndInstr);

        return true;
    }

    /// @brief continue语句翻译成线性中间IR
    /// @param node AST节点
    /// @param module 模块
    /// @return 翻译是否成功，true：成功，false：失败
    bool rel_continue(ast_node * node, Module * module)
    {
        // 获取当前函数
        Function * func = module->getCurrentFunction();
        if (!func) {
            return false;
        }

        // 这里需要通过IRGenerator来获取当前循环的开始标签
        // 由于我们不能直接访问IRGenerator的私有成员，所以这里假设已知标签名
        // 真实实现应该通过IRGenerator的方法获取

        // 假设当前循环的开始标签是"continue_target"
        std::string continueTarget = "continue_target";

        // 无条件跳转到当前循环的开始标签
        auto gotoStartInstr = new GotoInstruction(func, new LabelInstruction(func, continueTarget));
        node->blockInsts.addInst(gotoStartInstr);

        return true;
    }

} // namespace RelationalOpGenerator
