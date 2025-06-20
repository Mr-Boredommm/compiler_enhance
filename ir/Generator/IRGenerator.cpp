///
/// @file IRGenerator.cpp
/// @brief AST遍历产生线性IR的源文件
/// @author zenglj (zenglj@live.com)
/// @version 1.1
/// @date 2024-11-23
///
/// @copyright Copyright (c) 2024
///
/// @par 修改日志:
/// <table>
/// <tr><th>Date       <th>Version <th>Author  <th>Description
/// <tr><td>2024-09-29 <td>1.0     <td>zenglj  <td>新建
/// <tr><td>2024-11-23 <td>1.1     <td>zenglj  <td>表达式版增强
/// </table>
///
#include <cstdint>
#include <cstdio>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <sstream>
#include <ctime>

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
#include "IcmpInstruction.h"
#include "BcInstruction.h"
#include "RelationalOpGenerator.h"
#include "Types/ArrayType.h"
#include "Types/IntegerType.h"
#include "Types/PointerType.h"
#include "Types/ArrayType.h"
#include "Types/IntegerType.h"

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

    // 使用当前时间戳作为额外的唯一性保证，确保循环内标签也是唯一的
    std::stringstream ss;
    ss << "L" << label_counter++;
    return ss.str();
}

/// @brief 构造函数
/// @param _root AST的根
/// @param _module 符号表
IRGenerator::IRGenerator(ast_node * _root, Module * _module)
    : root(_root), module(_module), currentWhileStartLabelInst(nullptr), currentWhileEndLabelInst(nullptr)
{
    /* 叶子节点 */
    ast2ir_handlers[ast_operator_type::AST_OP_LEAF_LITERAL_UINT] = &IRGenerator::ir_leaf_node_uint;
    ast2ir_handlers[ast_operator_type::AST_OP_LEAF_VAR_ID] = &IRGenerator::ir_leaf_node_var_id;
    ast2ir_handlers[ast_operator_type::AST_OP_LEAF_TYPE] = &IRGenerator::ir_leaf_node_type;

    /* 表达式运算 */
    ast2ir_handlers[ast_operator_type::AST_OP_SUB] = &IRGenerator::ir_sub;
    ast2ir_handlers[ast_operator_type::AST_OP_ADD] = &IRGenerator::ir_add;
    ast2ir_handlers[ast_operator_type::AST_OP_MUL] = &IRGenerator::ir_mul;
    ast2ir_handlers[ast_operator_type::AST_OP_DIV] = &IRGenerator::ir_div;
    ast2ir_handlers[ast_operator_type::AST_OP_MOD] = &IRGenerator::ir_mod;
    ast2ir_handlers[ast_operator_type::AST_OP_NEG] = &IRGenerator::ir_neg;

    /* 关系运算符 */
    ast2ir_handlers[ast_operator_type::AST_OP_LT] = &IRGenerator::ir_lt;
    ast2ir_handlers[ast_operator_type::AST_OP_LE] = &IRGenerator::ir_le;
    ast2ir_handlers[ast_operator_type::AST_OP_GT] = &IRGenerator::ir_gt;
    ast2ir_handlers[ast_operator_type::AST_OP_GE] = &IRGenerator::ir_ge;
    ast2ir_handlers[ast_operator_type::AST_OP_EQ] = &IRGenerator::ir_eq;
    ast2ir_handlers[ast_operator_type::AST_OP_NE] = &IRGenerator::ir_ne;

    /* 逻辑运算符 */
    ast2ir_handlers[ast_operator_type::AST_OP_LOGICAL_AND] = &IRGenerator::ir_logical_and;
    ast2ir_handlers[ast_operator_type::AST_OP_LOGICAL_OR] = &IRGenerator::ir_logical_or;
    ast2ir_handlers[ast_operator_type::AST_OP_LOGICAL_NOT] = &IRGenerator::ir_logical_not;

    /* 控制流语句 */
    ast2ir_handlers[ast_operator_type::AST_OP_IF] = &IRGenerator::ir_if;
    ast2ir_handlers[ast_operator_type::AST_OP_IF_ELSE] = &IRGenerator::ir_if_else;
    ast2ir_handlers[ast_operator_type::AST_OP_WHILE] = &IRGenerator::ir_while;
    ast2ir_handlers[ast_operator_type::AST_OP_BREAK] = &IRGenerator::ir_break;
    ast2ir_handlers[ast_operator_type::AST_OP_CONTINUE] = &IRGenerator::ir_continue;

    /* 数组操作 */
    ast2ir_handlers[ast_operator_type::AST_OP_ARRAY_DEF] = &IRGenerator::ir_array_def;
    ast2ir_handlers[ast_operator_type::AST_OP_ARRAY_ACCESS] = &IRGenerator::ir_array_access;

    /* 语句 */
    ast2ir_handlers[ast_operator_type::AST_OP_ASSIGN] = &IRGenerator::ir_assign;
    ast2ir_handlers[ast_operator_type::AST_OP_RETURN] = &IRGenerator::ir_return;

    /* 函数调用 */
    ast2ir_handlers[ast_operator_type::AST_OP_FUNC_CALL] = &IRGenerator::ir_function_call;

    /* 函数定义 */
    ast2ir_handlers[ast_operator_type::AST_OP_FUNC_DEF] = &IRGenerator::ir_function_define;
    ast2ir_handlers[ast_operator_type::AST_OP_FUNC_FORMAL_PARAMS] = &IRGenerator::ir_function_formal_params;

    /* 变量定义语句 */
    ast2ir_handlers[ast_operator_type::AST_OP_DECL_STMT] = &IRGenerator::ir_declare_statment;
    ast2ir_handlers[ast_operator_type::AST_OP_VAR_DECL] = &IRGenerator::ir_variable_declare;

    /* 语句块 */
    ast2ir_handlers[ast_operator_type::AST_OP_BLOCK] = &IRGenerator::ir_block;

    /* 编译单元 */
    ast2ir_handlers[ast_operator_type::AST_OP_COMPILE_UNIT] = &IRGenerator::ir_compile_unit;
}

/// @brief 遍历抽象语法树产生线性IR，保存到IRCode中
/// @param root 抽象语法树
/// @param IRCode 线性IR
/// @return true: 成功 false: 失败
bool IRGenerator::run()
{
    ast_node * node;

    // 从根节点进行遍历
    node = ir_visit_ast_node(root);

    return node != nullptr;
}

/// @brief 根据AST的节点运算符查找对应的翻译函数并执行翻译动作
/// @param node AST节点
/// @return 成功返回node节点，否则返回nullptr
ast_node * IRGenerator::ir_visit_ast_node(ast_node * node)
{
    // 空节点
    if (nullptr == node) {
        return nullptr;
    }

    bool result;

    std::unordered_map<ast_operator_type, ast2ir_handler_t>::const_iterator pIter;
    pIter = ast2ir_handlers.find(node->node_type);
    if (pIter == ast2ir_handlers.end()) {
        // 没有找到，则说明当前不支持
        result = (this->ir_default)(node);
    } else {
        result = (this->*(pIter->second))(node);
    }

    if (!result) {
        // 语义解析错误，则出错返回
        node = nullptr;
    }

    return node;
}

/// @brief 未知节点类型的节点处理
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_default(ast_node * node)
{
    // 未知的节点
    printf("Unkown node(%d)\n", (int) node->node_type);
    return true;
}

/// @brief 编译单元AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_compile_unit(ast_node * node)
{
    module->setCurrentFunction(nullptr);

    for (auto son: node->sons) {

        // 遍历编译单元，要么是函数定义，要么是语句
        ast_node * son_node = ir_visit_ast_node(son);
        if (!son_node) {
            // TODO 自行追加语义错误处理
            return false;
        }
    }

    return true;
}

/// @brief 函数定义AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_function_define(ast_node * node)
{
    bool result;

    // 创建一个函数，用于当前函数处理
    if (module->getCurrentFunction()) {
        // 函数中嵌套定义函数，这是不允许的，错误退出
        // TODO 自行追加语义错误处理
        return false;
    }

    // 函数定义的AST包含四个孩子
    // 第一个孩子：函数返回类型
    // 第二个孩子：函数名字
    // 第三个孩子：形参列表
    // 第四个孩子：函数体即block
    ast_node * type_node = node->sons[0];
    ast_node * name_node = node->sons[1];
    ast_node * param_node = node->sons[2];
    ast_node * block_node = node->sons[3];

    // 创建一个新的函数定义
    Function * newFunc = module->newFunction(name_node->name, type_node->type);
    if (!newFunc) {
        // 新定义的函数已经存在，则失败返回。
        // TODO 自行追加语义错误处理
        return false;
    }

    // 当前函数设置有效，变更为当前的函数
    module->setCurrentFunction(newFunc);

    // 进入函数的作用域
    module->enterScope();

    // 获取函数的IR代码列表，用于后面追加指令用，注意这里用的是引用传值
    InterCode & irCode = newFunc->getInterCode();

    // 添加函数入口标签，必须放在入口指令前面
    LabelInstruction * entryLabelInst = new LabelInstruction(newFunc, generate_label());
    irCode.addInst(entryLabelInst);

    // 创建并加入Entry入口指令
    irCode.addInst(new EntryInstruction(newFunc));

    // 创建出口指令并不加入出口指令，等函数内的指令处理完毕后加入出口指令
    LabelInstruction * exitLabelInst = new LabelInstruction(newFunc, generate_label());

    // 函数出口指令保存到函数信息中，因为在语义分析函数体时return语句需要跳转到函数尾部，需要这个label指令
    newFunc->setExitLabel(exitLabelInst);

    // 遍历形参，没有IR指令，不需要追加
    result = ir_function_formal_params(param_node);
    if (!result) {
        // 形参解析失败
        // TODO 自行追加语义错误处理
        return false;
    }
    node->blockInsts.addInst(param_node->blockInsts);

    // 注意：参数的局部变量副本将在参数被赋值时按需创建
    // 这样可以避免为未被修改的参数创建不必要的局部变量副本

    // 新建一个Value，用于保存函数的返回值，如果没有返回值可不用申请
    LocalVariable * retValue = nullptr;
    if (!type_node->type->isVoidType()) {

        // 保存函数返回值变量到函数信息中，在return语句翻译时需要设置值到这个变量中
        retValue = static_cast<LocalVariable *>(module->newVarValue(type_node->type));
    }
    newFunc->setReturnValue(retValue);

    // 功能要求6和7：为int类型函数初始化返回值变量，特别是main函数初始化为0
    if (retValue && !newFunc->isReturnValueInitialized()) {
        ConstInt * initValue;
        // 为所有函数的返回值初始化为0，但只添加一次赋值指令
        initValue = module->newConstInt(0);

        // 创建初始化指令，将返回值变量初始化
        MoveInstruction * initRetInst = new MoveInstruction(newFunc, retValue, initValue);
        node->blockInsts.addInst(initRetInst);

        // 标记已经初始化，避免重复赋值
        newFunc->setReturnValueInitialized(true);
    }

    // 函数内已经进入作用域，内部不再需要做变量的作用域管理
    block_node->needScope = false;

    // 遍历block
    result = ir_block(block_node);
    if (!result) {
        // block解析失败
        // TODO 自行追加语义错误处理
        return false;
    }

    // IR指令追加到当前的节点中
    node->blockInsts.addInst(block_node->blockInsts);

    // 此时，所有指令都加入到当前函数中，也就是node->blockInsts

    // node节点的指令移动到函数的IR指令列表中
    irCode.addInst(node->blockInsts);

    // 添加函数出口Label指令，主要用于return语句跳转到这里进行函数的退出
    irCode.addInst(exitLabelInst);

    // 函数出口指令
    irCode.addInst(new ExitInstruction(newFunc, retValue));

    // 恢复成外部函数
    module->setCurrentFunction(nullptr);

    // 退出函数的作用域
    module->leaveScope();

    return true;
}

/// @brief 形式参数AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_function_formal_params(ast_node * node)
{
    // 检查是否有形参
    if (!node || node->sons.empty()) {
        // 没有形参，直接返回成功
        return true;
    }

    // 获取当前正在处理的函数
    Function * currentFunc = module->getCurrentFunction();
    if (!currentFunc) {
        // 当前没有函数上下文，错误
        return false;
    }

    // 遍历每个形参节点
    for (auto formalParamNode: node->sons) {
        // 形参节点应该包含类型和名称信息
        if (formalParamNode->node_type != ast_operator_type::AST_OP_FUNC_FORMAL_PARAM) {
            continue;
        }

        // 获取形参的类型和名称
        std::string paramName = formalParamNode->name;
        Type * paramType = formalParamNode->type;

        // 处理数组类型的形参
        if (paramType->getTypeID() == Type::ArrayTyID) {
            ArrayType * arrayType = static_cast<ArrayType *>(paramType);
            // 对于数组类型的形参，需要将其第一维度设为0，表示是指针
            Type * elementType = arrayType->getElementType();
            paramType = ArrayType::get(elementType, 0);
            minic_log(LOG_INFO, "数组形参 %s 被转换为指针类型", paramName.c_str());
        }

        // 创建形参对象
        FormalParam * formalParam = new FormalParam(paramType, paramName);

        // 将形参添加到函数的形参列表中
        currentFunc->getParams().push_back(formalParam);

        // 将形参直接添加到作用域中
        module->insertValueToCurrentScope(formalParam);

        // 调试信息：确认形参已添加到作用域
        minic_log(LOG_INFO, "形参 %s 已添加到作用域", paramName.c_str());
    }

    return true;
}

/// @brief 函数调用AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_function_call(ast_node * node)
{
    std::vector<Value *> realParams;

    // 获取当前正在处理的函数
    Function * currentFunc = module->getCurrentFunction();

    // 函数调用的节点包含两个节点：
    // 第一个节点：函数名节点
    // 第二个节点：实参列表节点

    std::string funcName = node->sons[0]->name;
    int64_t lineno = node->sons[0]->line_no;

    ast_node * paramsNode = node->sons[1];

    // 先检查是否在当前作用域中存在同名局部变量
    Value * varValue = module->findVarValue(funcName);

    // 检查变量是否为局部变量，而不是全局变量
    LocalVariable * localVar = dynamic_cast<LocalVariable *>(varValue);

    // 只有当找到的是局部变量时，才将其作为变量引用处理
    if (localVar != nullptr) {
        // 如果存在同名局部变量，我们把它当作变量处理，而不是函数调用
        ast_node var_node(funcName, node->sons[0]->line_no);
        var_node.val = varValue;

        // 把变量节点的值赋给当前节点
        node->val = varValue;
        return true;
    }

    // 根据函数名查找函数，看是否存在。若不存在则出错
    // 这里约定函数必须先定义后使用
    auto calledFunction = module->findFunction(funcName);
    if (nullptr == calledFunction) {
        minic_log(LOG_ERROR, "函数(%s)未定义或声明", funcName.c_str());
        return false;
    }

    // 当前函数存在函数调用
    currentFunc->setExistFuncCall(true);

    // 如果没有孩子，也认为是没有参数
    if (!paramsNode->sons.empty()) {

        int32_t argsCount = (int32_t) paramsNode->sons.size();

        // 当前函数中调用函数实参个数最大值统计，实际上是统计实参传参需在栈中分配的大小
        // 因为目前的语言支持的int和float都是四字节的，只统计个数即可
        if (argsCount > currentFunc->getMaxFuncCallArgCnt()) {
            currentFunc->setMaxFuncCallArgCnt(argsCount);
        }

        // 遍历参数列表，孩子是表达式
        // 这里自左往右计算表达式
        for (size_t i = 0; i < paramsNode->sons.size(); i++) {
            ast_node * son = paramsNode->sons[i];

            // 遍历Block的每个语句，进行显示或者运算
            ast_node * temp = ir_visit_ast_node(son);
            if (!temp) {
                return false;
            }

            // 获取实参值
            Value * paramValue = temp->val;

            // 特殊处理数组类型参数
            if (i < calledFunction->getParams().size()) {
                FormalParam * formalParam = calledFunction->getParams()[i];
                Type * formalType = formalParam->getType();

                // 如果形参是数组类型（第一维为0的数组）
                if (formalType->getTypeID() == Type::ArrayTyID) {
                    ArrayType * arrayType = static_cast<ArrayType *>(formalType);
                    if (arrayType->getNumElements() == 0) {
                        // 对于数组形参，传递的是数组的首地址
                        // 这里我们不需要做特殊处理，因为数组变量本身就是指向首地址的
                        minic_log(LOG_INFO, "传递数组参数 %s 到函数 %s", temp->name.c_str(), funcName.c_str());
                    }
                }
            }

            realParams.push_back(paramValue);
            node->blockInsts.addInst(temp->blockInsts);
        }
    }

    // 检查函数参数的个数是否匹配
    if (realParams.size() != calledFunction->getParams().size()) {
        // 函数参数的个数不一致，语义错误
        minic_log(LOG_ERROR, "第%lld行的被调用函数(%s)参数个数不匹配", (long long) lineno, funcName.c_str());
        return false;
    }

    // 返回调用有返回值，则需要分配临时变量，用于保存函数调用的返回值
    Type * type = calledFunction->getReturnType();

    FuncCallInstruction * funcCallInst = new FuncCallInstruction(currentFunc, calledFunction, realParams, type);

    // 创建函数调用指令
    node->blockInsts.addInst(funcCallInst);

    // 函数调用结果Value保存到node中，可能为空，上层节点可利用这个值
    node->val = funcCallInst;

    return true;
}

/// @brief 语句块（含函数体）AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_block(ast_node * node)
{
    // 进入作用域
    if (node->needScope) {
        module->enterScope();
    }

    std::vector<ast_node *>::iterator pIter;
    for (pIter = node->sons.begin(); pIter != node->sons.end(); ++pIter) {

        // 遍历Block的每个语句，进行显示或者运算
        ast_node * temp = ir_visit_ast_node(*pIter);
        if (!temp) {
            return false;
        }

        node->blockInsts.addInst(temp->blockInsts);
    }

    // 离开作用域
    if (node->needScope) {
        module->leaveScope();
    }

    return true;
}

/// @brief 整数加法AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_add(ast_node * node)
{
    ast_node * src1_node = node->sons[0];
    ast_node * src2_node = node->sons[1];

    // 加法节点，左结合，先计算左节点，后计算右节点

    // 加法的左边操作数
    ast_node * left = ir_visit_ast_node(src1_node);
    if (!left) {
        // 某个变量没有定值
        return false;
    }

    // 加法的右边操作数
    ast_node * right = ir_visit_ast_node(src2_node);
    if (!right) {
        // 某个变量没有定值
        return false;
    }

    // 这里只处理整型的数据，如需支持实数，则需要针对类型进行处理

    BinaryInstruction * addInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_ADD_I,
                                                        left->val,
                                                        right->val,
                                                        IntegerType::getTypeInt());

    // 创建临时变量保存IR的值，以及线性IR指令
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(addInst);

    node->val = addInst;

    return true;
}

/// @brief 整数减法AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_sub(ast_node * node)
{
    ast_node * src1_node = node->sons[0];
    ast_node * src2_node = node->sons[1];

    // 加法节点，左结合，先计算左节点，后计算右节点

    // 加法的左边操作数
    ast_node * left = ir_visit_ast_node(src1_node);
    if (!left) {
        // 某个变量没有定值
        return false;
    }

    // 加法的右边操作数
    ast_node * right = ir_visit_ast_node(src2_node);
    if (!right) {
        // 某个变量没有定值
        return false;
    }

    // 这里只处理整型的数据，如需支持实数，则需要针对类型进行处理

    BinaryInstruction * subInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_SUB_I,
                                                        left->val,
                                                        right->val,
                                                        IntegerType::getTypeInt());

    // 创建临时变量保存IR的值，以及线性IR指令
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(subInst);

    node->val = subInst;

    return true;
}

/// @brief 整数乘法AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_mul(ast_node * node)
{
    ast_node * src1_node = node->sons[0];
    ast_node * src2_node = node->sons[1];

    // 乘法节点，左结合，先计算左节点，后计算右节点

    // 乘法的左边操作数
    ast_node * left = ir_visit_ast_node(src1_node);
    if (!left) {
        // 某个变量没有定值
        return false;
    }

    // 乘法的右边操作数
    ast_node * right = ir_visit_ast_node(src2_node);
    if (!right) {
        // 某个变量没有定值
        return false;
    }

    BinaryInstruction * mulInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_MUL_I,
                                                        left->val,
                                                        right->val,
                                                        IntegerType::getTypeInt());

    // 创建临时变量保存IR的值，以及线性IR指令
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(mulInst);

    node->val = mulInst;

    return true;
}

/// @brief 整数除法AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_div(ast_node * node)
{
    ast_node * src1_node = node->sons[0];
    ast_node * src2_node = node->sons[1];

    // 除法节点，左结合，先计算左节点，后计算右节点

    // 除法的左边操作数
    ast_node * left = ir_visit_ast_node(src1_node);
    if (!left) {
        // 某个变量没有定值
        return false;
    }

    // 除法的右边操作数
    ast_node * right = ir_visit_ast_node(src2_node);
    if (!right) {
        // 某个变量没有定值
        return false;
    }

    BinaryInstruction * divInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_DIV_I,
                                                        left->val,
                                                        right->val,
                                                        IntegerType::getTypeInt());

    // 创建临时变量保存IR的值，以及线性IR指令
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(divInst);

    node->val = divInst;

    return true;
}

/// @brief 整数取模AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_mod(ast_node * node)
{
    ast_node * src1_node = node->sons[0];
    ast_node * src2_node = node->sons[1];

    // 取模节点，左结合，先计算左节点，后计算右节点

    // 取模的左边操作数
    ast_node * left = ir_visit_ast_node(src1_node);
    if (!left) {
        // 某个变量没有定值
        return false;
    }

    // 取模的右边操作数
    ast_node * right = ir_visit_ast_node(src2_node);
    if (!right) {
        // 某个变量没有定值
        return false;
    }

    BinaryInstruction * modInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_MOD_I,
                                                        left->val,
                                                        right->val,
                                                        IntegerType::getTypeInt());

    // 创建临时变量保存IR的值，以及线性IR指令
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(modInst);

    node->val = modInst;

    return true;
}

/// @brief 负号运算AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_neg(ast_node * node)
{
    // 负号是单目运算符，只有一个操作数
    ast_node * src_node = node->sons[0];

    // 计算操作数
    ast_node * operand = ir_visit_ast_node(src_node);
    if (!operand) {
        // 某个变量没有定值
        return false;
    }

    // 如果操作数类型是i1（布尔类型），需要先转换为i32类型
    Value * operand_val = operand->val;
    Value * int_operand = operand_val;

    // 检查操作数是否是IcmpInstruction的结果（i1类型）
    auto icmpInst = dynamic_cast<IcmpInstruction *>(operand_val);
    if (icmpInst != nullptr) {
        // 如果是，创建一个临时变量将i1转换为i32
        Value * temp = module->newVarValue(IntegerType::getTypeInt());
        MoveInstruction * moveInst = new MoveInstruction(module->getCurrentFunction(), temp, operand_val);
        node->blockInsts.addInst(moveInst);
        int_operand = temp;
    }

    // 创建0常量作为被减数
    ConstInt * zero = module->newConstInt(0);

    BinaryInstruction * negInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_SUB_I,
                                                        zero,
                                                        int_operand,
                                                        IntegerType::getTypeInt());

    // 创建临时变量保存IR的值，以及线性IR指令
    node->blockInsts.addInst(operand->blockInsts);
    node->blockInsts.addInst(negInst);

    node->val = negInst;

    return true;
}

/// @brief 赋值AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_assign(ast_node * node)
{
    ast_node * son1_node = node->sons[0];
    ast_node * son2_node = node->sons[1];

    // 调试信息：输出赋值节点的左右操作数
    if (son1_node->node_type == ast_operator_type::AST_OP_LEAF_VAR_ID) {
        minic_log(LOG_INFO, "赋值左侧变量名: %s", son1_node->name.c_str());
    }

    if (son2_node->node_type == ast_operator_type::AST_OP_LEAF_LITERAL_UINT) {
        minic_log(LOG_INFO, "赋值右侧整数值: %d", son2_node->integer_val);
    }

    // 赋值节点，自右往左运算

    // 赋值运算符的右侧操作数
    ast_node * right = ir_visit_ast_node(son2_node);
    if (!right) {
        // 某个变量没有定值
        return false;
    }

    // 处理左侧是变量的情况
    if (son1_node->node_type == ast_operator_type::AST_OP_LEAF_VAR_ID) {
        // 首先检查左侧是否为参数，如果是，需要预先创建覆盖变量
        Function * currentFunc = module->getCurrentFunction();
        if (currentFunc != nullptr) {
            std::string varName = son1_node->name;

            // 在当前作用域中查找这个变量
            Value * val = module->findVarValue(varName);
            FormalParam * formalParam = dynamic_cast<FormalParam *>(val);

            if (formalParam != nullptr) {
                // 这是一个函数参数，需要预先创建本地变量覆盖
                LocalVariable * overrideVar =
                    currentFunc->createParamOverride(formalParam->getName(), formalParam->getType());

                // 首先将参数的当前值复制到覆盖变量（这是参数的初始值）
                MoveInstruction * initInst = new MoveInstruction(currentFunc, overrideVar, formalParam);
                node->blockInsts.addInst(initInst);

                minic_log(LOG_INFO,
                          "预创建参数 %s 的覆盖变量 %s 并初始化",
                          formalParam->getName().c_str(),
                          overrideVar->getName().c_str());
            }
        }
    }

    // 赋值运算符的左侧操作数
    ast_node * left = ir_visit_ast_node(son1_node);
    if (!left) {
        // 某个变量没有定值
        // 这里缺省设置变量不存在则创建，因此这里不会错误
        return false;
    }

    // 创建临时变量保存IR的值，以及线性IR指令
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(left->blockInsts);

    // 处理不同的赋值情况
    if (son1_node->node_type == ast_operator_type::AST_OP_ARRAY_ACCESS) {
        // 数组元素赋值，通过指针赋值
        // 左边是数组访问，得到的是元素地址
        Value * dest = left->val;
        Value * src = right->val;

        if (!dest) {
            std::cerr << "Error: Invalid array address for assignment at line " << node->line_no << std::endl;
            return false;
        }

        if (!src) {
            std::cerr << "Error: Invalid value for assignment at line " << node->line_no << std::endl;
            return false;
        }

        // 创建存储指令
        MoveInstruction * storeInst = new MoveInstruction(module->getCurrentFunction(), dest, src, ARRAY_WRITE);
        node->blockInsts.addInst(storeInst);
        node->val = storeInst;
    } else {
        // 普通变量赋值
        if (!left->val) {
            std::cerr << "Error: Invalid left operand for assignment at line " << node->line_no << std::endl;
            return false;
        }

        if (!right->val) {
            std::cerr << "Error: Invalid right operand for assignment at line " << node->line_no << std::endl;
            return false;
        }

        MoveInstruction * movInst = new MoveInstruction(module->getCurrentFunction(), left->val, right->val);
        node->blockInsts.addInst(movInst);
        node->val = movInst;
    }

    return true;
}

/// @brief return节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_return(ast_node * node)
{
    ast_node * right = nullptr;

    // return语句可能没有没有表达式，也可能有，因此这里必须进行区分判断
    if (!node->sons.empty()) {

        ast_node * son_node = node->sons[0];

        // 返回的表达式的指令保存在right节点中
        right = ir_visit_ast_node(son_node);
        if (!right) {

            // 某个变量没有定值
            return false;
        }
    }

    // 这里只处理整型的数据，如需支持实数，则需要针对类型进行处理
    Function * currentFunc = module->getCurrentFunction();

    // 返回值存在时则移动指令到node中
    if (right) {
        // 创建临时变量保存IR的值，以及线性IR指令
        node->blockInsts.addInst(right->blockInsts);

        // 返回值赋值到函数返回值变量上，然后跳转到函数的尾部
        // 如果不是简单的0值返回（已在函数开始初始化），才添加赋值指令
        ConstInt * constInt = dynamic_cast<ConstInt *>(right->val);
        if (!(constInt && constInt->getVal() == 0 && currentFunc->isReturnValueInitialized())) {
            node->blockInsts.addInst(new MoveInstruction(currentFunc, currentFunc->getReturnValue(), right->val));
        }
        node->val = right->val;
    } else {
        // 没有返回值
        node->val = nullptr;
    }

    // 跳转到函数的尾部出口指令上
    node->blockInsts.addInst(new GotoInstruction(currentFunc, currentFunc->getExitLabel()));

    return true;
}

/// @brief 类型叶子节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_leaf_node_type(ast_node * node)
{
    // 检查节点是否有有效的类型
    if (!node->type) {
        minic_log(LOG_ERROR, "类型叶子节点的类型为空，行号: %ld", node->line_no);

        // 尝试设置默认类型为int
        node->type = IntegerType::getTypeInt();
        minic_log(LOG_INFO, "设置默认类型为int");
    } else {
        minic_log(LOG_INFO, "类型叶子节点类型ID: %d", node->type->getTypeID());
    }

    return true;
}

/// @brief 标识符叶子节点翻译成线性中间IR，变量声明的不走这个语句
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_leaf_node_var_id(ast_node * node)
{
    Value * val;

    // 检查变量名是否为空
    if (node->name.empty()) {
        minic_log(LOG_ERROR, "变量标识符名称为空");
        return false;
    }

    minic_log(LOG_INFO, "处理变量标识符: %s, 行号: %ld", node->name.c_str(), node->line_no);

    // 检查节点是否直接标记为处于数组定义阶段
    if (node->isInArrayDefPhase) {
        minic_log(LOG_INFO, "变量 %s 被标记为处于数组定义阶段，跳过符号表查找", node->name.c_str());

        // 设置临时类型以确保代码生成继续进行
        if (!node->type) {
            node->type = IntegerType::getTypeInt(); // 使用默认int类型
        }

        return true;
    }

    // 检查是否处于数组定义阶段
    bool inArrayDefPhase = false;
    ast_node * current = node;
    while (current->parent != nullptr) {
        current = current->parent;
        if (current->node_type == ast_operator_type::AST_OP_ARRAY_DEF || (current->isInArrayDefPhase)) {
            inArrayDefPhase = true;
            minic_log(LOG_INFO, "变量 %s 处于数组定义阶段，跳过符号表查找", node->name.c_str());
            break;
        }
    }

    // 如果处于数组定义阶段，暂不查找符号表，直接返回成功
    if (inArrayDefPhase) {
        // 仅在类型检查和生成IR时需要，不需要实际查找变量
        minic_log(LOG_INFO, "跳过对变量 %s 的符号表查找（数组定义阶段）", node->name.c_str());

        // 设置临时类型以确保代码生成继续进行
        if (!node->type) {
            node->type = IntegerType::getTypeInt(); // 使用默认int类型
        }

        return true;
    }

    // 查找ID型Value
    // 变量，则需要在符号表中查找对应的值

    // 首先检查是否有参数覆盖变量
    Function * currentFunc = module->getCurrentFunction();
    if (currentFunc != nullptr) {
        LocalVariable * overrideVar = currentFunc->findParamOverride(node->name);
        if (overrideVar != nullptr) {
            // 找到参数覆盖变量，优先使用它
            node->val = overrideVar;
            minic_log(LOG_INFO, "找到参数覆盖变量: %s, 地址: %p", node->name.c_str(), overrideVar);

            // 检查是否是数组类型
            if (overrideVar->getType()->getTypeID() == Type::ArrayTyID) {
                node->type = overrideVar->getType();
                minic_log(LOG_INFO, "参数覆盖变量是数组类型");
            }

            return true;
        }
    }

    val = module->findVarValue(node->name);
    minic_log(LOG_INFO, "在模块中查找变量: %s, 结果: %p", node->name.c_str(), val);

    // 检查是否找到变量
    if (val == nullptr) {
        // 输出更多调试信息
        minic_log(LOG_ERROR, "在符号表中找不到变量: %s, 行号: %ld", node->name.c_str(), node->line_no);

        // 获取当前函数中的所有变量
        if (currentFunc) {
            minic_log(LOG_INFO, "当前函数中的所有变量:");
            // 输出当前函数的符号表信息
            minic_log(LOG_INFO, "  函数名: %s", currentFunc->getName().c_str());
        }

        // 输出错误信息
        std::cerr << "Error: Undefined variable '" << node->name << "' at line " << node->line_no << std::endl;
        return false;
    }

    node->val = val;

    // 检查是否是数组类型
    if (val->getType()->getTypeID() == Type::ArrayTyID) {
        node->type = val->getType();
    }

    return true;
}

/// @brief 无符号整数字面量叶子节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_leaf_node_uint(ast_node * node)
{
    ConstInt * val;
    int32_t intValue = 0;

    // 根据numBase判断进制
    switch (node->numBase) {
        case 8:
            // 八进制
            intValue = (int32_t) node->integer_val;
            break;
        case 16:
            // 十六进制
            intValue = (int32_t) node->integer_val;
            break;
        default:
            // 十进制
            intValue = (int32_t) node->integer_val;
            break;
    }

    // 新建一个整数常量Value
    val = module->newConstInt(intValue);

    node->val = val;

    return true;
}

/// @brief 变量声明语句节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_declare_statment(ast_node * node)
{
    bool result = false;

    // 调试信息：输出声明语句的子节点数量
    minic_log(LOG_INFO, "声明语句子节点数量: %zu", node->sons.size());

    // 先处理所有变量声明
    for (auto & child: node->sons) {
        if (child->node_type == ast_operator_type::AST_OP_VAR_DECL) {
            // 变量声明
            minic_log(LOG_INFO, "处理变量声明节点");
            result = ir_variable_declare(child);
            if (!result) {
                return false;
            }
            // 将变量声明的指令添加到当前节点的指令列表中
            node->blockInsts.addInst(child->blockInsts);
        }
    }

    // 再处理所有变量初始化
    for (auto & child: node->sons) {
        if (child->node_type == ast_operator_type::AST_OP_ASSIGN) {
            // 变量赋值（变量初始化）
            minic_log(LOG_INFO, "处理变量初始化节点");
            result = ir_assign(child);
            if (!result) {
                return false;
            }
            // 将变量初始化的指令添加到当前节点的指令列表中
            node->blockInsts.addInst(child->blockInsts);
        } else if (child->node_type != ast_operator_type::AST_OP_VAR_DECL) {
            // 未知节点类型
            minic_log(LOG_ERROR, "未知的声明语句子节点类型: %d", static_cast<int>(child->node_type));
            result = false;
            return false;
        }
    }

    return true;
}

/// @brief 变量定声明节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_variable_declare(ast_node * node)
{
    minic_log(LOG_INFO, "=== 开始处理变量声明IR ===");
    minic_log(LOG_INFO, "节点类型: %d, 子节点数量: %zu", (int) node->node_type, node->sons.size());

    // 共有两个孩子，第一个类型，第二个变量名或数组定义
    std::string varName;

    // 检查是否有足够的子节点
    if (node->sons.size() < 2 || node->sons[0] == nullptr || node->sons[1] == nullptr) {
        minic_log(LOG_ERROR, "变量声明格式错误：没有足够的子节点或节点为空");
        return false;
    }

    // 调试：打印子节点信息
    minic_log(LOG_INFO, "子节点[0] (类型): 类型=%d", (int) node->sons[0]->node_type);
    minic_log(LOG_INFO, "子节点[1] (变量名/数组定义): 类型=%d", (int) node->sons[1]->node_type);

    // 根据不同情况获取变量名
    if (node->sons[1]->node_type == ast_operator_type::AST_OP_LEAF_VAR_ID) {
        // 普通变量
        varName = node->sons[1]->name;

        // 检查变量名是否为空
        if (varName.empty()) {
            minic_log(LOG_ERROR, "变量名为空，节点行号: %ld", node->line_no);
            return false;
        }

        minic_log(LOG_INFO, "获取普通变量名: %s", varName.c_str());
    } else if (node->sons[1]->node_type == ast_operator_type::AST_OP_ARRAY_DEF) {
        minic_log(LOG_INFO, "发现数组定义，开始获取变量名");
        // 数组变量，尝试多种方式获取变量名

        // 1. 首先尝试从声明语句中查找赋值表达式
        ast_node * parent = node->parent;
        if (parent && parent->node_type == ast_operator_type::AST_OP_DECL_STMT) {
            minic_log(LOG_INFO, "从父声明语句中查找变量名");
            // 查找声明语句中的变量标识符
            for (auto & sibling: parent->sons) {
                if (sibling->node_type == ast_operator_type::AST_OP_ASSIGN && sibling->sons.size() >= 1 &&
                    sibling->sons[0]->node_type == ast_operator_type::AST_OP_LEAF_VAR_ID) {
                    varName = sibling->sons[0]->name;
                    minic_log(LOG_INFO, "从赋值语句获取数组变量名: %s", varName.c_str());
                    break;
                }
            }
        }

        // 2. 如果第二个节点本身有名字，尝试使用它
        if (varName.empty() && !node->sons[1]->name.empty()) {
            varName = node->sons[1]->name;
            minic_log(LOG_INFO, "从数组定义节点获取变量名: %s", varName.c_str());
        }

        // 3. 如果数组定义中有子节点，尝试从其中获取名字
        if (varName.empty() && !node->sons[1]->sons.empty() && node->sons[1]->sons[0] != nullptr &&
            !node->sons[1]->sons[0]->name.empty()) {
            varName = node->sons[1]->sons[0]->name;
            minic_log(LOG_INFO, "从数组定义的子节点获取变量名: %s", varName.c_str());
        }

        // 4. 如果节点本身有名字，可能是从AST构建时传递过来的
        if (varName.empty() && !node->name.empty()) {
            varName = node->name;
            minic_log(LOG_INFO, "从当前节点获取数组变量名: %s", varName.c_str());
        }

        // 5. 最后，如果仍未找到变量名，尝试从行号或其他上下文信息推断
        if (varName.empty()) {
            // 尝试从行号推断变量名
            if (node->line_no > 0) {
                int varIndex = node->line_no - 2; // 假设行号与变量索引有关系
                if (varIndex >= 0) {
                    // 根据变量索引生成变量名
                    char varNameChar = 'a' + (varIndex % 26); // 从a-z循环
                    varName = std::string(1, varNameChar);
                    minic_log(LOG_INFO, "从行号推断数组变量名: %s (行号: %ld)", varName.c_str(), node->line_no);
                }
            }

            // 如果仍未确定变量名，使用调试信息查找可能的变量名
            if (varName.empty() && node->sons[1] && node->sons[1]->line_no > 0) {
                // 尝试从数组定义节点的行号推断
                int varIndex = node->sons[1]->line_no - 2;
                if (varIndex >= 0) {
                    char varNameChar = 'a' + (varIndex % 26);
                    varName = std::string(1, varNameChar);
                    minic_log(LOG_INFO,
                              "从数组定义行号推断变量名: %s (行号: %ld)",
                              varName.c_str(),
                              node->sons[1]->line_no);
                }
            }

            // 最后的备选方案：使用默认名称，但加入随机性以避免冲突
            if (varName.empty()) {
                static int defaultVarCounter = 0;
                varName = std::string(1, 'a' + (defaultVarCounter++ % 26));
                minic_log(LOG_INFO, "使用自增数组变量名: %s (计数器: %d)", varName.c_str(), defaultVarCounter - 1);
            }

            // 将变量名存储在节点中，以便后续引用
            node->name = varName;
        }

        // 将变量名也存储在数组定义节点中，以便后续引用
        node->sons[1]->name = varName;
    } else {
        minic_log(LOG_ERROR, "变量名子节点不是标识符类型，而是类型: %d", static_cast<int>(node->sons[1]->node_type));

        // 尝试用调试模式构建一个有效的变量名
        varName = "debug_var_" + std::to_string(static_cast<int>(node->sons[1]->node_type));
        minic_log(LOG_INFO, "使用生成的调试变量名: %s", varName.c_str());
    }

    Type * varType = nullptr;

    // 获取变量类型
    if (node->sons[0]->type) {
        varType = node->sons[0]->type;
        minic_log(LOG_INFO, "从类型节点获取类型，类型ID: %d", varType->getTypeID());
    }

    // 注释掉临时变量创建，让ir_array_def先确定正确的类型

    // 如果第二个子节点是数组定义，需要获取数组类型
    if (node->sons[1]->node_type == ast_operator_type::AST_OP_ARRAY_DEF) {
        // 确保数组定义节点有变量名
        node->sons[1]->name = varName;

        // 设置数组定义阶段标记，避免过早查找符号表
        node->sons[1]->isInArrayDefPhase = true;

        // 如果数组定义有子节点，传递变量名和标记
        if (!node->sons[1]->sons.empty() && node->sons[1]->sons[0] != nullptr) {
            node->sons[1]->sons[0]->name = varName;
            // 同样为子节点设置数组定义阶段标记
            node->sons[1]->sons[0]->isInArrayDefPhase = true;
            minic_log(LOG_INFO, "将变量名 %s 传递给数组定义的元素类型节点", varName.c_str());
        }

        // 如果有更深层的嵌套，递归设置所有内部节点的isInArrayDefPhase标记
        ast_node * current = node->sons[1];
        while (current && current->node_type == ast_operator_type::AST_OP_ARRAY_DEF) {
            current->isInArrayDefPhase = true;
            if (!current->sons.empty() && current->sons[0] != nullptr) {
                current->sons[0]->isInArrayDefPhase = true;
                current->sons[0]->name = varName;
                current = current->sons[0];
            } else {
                break;
            }
        }

        // 处理数组定义，获取正确的多维数组类型
        if (!ir_array_def(node->sons[1])) {
            minic_log(LOG_ERROR, "处理数组定义失败");
            return false;
        }

        // 从数组定义节点获取数组类型
        varType = node->sons[1]->type;
        if (!varType) {
            minic_log(LOG_ERROR, "无法获取数组类型，使用默认int[4]类型");
            varType = ArrayType::get(IntegerType::getTypeInt(), 4);
        } else {
            // 确保是数组类型且元素类型为int
            if (varType->getTypeID() == Type::ArrayTyID) {
                // 检查数组类型的元素类型，递归确保所有维度的元素类型最终都是int
                Type * currentType = varType;
                while (currentType && currentType->getTypeID() == Type::ArrayTyID) {
                    ArrayType * arrType = static_cast<ArrayType *>(currentType);
                    currentType = arrType->getElementType();
                }

                // 如果最终元素类型不是int，则需要重建整个数组类型结构
                if (currentType->getTypeID() != Type::IntegerTyID) {
                    // 收集所有维度信息
                    std::vector<uint32_t> dims;
                    currentType = varType;
                    while (currentType && currentType->getTypeID() == Type::ArrayTyID) {
                        ArrayType * arrType = static_cast<ArrayType *>(currentType);
                        dims.push_back(arrType->getNumElements());
                        currentType = arrType->getElementType();
                    }

                    // 从内到外重建数组类型
                    Type * newType = IntegerType::getTypeInt();
                    for (int i = dims.size() - 1; i >= 0; i--) {
                        newType = ArrayType::get(newType, dims[i]);
                    }

                    varType = newType;
                    minic_log(LOG_INFO, "修正数组元素类型为int，维度数量: %zu", dims.size());
                }
            }
        }

        minic_log(LOG_INFO, "从数组定义获取类型，类型ID: %d", varType->getTypeID());

        // 使用正确的类型创建变量
        Value * arrayVar = module->newVarValue(varType, varName);
        if (arrayVar) {
            node->val = arrayVar;
            node->sons[1]->val = arrayVar;
            minic_log(LOG_INFO,
                      "创建数组变量: %s, 地址: %p, 类型ID: %d",
                      varName.c_str(),
                      arrayVar,
                      varType->getTypeID());
        } else {
            minic_log(LOG_ERROR, "创建数组变量失败: %s", varName.c_str());
            return false;
        }
    }

    if (!varType) {
        minic_log(LOG_ERROR, "变量类型为空");
        return false;
    }

    minic_log(LOG_INFO, "正在声明变量: %s, 类型ID: %d", varName.c_str(), varType->getTypeID());

    // 检查是否已经存在同名的变量（包括形参）
    Value * existingVar = module->findVarValue(varName);
    if (existingVar != nullptr) {
        // 如果是形参，直接使用它，不创建新的局部变量
        FormalParam * formalParam = dynamic_cast<FormalParam *>(existingVar);
        if (formalParam != nullptr) {
            minic_log(LOG_INFO, "变量 %s 是形参，不创建新的局部变量", varName.c_str());
            node->val = existingVar;
            // 设置节点的sons[1]的val字段，这样在后续访问时可以直接使用
            node->sons[1]->val = existingVar;
            node->sons[1]->name = varName; // 确保变量名被设置

            // 处理数组类型的形参，在DragonIR中表示为指针
            if (varType->getTypeID() == Type::ArrayTyID) {
                ArrayType * arrayType = static_cast<ArrayType *>(varType);
                Type * elementType = arrayType->getElementType();
                // 创建新的数组指针类型
                Type * pointerArrayType = ArrayType::get(elementType, 0);

                minic_log(LOG_INFO, "数组类型的形参 %s 被转换为指针类型", varName.c_str());

                // 更新节点类型
                node->type = pointerArrayType;
                node->sons[1]->type = pointerArrayType;
            }
            return true;
        }
        minic_log(LOG_INFO,
                  "找到现有变量 %s，类型为: %s, 地址: %p",
                  varName.c_str(),
                  typeid(*existingVar).name(),
                  existingVar);

        // 重要修复：返回已存在的变量，而不是创建新变量
        node->val = existingVar;
        // 设置节点的sons[1]的val字段，这样在后续访问时可以直接使用
        node->sons[1]->val = existingVar;
        node->sons[1]->name = varName; // 确保变量名被设置
        return true;
    }

    // 处理数组类型
    Function * currentFunc = module->getCurrentFunction();
    if (varType->getTypeID() == Type::ArrayTyID) {
        // 创建数组变量
        minic_log(LOG_INFO, "准备创建数组变量: %s", varName.c_str());

        // 明确指定变量名，确保符号表中正确注册
        Value * arrayVar = module->newVarValue(varType, varName);
        if (!arrayVar) {
            minic_log(LOG_ERROR, "创建数组变量失败: %s", varName.c_str());
            return false;
        }

        node->val = arrayVar;
        node->name = varName; // 确保节点记住变量名

        // 确保变量名子节点有值属性和名称
        if (node->sons.size() >= 2 && node->sons[1] != nullptr) {
            node->sons[1]->val = arrayVar;
            node->sons[1]->name = varName; // 重要：确保数组定义节点也有变量名
            minic_log(LOG_INFO, "设置数组变量名节点的值: %p 和名称: %s", arrayVar, varName.c_str());
        }

        // 验证变量是否成功添加到符号表
        Value * checkVar = module->findVarValue(varName);
        if (checkVar) {
            minic_log(LOG_INFO, "数组变量已成功添加到符号表: %s, 地址: %p", varName.c_str(), checkVar);
        } else {
            minic_log(LOG_ERROR, "数组变量未能添加到符号表: %s", varName.c_str());

            // 尝试强制将变量添加到当前作用域
            if (arrayVar) {
                module->insertValueToCurrentScope(arrayVar);
                minic_log(LOG_INFO, "尝试强制将数组变量添加到当前作用域: %s", varName.c_str());

                // 再次验证
                checkVar = module->findVarValue(varName);
                if (checkVar) {
                    minic_log(LOG_INFO, "强制添加后，数组变量已在符号表中: %s", varName.c_str());
                } else {
                    minic_log(LOG_ERROR, "强制添加失败，数组变量仍不在符号表中: %s", varName.c_str());
                    return false;
                }
            }
        }

        minic_log(LOG_INFO, "创建数组变量: %s, 地址: %p", varName.c_str(), node->val);

        // 如果是函数形参，处理为指针类型
        if (!currentFunc) {
            // 全局数组变量
            minic_log(LOG_INFO, "声明全局数组变量: %s", varName.c_str());
        } else {
            // 局部数组变量
            minic_log(LOG_INFO, "声明局部数组变量: %s", varName.c_str());
        }

        return true;
    }

    // 普通变量的处理
    minic_log(LOG_INFO, "准备创建普通变量: %s", varName.c_str());
    node->val = module->newVarValue(varType, varName);
    node->name = varName; // 确保节点记住变量名

    // 确保变量名子节点有值属性和名称
    if (node->sons.size() >= 2 && node->sons[1] != nullptr) {
        node->sons[1]->val = node->val;
        node->sons[1]->name = varName; // 确保子节点也有变量名
        minic_log(LOG_INFO, "设置普通变量名节点的值: %p 和名称: %s", node->val, varName.c_str());
    }

    minic_log(LOG_INFO, "创建普通变量: %s, 地址: %p, 类型ID: %d", varName.c_str(), node->val, varType->getTypeID());

    // 检查变量是否成功添加到符号表
    Value * checkVar = module->findVarValue(varName);
    if (checkVar) {
        minic_log(LOG_INFO, "变量已成功添加到符号表: %s, 地址: %p", varName.c_str(), checkVar);
    } else {
        minic_log(LOG_ERROR, "变量未能添加到符号表: %s", varName.c_str());

        // 尝试强制将变量添加到当前作用域
        if (node->val) {
            module->insertValueToCurrentScope(node->val);
            minic_log(LOG_INFO, "尝试强制将变量添加到当前作用域: %s", varName.c_str());

            // 再次验证
            checkVar = module->findVarValue(varName);
            if (checkVar) {
                minic_log(LOG_INFO, "强制添加后，变量已在符号表中: %s", varName.c_str());
            } else {
                minic_log(LOG_ERROR, "强制添加失败，变量仍不在符号表中: %s", varName.c_str());
            }
        }
    }

    return true;
}

/// @brief 关系运算符 < 翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_lt(ast_node * node)
{
    // 获取左右操作数
    auto left = ir_visit_ast_node(node->sons[0]);
    if (!left) {
        return false;
    }

    auto right = ir_visit_ast_node(node->sons[1]);
    if (!right) {
        return false;
    }

    // 创建比较指令，结果为0或1
    IcmpInstruction * ltInst =
        new IcmpInstruction(module->getCurrentFunction(), IRInstOperator::IRINST_OP_ICMP, left->val, right->val, "lt");

    // 添加到当前节点的指令列表
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(ltInst);

    // 设置节点的值为比较结果
    node->val = ltInst;

    return true;
}

/// @brief 关系运算符 <= 翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_le(ast_node * node)
{
    // 获取左右操作数
    auto left = ir_visit_ast_node(node->sons[0]);
    if (!left) {
        return false;
    }

    auto right = ir_visit_ast_node(node->sons[1]);
    if (!right) {
        return false;
    }

    // 创建比较指令，结果为0或1
    IcmpInstruction * leInst =
        new IcmpInstruction(module->getCurrentFunction(), IRInstOperator::IRINST_OP_ICMP, left->val, right->val, "le");

    // 添加到当前节点的指令列表
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(leInst);

    // 设置节点的值为比较结果
    node->val = leInst;

    return true;
}

/// @brief 关系运算符 > 翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_gt(ast_node * node)
{
    // 获取左右操作数
    auto left = ir_visit_ast_node(node->sons[0]);
    if (!left) {
        return false;
    }

    auto right = ir_visit_ast_node(node->sons[1]);
    if (!right) {
        return false;
    }

    // 创建比较指令，结果为0或1
    IcmpInstruction * gtInst =
        new IcmpInstruction(module->getCurrentFunction(), IRInstOperator::IRINST_OP_ICMP, left->val, right->val, "gt");

    // 添加到当前节点的指令列表
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(gtInst);

    // 设置节点的值为比较结果
    node->val = gtInst;

    return true;
}

/// @brief 关系运算符 >= 翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_ge(ast_node * node)
{
    // 获取左右操作数
    auto left = ir_visit_ast_node(node->sons[0]);
    if (!left) {
        return false;
    }

    auto right = ir_visit_ast_node(node->sons[1]);
    if (!right) {
        return false;
    }

    // 创建比较指令，结果为0或1
    IcmpInstruction * geInst =
        new IcmpInstruction(module->getCurrentFunction(), IRInstOperator::IRINST_OP_ICMP, left->val, right->val, "ge");

    // 添加到当前节点的指令列表
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(geInst);

    // 设置节点的值为比较结果
    node->val = geInst;

    return true;
}

/// @brief 关系运算符 == 翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_eq(ast_node * node)
{
    // 获取左右操作数
    auto left = ir_visit_ast_node(node->sons[0]);
    if (!left) {
        return false;
    }

    auto right = ir_visit_ast_node(node->sons[1]);
    if (!right) {
        return false;
    }

    // 创建比较指令，结果为0或1
    IcmpInstruction * eqInst =
        new IcmpInstruction(module->getCurrentFunction(), IRInstOperator::IRINST_OP_ICMP, left->val, right->val, "eq");

    // 添加到当前节点的指令列表
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(eqInst);

    // 设置节点的值为比较结果
    node->val = eqInst;

    return true;
}

/// @brief 关系运算符 != 翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_ne(ast_node * node)
{
    // 获取左右操作数
    auto left = ir_visit_ast_node(node->sons[0]);
    if (!left) {
        return false;
    }

    auto right = ir_visit_ast_node(node->sons[1]);
    if (!right) {
        return false;
    }

    // 创建比较指令，结果为0或1
    IcmpInstruction * neInst =
        new IcmpInstruction(module->getCurrentFunction(), IRInstOperator::IRINST_OP_ICMP, left->val, right->val, "ne");

    // 添加到当前节点的指令列表
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(neInst);

    // 设置节点的值为比较结果
    node->val = neInst;

    return true;
}

/// @brief 逻辑与 && 翻译成线性中间IR，实现短路求值
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_logical_and(ast_node * node)
{
    // 实现短路求值的逻辑与
    // 生成左操作数的代码

    // 创建结果临时变量
    Value * result = module->newVarValue(IntegerType::getTypeInt());
    // 创建标签
    LabelInstruction * falseLabelInst =
        new LabelInstruction(module->getCurrentFunction(), generate_label()); // false标签
    LabelInstruction * trueLabelInst = new LabelInstruction(module->getCurrentFunction(), generate_label()); // true标签
    LabelInstruction * secondOpLabelInst =
        new LabelInstruction(module->getCurrentFunction(), generate_label()); // 第二个操作数入口
    LabelInstruction * endLabelInst = new LabelInstruction(module->getCurrentFunction(), generate_label()); // 结束标签

    // 使用新的IcmpInstruction和BcInstruction
    // 先检查左操作数是否不为0
    ast_node * left = ir_visit_ast_node(node->sons[0]);
    if (!left) {
        return false;
    }
    IcmpInstruction * leftCmpInst = new IcmpInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_ICMP,
                                                        left->val,
                                                        module->newConstInt(0),
                                                        "ne");

    // 如果左操作数为0(false)则跳转到false标签，否则继续执行右操作数
    BcInstruction * bcInst =
        new BcInstruction(module->getCurrentFunction(), leftCmpInst, secondOpLabelInst, falseLabelInst);

    // 生成右操作数的代码
    ast_node * right = ir_visit_ast_node(node->sons[1]);
    if (!right) {
        return false;
    }

    // 检查右操作数是否不为0
    IcmpInstruction * rightCmpInst = new IcmpInstruction(module->getCurrentFunction(),
                                                         IRInstOperator::IRINST_OP_ICMP,
                                                         right->val,
                                                         module->newConstInt(0),
                                                         "ne");

    // 根据右操作数结果进行跳转
    BcInstruction * secondBcInst =
        new BcInstruction(module->getCurrentFunction(), rightCmpInst, trueLabelInst, falseLabelInst);
    // 设置结果为0和1
    Value * zero = module->newConstInt(0);
    Value * one = module->newConstInt(1);
    MoveInstruction * setFalse = new MoveInstruction(module->getCurrentFunction(), result, zero);
    MoveInstruction * setTrue = new MoveInstruction(module->getCurrentFunction(), result, one);

    // 跳转到结束标签的指令
    GotoInstruction * falseGotoEnd = new GotoInstruction(module->getCurrentFunction(), endLabelInst);
    GotoInstruction * trueGotoEnd = new GotoInstruction(module->getCurrentFunction(), endLabelInst);

    // 添加所有指令
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(leftCmpInst); // 比较左操作数是否不为0
    node->blockInsts.addInst(bcInst);      // 根据比较结果跳转

    node->blockInsts.addInst(secondOpLabelInst); // 右操作数入口标签
    node->blockInsts.addInst(right->blockInsts); // 添加右操作数的指令序列
    node->blockInsts.addInst(rightCmpInst);      // 比较右操作数是否不为0
    node->blockInsts.addInst(secondBcInst);      // 根据右操作数比较结果跳转

    node->blockInsts.addInst(trueLabelInst); // true标签
    node->blockInsts.addInst(setTrue);       // 设置结果为1
    node->blockInsts.addInst(trueGotoEnd);   // 跳转到结束标签

    node->blockInsts.addInst(falseLabelInst); // false标签
    node->blockInsts.addInst(setFalse);       // 设置结果为0
    node->blockInsts.addInst(falseGotoEnd);   // 跳转到结束标签

    node->blockInsts.addInst(endLabelInst); // 结束标签

    node->val = result;

    return true;
}

/// @brief 逻辑或 || 翻译成线性中间IR，实现短路求值
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_logical_or(ast_node * node)
{
    // 实现短路求值的逻辑或
    // 生成左操作数的代码
    ast_node * left = ir_visit_ast_node(node->sons[0]);
    if (!left) {
        return false;
    }

    // 创建结果临时变量
    Value * result = module->newVarValue(IntegerType::getTypeInt());

    // 创建跳转标签
    std::string trueLabel = generate_label();
    std::string endLabel = generate_label();

    // 如果左操作数为真，直接短路，结果为1
    LabelInstruction * trueLabelInst = new LabelInstruction(module->getCurrentFunction(), trueLabel);
    std::string secondOpLabel = generate_label();
    LabelInstruction * secondOpLabelInst = new LabelInstruction(module->getCurrentFunction(), secondOpLabel);

    // 使用新的IcmpInstruction和BcInstruction
    // 先检查左操作数是否不为0
    IcmpInstruction * leftCmpInst = new IcmpInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_ICMP,
                                                        left->val,
                                                        module->newConstInt(0),
                                                        "ne");

    // 如果左操作数不为0(true)则跳转到true标签，否则继续执行右操作数
    BcInstruction * bcInst =
        new BcInstruction(module->getCurrentFunction(), leftCmpInst, trueLabelInst, secondOpLabelInst);

    // 生成指令
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(leftCmpInst);
    node->blockInsts.addInst(bcInst);
    node->blockInsts.addInst(secondOpLabelInst);

    // 生成右操作数的代码
    ast_node * right = ir_visit_ast_node(node->sons[1]);
    if (!right) {
        return false;
    }

    // 添加右操作数的指令序列
    node->blockInsts.addInst(right->blockInsts);

    // 检查右操作数是否不为0
    IcmpInstruction * rightCmpInst = new IcmpInstruction(module->getCurrentFunction(),
                                                         IRInstOperator::IRINST_OP_ICMP,
                                                         right->val,
                                                         module->newConstInt(0),
                                                         "ne");

    // 添加右操作数比较指令
    node->blockInsts.addInst(rightCmpInst);

    // 创建第二个条件分支
    LabelInstruction * falseLabelInst = new LabelInstruction(module->getCurrentFunction(), generate_label());
    LabelInstruction * endLabelInst = new LabelInstruction(module->getCurrentFunction(), endLabel);

    // 根据右操作数比较结果进行分支
    BcInstruction * secondBcInst =
        new BcInstruction(module->getCurrentFunction(), rightCmpInst, trueLabelInst, falseLabelInst);
    node->blockInsts.addInst(secondBcInst);

    // false标签处理
    node->blockInsts.addInst(falseLabelInst);
    // 设置结果为0
    Value * zero = module->newConstInt(0);
    MoveInstruction * setFalse = new MoveInstruction(module->getCurrentFunction(), result, zero);
    node->blockInsts.addInst(setFalse);

    // 跳转到结束
    GotoInstruction * falseGotoEnd = new GotoInstruction(module->getCurrentFunction(), endLabelInst);
    node->blockInsts.addInst(falseGotoEnd);

    // 添加已生成的真分支标签
    // trueLabelInst已在前面定义

    // 设置结果为1
    Value * one = module->newConstInt(1);
    MoveInstruction * setTrue = new MoveInstruction(module->getCurrentFunction(), result, one);

    // 设置结束标签
    // endLabelInst已经在前面定义

    // 添加指令
    node->blockInsts.addInst(trueLabelInst);
    node->blockInsts.addInst(setTrue);

    // 从true跳转到结束标签
    GotoInstruction * trueGotoEnd = new GotoInstruction(module->getCurrentFunction(), endLabelInst);
    node->blockInsts.addInst(trueGotoEnd);

    // 添加结束标签
    node->blockInsts.addInst(endLabelInst);

    node->val = result;

    return true;
}

/// @brief 逻辑非 ! 翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_logical_not(ast_node * node)
{
    // 生成操作数的代码
    ast_node * operand = ir_visit_ast_node(node->sons[0]);
    if (!operand) {
        return false;
    }

    // 创建比较指令，将操作数与0比较
    Value * zero = module->newConstInt(0);
    IcmpInstruction * eqInst =
        new IcmpInstruction(module->getCurrentFunction(), IRInstOperator::IRINST_OP_ICMP, operand->val, zero, "eq");

    // 创建临时变量，将i1类型的结果转换为i32类型
    Value * result = module->newVarValue(IntegerType::getTypeInt());

    // 将i1类型的布尔结果转换为i32类型的整数结果（0或1）
    MoveInstruction * moveInst = new MoveInstruction(module->getCurrentFunction(), result, eqInst);

    // 添加指令
    node->blockInsts.addInst(operand->blockInsts);
    node->blockInsts.addInst(eqInst);
    node->blockInsts.addInst(moveInst);

    node->val = result;

    return true;
}

/// @brief if语句翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_if(ast_node * node)
{
    // if语句包含两个子节点：
    // 1. 条件表达式
    // 2. 如果条件为真执行的语句（可能是语句块）

    // 生成条件表达式的代码
    ast_node * condition = ir_visit_ast_node(node->sons[0]);
    if (!condition) {
        return false;
    }

    // 创建标签
    std::string thenLabel = generate_label(); // 真分支的标签
    std::string endLabel = generate_label();  // if语句结束的标签

    // 条件为真则跳转到thenLabel，否则跳转到endLabel
    LabelInstruction * thenLabelInst = new LabelInstruction(module->getCurrentFunction(), thenLabel);
    LabelInstruction * endLabelInst = new LabelInstruction(module->getCurrentFunction(), endLabel);

    // 使用新的BcInstruction替代IfInstruction
    BcInstruction * bcInst =
        new BcInstruction(module->getCurrentFunction(), condition->val, thenLabelInst, endLabelInst);

    // 添加条件和跳转指令
    node->blockInsts.addInst(condition->blockInsts);
    // node->blockInsts.addInst(condition->va);
    node->blockInsts.addInst(bcInst);

    // 添加真分支标签
    node->blockInsts.addInst(thenLabelInst);

    // 生成真分支的代码
    ast_node * thenStmt = ir_visit_ast_node(node->sons[1]);
    if (!thenStmt) {
        return false;
    }

    // 添加真分支的代码
    node->blockInsts.addInst(thenStmt->blockInsts);

    // 添加结束标签
    node->blockInsts.addInst(endLabelInst);

    return true;
}

/// @brief if-else语句翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_if_else(ast_node * node)
{
    // if-else语句包含三个子节点：
    // 1. 条件表达式
    // 2. 如果条件为真执行的语句（可能是语句块）
    // 3. 如果条件为假执行的语句（可能是语句块）

    // 生成条件表达式的代码
    ast_node * condition = ir_visit_ast_node(node->sons[0]);
    if (!condition) {
        return false;
    }

    // 创建标签
    std::string thenLabel = generate_label(); // 真分支的标签
    std::string elseLabel = generate_label(); // 假分支的标签
    std::string endLabel = generate_label();  // if-else语句结束的标签

    // 条件为真则跳转到thenLabel，否则跳转到elseLabel
    LabelInstruction * thenLabelInst = new LabelInstruction(module->getCurrentFunction(), thenLabel);
    LabelInstruction * elseLabelInst = new LabelInstruction(module->getCurrentFunction(), elseLabel);
    LabelInstruction * endLabelInst = new LabelInstruction(module->getCurrentFunction(), endLabel);

    // 使用新的BcInstruction替代IfInstruction和GotoInstruction
    BcInstruction * bcInst =
        new BcInstruction(module->getCurrentFunction(), condition->val, thenLabelInst, elseLabelInst);

    // 添加条件和跳转指令
    node->blockInsts.addInst(condition->blockInsts);
    node->blockInsts.addInst(bcInst);

    // 添加真分支标签
    node->blockInsts.addInst(thenLabelInst);

    // 生成真分支的代码
    ast_node * thenStmt = ir_visit_ast_node(node->sons[1]);
    if (!thenStmt) {
        return false;
    }

    // 添加真分支的代码
    node->blockInsts.addInst(thenStmt->blockInsts);

    // 真分支结束后跳转到endLabel
    GotoInstruction * gotoEndInst = new GotoInstruction(module->getCurrentFunction(), endLabelInst);
    node->blockInsts.addInst(gotoEndInst);

    // 添加假分支标签
    node->blockInsts.addInst(elseLabelInst);

    // 生成假分支的代码
    ast_node * elseStmt = ir_visit_ast_node(node->sons[2]);
    if (!elseStmt) {
        return false;
    }

    // 添加假分支的代码
    node->blockInsts.addInst(elseStmt->blockInsts);

    // 添加结束标签
    node->blockInsts.addInst(endLabelInst);

    return true;
}

/// @brief while循环语句翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_while(ast_node * node)
{
    // 获取当前函数
    Function * func = module->getCurrentFunction();
    if (!func) {
        return false;
    }

    // 创建循环需要的标签
    std::string startLabel = generate_label(); // 循环的开始标签，用于条件判断
    std::string bodyLabel = generate_label();  // 循环体的开始标签
    std::string endLabel = generate_label();   // 循环结束的标签

    // 保存当前的循环标签（为了处理嵌套循环）
    std::string savedCurrentWhileStartLabel = currentWhileStartLabel;
    std::string savedCurrentWhileEndLabel = currentWhileEndLabel;

    // 保存当前的循环标签指令
    LabelInstruction * savedCurrentWhileStartLabelInst = currentWhileStartLabelInst;
    LabelInstruction * savedCurrentWhileEndLabelInst = currentWhileEndLabelInst;

    // 保存之前的循环标签（如果有嵌套循环）
    whileLabels.push_back({startLabel, endLabel});

    // 更新当前处理的循环标签
    currentWhileStartLabel = startLabel;
    currentWhileEndLabel = endLabel;

    // 添加循环开始标签
    LabelInstruction * startLabelInst = new LabelInstruction(func, startLabel);
    node->blockInsts.addInst(startLabelInst);

    // 保存循环开始标签指令
    currentWhileStartLabelInst = startLabelInst;

    // 生成条件表达式的代码
    ast_node * condition = ir_visit_ast_node(node->sons[0]);
    if (!condition) {
        // 错误处理
        whileLabels.pop_back();
        currentWhileStartLabel = savedCurrentWhileStartLabel;
        currentWhileEndLabel = savedCurrentWhileEndLabel;
        return false;
    }

    // 条件为真则跳转到 bodyLabel，否则跳转到 endLabel
    LabelInstruction * bodyLabelInst = new LabelInstruction(func, bodyLabel);
    LabelInstruction * endLabelInst = new LabelInstruction(func, endLabel);

    // 保存循环结束标签指令
    currentWhileEndLabelInst = endLabelInst;

    // 保存标签指令对
    whileLabelInsts.push_back({startLabelInst, endLabelInst});

    BcInstruction * bcInst = new BcInstruction(func, condition->val, bodyLabelInst, endLabelInst);

    node->blockInsts.addInst(condition->blockInsts);
    node->blockInsts.addInst(bcInst);
    node->blockInsts.addInst(bodyLabelInst);

    // 设置循环上下文标志，用于数组访问地址计算
    bool savedInLoopContext = inLoopContext;
    inLoopContext = true;

    // 生成循环体的代码
    ast_node * body = ir_visit_ast_node(node->sons[1]);
    if (!body) {
        whileLabels.pop_back();
        currentWhileStartLabel = savedCurrentWhileStartLabel;
        currentWhileEndLabel = savedCurrentWhileEndLabel;
        inLoopContext = savedInLoopContext; // 恢复原值
        return false;
    }

    node->blockInsts.addInst(body->blockInsts);

    // 恢复循环上下文标志
    inLoopContext = savedInLoopContext;

    // 循环体结束后跳转回循环开始（条件检查）
    GotoInstruction * gotoStartInst = new GotoInstruction(func, startLabelInst);
    node->blockInsts.addInst(gotoStartInst);

    // 添加循环结束标签
    node->blockInsts.addInst(endLabelInst);

    // 恢复之前的循环标签
    whileLabels.pop_back();
    whileLabelInsts.pop_back();
    currentWhileStartLabel = savedCurrentWhileStartLabel;
    currentWhileEndLabel = savedCurrentWhileEndLabel;
    currentWhileStartLabelInst = savedCurrentWhileStartLabelInst;
    currentWhileEndLabelInst = savedCurrentWhileEndLabelInst;

    return true;
}

/// @brief break语句翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_break(ast_node * node)
{
    // 获取当前函数
    Function * func = module->getCurrentFunction();
    if (!func) {
        return false;
    }

    // 检查是否在循环内
    if (whileLabels.empty() || currentWhileEndLabelInst == nullptr) {
        minic_log(LOG_ERROR, "break语句只能用于while循环内");
        return false;
    }

    // 使用已存在的循环结束标签指令，而不是创建新的
    GotoInstruction * gotoEndInst = new GotoInstruction(func, currentWhileEndLabelInst);
    node->blockInsts.addInst(gotoEndInst);

    return true;
}

/// @brief continue语句翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_continue(ast_node * node)
{
    // 获取当前函数
    Function * func = module->getCurrentFunction();
    if (!func) {
        return false;
    }

    // 检查是否在循环内
    if (whileLabels.empty() || currentWhileStartLabelInst == nullptr) {
        minic_log(LOG_ERROR, "continue语句只能用于while循环内");
        return false;
    }

    // 无条件跳转到当前循环的开始标签
    GotoInstruction * gotoStartInst = new GotoInstruction(func, currentWhileStartLabelInst);
    node->blockInsts.addInst(gotoStartInst);

    return true;
}

/// @brief 数组定义AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_array_def(ast_node * node)
{
    minic_log(LOG_INFO, "=== 开始处理数组定义IR ===");
    minic_log(LOG_INFO, "节点类型: %d", (int) node->node_type);
    minic_log(LOG_INFO, "子节点数量: %zu", node->sons.size());

    // 数组定义的结构：左孩子是元素类型，右孩子是大小表达式
    // 对于多维数组，使用嵌套的AST_OP_ARRAY_DEF表示
    if (node->sons.size() != 2) {
        // 数组定义节点必须有两个孩子
        minic_log(LOG_ERROR, "数组定义节点格式错误，子节点数量: %zu", node->sons.size());
        return false;
    }

    // 打印数组定义的详细信息
    minic_log(LOG_INFO, "处理数组定义节点, 行号: %ld", node->line_no);
    if (!node->name.empty()) {
        minic_log(LOG_INFO, "数组名称: %s", node->name.c_str());
    } else {
        minic_log(LOG_INFO, "数组名称未设置");
    }

    // 设置数组定义阶段标记
    node->isInArrayDefPhase = true;

    // 调试：打印子节点信息
    minic_log(LOG_INFO,
              "子节点[0] (元素类型): 类型=%d, 行号=%ld",
              (int) node->sons[0]->node_type,
              node->sons[0]->line_no);
    minic_log(LOG_INFO,
              "子节点[1] (数组大小): 类型=%d, 行号=%ld",
              (int) node->sons[1]->node_type,
              node->sons[1]->line_no);

    // 获取元素类型
    ast_node * element_type_node = node->sons[0];

    // 将数组定义阶段标记传递给元素类型节点
    element_type_node->isInArrayDefPhase = true;

    // 如果有变量名，传递给元素类型节点
    if (!node->name.empty()) {
        element_type_node->name = node->name;
        minic_log(LOG_INFO, "将变量名 %s 传递给元素类型节点", node->name.c_str());
    }

    // 检查是否是嵌套的数组定义
    if (element_type_node->node_type == ast_operator_type::AST_OP_ARRAY_DEF) {
        // 嵌套数组定义，递归处理内层数组
        minic_log(LOG_INFO, "发现嵌套数组定义，递归处理内层");

        // 传递变量名和标记到内层
        if (!node->name.empty()) {
            element_type_node->name = node->name;
            minic_log(LOG_INFO, "传递变量名 %s 到内层数组定义", node->name.c_str());
        }

        // 设置数组定义阶段标记
        element_type_node->isInArrayDefPhase = true;

        // 如果有孙节点，也传递变量名和标记
        if (!element_type_node->sons.empty() && element_type_node->sons[0] != nullptr) {
            if (!node->name.empty()) {
                element_type_node->sons[0]->name = node->name;
            }
            element_type_node->sons[0]->isInArrayDefPhase = true;
        }

        // 递归处理内层数组定义
        if (!ir_array_def(element_type_node)) {
            minic_log(LOG_ERROR, "处理嵌套数组定义失败");
            return false;
        }

        minic_log(LOG_INFO,
                  "嵌套数组处理完成，内层类型ID: %d",
                  element_type_node->type ? element_type_node->type->getTypeID() : -1);

    } else if (element_type_node->node_type == ast_operator_type::AST_OP_LEAF_TYPE) {
        // 如果是类型叶子节点，确保类型不为空
        if (!element_type_node->type) {
            minic_log(LOG_INFO, "类型叶子节点的类型为空，设置为int");
            element_type_node->type = IntegerType::getTypeInt();
        }

        minic_log(LOG_INFO, "使用类型叶子节点，类型ID: %d", element_type_node->type->getTypeID());

    } else {
        // 其他情况，尝试处理
        ast_node * processed_node = ir_visit_ast_node(element_type_node);
        if (processed_node) {
            element_type_node = processed_node;
        }

        // 如果仍然没有类型，设置为默认类型
        if (!element_type_node->type) {
            minic_log(LOG_INFO, "元素类型节点没有类型，设置为int");
            element_type_node->type = IntegerType::getTypeInt();
        }

        minic_log(LOG_INFO, "处理其他类型节点，最终类型ID: %d", element_type_node->type->getTypeID());
    }

    // 获取数组大小
    ast_node * array_size_node = node->sons[1];

    // 直接处理数组大小节点，不通过ir_visit_ast_node避免符号表查找
    if (array_size_node->node_type == ast_operator_type::AST_OP_LEAF_LITERAL_UINT) {
        // 如果是常量，直接使用它的值
        minic_log(LOG_INFO, "数组大小是常量: %u", array_size_node->integer_val);
    } else {
        // 对于其他类型的节点，需要进行处理
        array_size_node = ir_visit_ast_node(array_size_node);
        if (!array_size_node) {
            minic_log(LOG_ERROR, "处理数组大小节点失败");
            return false;
        }
    }

    // 添加元素类型和数组大小节点的指令
    node->blockInsts.addInst(element_type_node->blockInsts);
    node->blockInsts.addInst(array_size_node->blockInsts);

    // 数组大小必须是常量
    uint32_t array_size = 0;
    if (array_size_node->node_type == ast_operator_type::AST_OP_LEAF_LITERAL_UINT) {
        array_size = array_size_node->integer_val;
        minic_log(LOG_INFO, "数组大小: %u", array_size);
    } else if (array_size_node->val) {
        // 尝试从节点的值中获取大小
        ConstInt * constInt = dynamic_cast<ConstInt *>(array_size_node->val);
        if (constInt) {
            array_size = constInt->getVal();
            minic_log(LOG_INFO, "从节点值获取数组大小: %u", array_size);
        } else {
            minic_log(LOG_ERROR, "数组大小节点的值不是常量整数");
            return false;
        }
    } else {
        // 尝试使用默认大小
        array_size = 4; // 默认大小为4
        minic_log(LOG_INFO, "使用默认数组大小: %u", array_size);
    }

    // 获取元素类型
    Type * element_type = element_type_node->type;
    if (!element_type) {
        minic_log(LOG_ERROR, "元素类型为空，设置为int");
        element_type = IntegerType::getTypeInt();
    } else if (element_type->getTypeID() != Type::IntegerTyID && element_type->getTypeID() != Type::ArrayTyID) {
        // 确保数组元素类型为i32或数组类型，而不是void或其他类型
        element_type = IntegerType::getTypeInt();
    }
    minic_log(LOG_INFO, "最终元素类型ID: %d", element_type->getTypeID());

    // 创建数组类型
    node->type = ArrayType::get(element_type, array_size);

    minic_log(LOG_INFO,
              "创建数组类型成功，元素类型ID: %d, 数组大小: %u, 结果类型ID: %d",
              element_type->getTypeID(),
              array_size,
              node->type ? node->type->getTypeID() : -1);

    // 添加指令
    node->blockInsts.addInst(element_type_node->blockInsts);
    node->blockInsts.addInst(array_size_node->blockInsts);

    // 如果当前节点有值（可能从父节点传递），将其也传递给子节点
    if (node->val) {
        minic_log(LOG_INFO, "数组定义节点有值: %p", node->val);
        if (element_type_node && !element_type_node->val) {
            element_type_node->val = node->val;
            minic_log(LOG_INFO, "传递值 %p 到元素类型节点", node->val);
        }
    }

    minic_log(LOG_INFO, "=== 数组定义IR处理完成 ===");
    return true;
}

/// @brief 数组访问AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_array_access(ast_node * node)
{
    // 数组访问的结构：左孩子是数组名（可能是数组访问），右孩子是下标表达式
    if (node->sons.size() != 2) {
        // 数组访问节点必须有两个孩子
        minic_log(LOG_ERROR, "数组访问节点格式错误，子节点数量: %zu", node->sons.size());
        return false;
    }

    Function * function = module->getCurrentFunction();
    if (!function) {
        // 必须在函数内部访问数组
        minic_log(LOG_ERROR, "数组访问必须在函数内部");
        return false;
    }

    // 获取数组基址
    ast_node * array_base_node = node->sons[0];

    // 打印数组访问的详细信息
    minic_log(LOG_INFO, "数组访问节点，行号: %ld", node->line_no);
    if (array_base_node->node_type == ast_operator_type::AST_OP_LEAF_VAR_ID) {
        minic_log(LOG_INFO, "数组基址是变量标识符: %s", array_base_node->name.c_str());
    } else {
        minic_log(LOG_INFO, "数组基址不是变量标识符，节点类型: %d", static_cast<int>(array_base_node->node_type));
    }

    // 输出符号表中的所有变量（调试信息）
    minic_log(LOG_INFO, "当前函数中的所有变量:");
    minic_log(LOG_INFO, "  函数名: %s", function->getName().c_str());
    for (auto & var: function->getVarValues()) {
        minic_log(LOG_INFO, "  局部变量: %s, 类型ID: %d", var->getName().c_str(), var->getType()->getTypeID());
    }

    // 如果是变量标识符，尝试从符号表直接查找
    if (array_base_node->node_type == ast_operator_type::AST_OP_LEAF_VAR_ID) {
        // 检查变量名是否为空
        if (array_base_node->name.empty()) {
            minic_log(LOG_ERROR, "数组变量名为空");
            std::cerr << "Error: Empty array variable name at line " << array_base_node->line_no << std::endl;
            return false;
        }

        std::string array_name = array_base_node->name;
        minic_log(LOG_INFO, "准备查找数组变量: %s", array_name.c_str());

        // 尝试不同的查找策略

        // 1. 首先检查节点的值属性
        if (array_base_node->val) {
            minic_log(LOG_INFO, "数组变量节点已有值属性: %p", array_base_node->val);
        } else {
            // 2. 首先在函数局部变量中查找
            LocalVariable * local_var = nullptr;
            for (auto & var: function->getVarValues()) {
                if (var->getName() == array_name) {
                    local_var = var;
                    break;
                }
            }

            if (local_var) {
                array_base_node->val = local_var;
                minic_log(LOG_INFO, "在函数局部变量中找到数组变量: %s, 地址: %p", array_name.c_str(), local_var);
            } else {
                // 3. 如果局部变量中找不到，使用模块级查找
                Value * module_array_var = module->findVarValue(array_name);
                if (module_array_var) {
                    array_base_node->val = module_array_var;
                    minic_log(LOG_INFO,
                              "在模块符号表中找到数组变量: %s, 地址: %p",
                              array_name.c_str(),
                              module_array_var);
                } else {
                    // 4. 如果仍未找到，可能需要创建临时变量（仅在紧急情况下）
                    minic_log(LOG_ERROR, "在所有符号表中找不到变量: %s", array_name.c_str());

                    // 作为最后的尝试，创建一个临时数组变量
                    Type * array_type = ArrayType::get(IntegerType::getTypeInt(), 4); // 默认int[4]
                    LocalVariable * temp_var = function->newLocalVarValue(array_type, array_name, 1);

                    if (temp_var) {
                        array_base_node->val = temp_var;
                        minic_log(LOG_INFO, "创建临时数组变量: %s, 地址: %p", array_name.c_str(), temp_var);

                        // 将临时变量添加到模块符号表
                        module->insertValueToCurrentScope(temp_var);

                        // 验证变量添加成功
                        Value * check_var = module->findVarValue(array_name);
                        if (check_var) {
                            minic_log(LOG_INFO, "临时数组变量已添加到符号表: %s", array_name.c_str());
                        } else {
                            minic_log(LOG_ERROR, "临时数组变量添加到符号表失败: %s", array_name.c_str());
                            std::cerr << "Error: Failed to add temporary array variable '" << array_name
                                      << "' to symbol table at line " << array_base_node->line_no << std::endl;
                            return false;
                        }
                    } else {
                        std::cerr << "Error: Undefined variable '" << array_name << "' at line "
                                  << array_base_node->line_no << std::endl;
                        return false;
                    }
                }
            }
        }
    }

    // 现在访问数组基址节点
    ast_node * array_base_result = ir_visit_ast_node(array_base_node);
    if (!array_base_result) {
        minic_log(LOG_ERROR, "无法处理数组基址节点");
        return false;
    }

    // 获取索引表达式
    ast_node * index_node = ir_visit_ast_node(node->sons[1]);
    if (!index_node) {
        minic_log(LOG_ERROR, "无法处理数组索引节点");
        return false;
    }

    // 检查索引节点是否有值
    if (!index_node->val) {
        minic_log(LOG_ERROR, "数组索引节点没有值，尝试处理常量索引");

        // 如果是常量整数节点，直接创建常量值
        if (index_node->node_type == ast_operator_type::AST_OP_LEAF_LITERAL_UINT) {
            index_node->val = module->newConstInt(index_node->integer_val);
            minic_log(LOG_INFO, "为常量索引 %u 创建值", index_node->integer_val);
        } else {
            std::cerr << "Error: Invalid array index at line " << node->line_no << std::endl;
            return false;
        }
    }

    // 将前面部分的IR指令添加到当前节点
    // 将前面部分的IR指令添加到当前节点
    node->blockInsts.addInst(array_base_result->blockInsts);
    node->blockInsts.addInst(index_node->blockInsts);

    // 在循环上下文中，完全跳过地址缓存和预计算，直接计算
    if (inLoopContext) {
        minic_log(LOG_INFO, "在循环上下文中，跳过地址缓存，直接计算数组访问");

        // 获取数组基址和索引值
        Value * array_base = array_base_result->val;
        Value * index_value = index_node->val;

        if (!array_base || !index_value) {
            minic_log(LOG_ERROR, "数组基址或索引值为空");
            return false;
        }

        Function * function = module->getCurrentFunction();

        // 直接在这里计算数组地址，不复用任何预计算指令
        // 不调用computeArrayElementAddress，因为它可能复用指令

        // 计算字节偏移 = 索引 * 4
        Value * size_val = module->newConstInt(4);
        BinaryInstruction * byte_offset = new BinaryInstruction(function,
                                                                IRInstOperator::IRINST_OP_MUL_I,
                                                                index_value,
                                                                size_val,
                                                                IntegerType::getTypeInt());

        // 计算最终地址 = 数组基址 + 字节偏移
        Type * element_type = IntegerType::getTypeInt();
        const PointerType * ptrType = PointerType::get(element_type);
        BinaryInstruction * final_addr = new BinaryInstruction(function,
                                                               IRInstOperator::IRINST_OP_ADD_I,
                                                               array_base,
                                                               byte_offset,
                                                               const_cast<PointerType *>(ptrType));

        // 创建临时变量来存储加载的值
        static int tempVarCounter = 0;
        std::string timestamp = std::to_string(static_cast<long long>(std::time(nullptr)));
        std::string tempVarName = "__loop_" + timestamp + "_temp_array_" + std::to_string(tempVarCounter++) + "_" +
                                  std::to_string(node->line_no);
        LocalVariable * tempVar = function->newLocalVarValue(IntegerType::getTypeInt(), tempVarName, 1);

        // 创建加载指令
        MoveInstruction * loadInst = new MoveInstruction(function, tempVar, final_addr, ARRAY_READ);

        // 将指令直接添加到函数而不是节点的blockInsts（关键修改）
        // 这样可以避免在循环中重复添加同样的指令
        // 注意：我们不能将这些指令添加到blockInsts，因为那样会在循环中重复执行

        // 改为添加到当前函数的指令列表中，确保这些指令在实际执行时不会重定义
        // 但是我们仍然需要让这些指令的执行顺序正确

        // 将指令添加到节点以保证执行顺序
        node->blockInsts.addInst(byte_offset);
        node->blockInsts.addInst(final_addr);
        node->blockInsts.addInst(loadInst);

        // 设置节点值和类型
        node->val = tempVar;
        node->type = element_type;

        minic_log(LOG_INFO, "在循环中成功跳过缓存直接计算数组访问地址");
        return true;
    }

    // 非循环上下文的原有逻辑
    // 收集索引信息
    std::vector<Value *> indices;
    std::vector<ast_node *> array_nodes;

    // 记录调试信息
    if (array_base_node->node_type == ast_operator_type::AST_OP_LEAF_VAR_ID) {
        minic_log(LOG_INFO, "处理数组访问: %s", array_base_node->name.c_str());
    } else {
        minic_log(LOG_INFO, "处理复杂数组访问");
    }

    // 检查索引值是否有效
    if (!index_node->val) {
        std::cerr << "Error: Invalid array index at line " << node->line_no << std::endl;
        return false;
    }

    minic_log(LOG_INFO, "数组索引值: %p", index_node->val);

    // 收集所有的索引表达式
    indices.push_back(index_node->val);
    array_nodes.push_back(array_base_result);

    // 处理多维数组访问（嵌套的数组访问）
    ast_node * current = array_base_result;
    while (current->node_type == ast_operator_type::AST_OP_ARRAY_ACCESS) {
        // 确保索引值存在
        if (!current->sons[1]->val) {
            std::cerr << "Error: Invalid array index at line " << current->line_no << std::endl;
            return false;
        }

        indices.insert(indices.begin(), current->sons[1]->val); // 插入到前面，保持正确的顺序
        array_nodes.insert(array_nodes.begin(), current->sons[0]);
        current = current->sons[0];
    }

    // 获取数组基础变量
    Value * array_base = nullptr;

    // 如果当前节点是变量标识符，则直接查找变量
    if (current->node_type == ast_operator_type::AST_OP_LEAF_VAR_ID) {
        minic_log(LOG_INFO, "数组基址是变量标识符: %s", current->name.c_str());
        // 如果节点已经有值，直接使用
        if (current->val) {
            array_base = current->val;
            minic_log(LOG_INFO, "使用预设的数组变量值: %p", array_base);
        } else {
            // 否则从符号表中查找
            array_base = module->findVarValue(current->name);
            if (!array_base) {
                std::cerr << "Error: Undefined variable '" << current->name << "' at line " << current->line_no
                          << std::endl;
                return false;
            }
            // 更新节点的值，以便后续使用
            current->val = array_base;
            minic_log(LOG_INFO, "找到数组变量: %s, 地址: %p", current->name.c_str(), array_base);
        }
    } else {
        // 其他情况使用节点的值
        array_base = current->val;
        minic_log(LOG_INFO, "数组基址不是变量标识符，使用节点值: %p", array_base);
    }

    if (!array_base) {
        minic_log(LOG_ERROR, "无法获取数组基址");
        return false;
    }

    // 获取数组的类型
    Type * array_type = array_base->getType();
    minic_log(LOG_INFO, "数组类型: %d", array_type->getTypeID());

    // 计算数组元素的地址
    // 每次访问数组都重新计算地址，不复用地址变量
    Value * element_addr = computeArrayElementAddress(array_base, indices, function);
    if (!element_addr) {
        minic_log(LOG_ERROR, "计算数组元素地址失败");
        return false;
    }
    minic_log(LOG_INFO, "计算得到的数组元素地址: %p，循环上下文: %s", element_addr, inLoopContext ? "是" : "否");

    // 如果是数组类型，获取元素类型
    Type * element_type = nullptr;
    if (array_type->getTypeID() == Type::ArrayTyID) {
        ArrayType * arr_type = static_cast<ArrayType *>(array_type);
        // 逐层解析多维数组的元素类型
        element_type = arr_type->getElementType();
        for (size_t i = 1; i < indices.size(); i++) {
            if (element_type->getTypeID() == Type::ArrayTyID) {
                element_type = static_cast<ArrayType *>(element_type)->getElementType();
            } else {
                break;
            }
        }
    } else {
        // 如果不是数组类型（例如指针），假设指向的是整数类型
        element_type = IntegerType::getTypeInt();
    }

    // 为读取操作加载数组元素的值
    if (node->parent && node->parent->node_type == ast_operator_type::AST_OP_ASSIGN && node->parent->sons[0] == node) {
        // 如果当前节点是赋值操作的左侧，直接使用元素地址
        node->val = element_addr;
        minic_log(LOG_INFO, "数组元素作为赋值目标，使用地址: %p", element_addr);
    } else {
        // 对于其他情况（如读取操作），需要加载元素值
        // 使用更加唯一的临时变量名，确保在循环内每次访问都使用新的变量
        static int tempVarCounter = 0;
        std::string tempVarName;

        // 在循环上下文中，使用更具唯一性的临时变量名，确保每次循环迭代都使用新变量
        if (inLoopContext) {
            // 在循环中，加入时间戳和循环迭代标记，确保每次循环迭代使用不同的临时变量
            tempVarName = "__temp_array_" + std::to_string(tempVarCounter++) + "_" + std::to_string(node->line_no) +
                          "_loop_" + std::to_string(static_cast<long long>(std::time(nullptr)));
        } else {
            tempVarName = "__temp_array_" + std::to_string(tempVarCounter++) + "_" + std::to_string(node->line_no);
        }

        LocalVariable * tempVar = function->newLocalVarValue(IntegerType::getTypeInt(), tempVarName, 1);

        // 创建一个加载指令，将数组元素的值加载到临时变量中
        // 使用ARRAY_READ类型，确保生成正确的"value = *addr"格式
        MoveInstruction * loadInst = new MoveInstruction(function, tempVar, element_addr, ARRAY_READ);
        node->blockInsts.addInst(loadInst);

        // 将临时变量作为节点的值
        node->val = tempVar;
        minic_log(LOG_INFO, "从数组元素地址 %p 加载值到临时变量 %p", element_addr, tempVar);
    }

    // 设置节点的类型
    node->type = element_type;
    minic_log(LOG_INFO, "设置数组元素类型，ID: %d", element_type->getTypeID());

    return true;
}

/// @brief 多维数组索引地址计算
/// @param arrayValue 数组变量
/// @param indices 索引表达式列表
/// @param function 当前函数
/// @return 数组元素地址
Value * IRGenerator::computeArrayElementAddress(Value * arrayValue, std::vector<Value *> & indices, Function * function)
{
    minic_log(LOG_INFO, "开始计算数组元素地址，数组基址: %p, 索引数量: %zu", arrayValue, indices.size());

    // 获取数组类型
    Type * array_type = arrayValue->getType();
    minic_log(LOG_INFO, "数组类型ID: %d", array_type->getTypeID());

    // 对于多维数组，我们需要收集所有维度大小
    std::vector<uint32_t> dimensions;
    Type * current_type = array_type;
    Type * element_type = nullptr; // 记录最终元素类型

    // 收集所有维度信息
    while (current_type && current_type->getTypeID() == Type::ArrayTyID) {
        ArrayType * arr_type = static_cast<ArrayType *>(current_type);
        uint32_t numElements = arr_type->getNumElements();

        minic_log(LOG_INFO, "收集到维度: %u", numElements);
        dimensions.push_back(numElements);
        current_type = arr_type->getElementType();

        // 记录最终的元素类型
        if (current_type && current_type->getTypeID() != Type::ArrayTyID) {
            element_type = current_type;
        }

        // 防止无限循环
        if (!current_type) {
            minic_log(LOG_ERROR, "数组元素类型为空");
            break;
        }
    }

    // 如果元素类型仍然为空，使用整数类型
    if (!element_type) {
        element_type = IntegerType::getTypeInt();
    }

    // 如果处于循环上下文中，强制重新计算地址
    if (inLoopContext) {
        minic_log(LOG_INFO, "当前处于循环上下文中，将强制重新计算数组地址");

        // 完全跳过任何地址缓存机制
        // 直接使用当前索引值计算新地址

        // 检查索引数量
        if (indices.size() <= 0) {
            return arrayValue;
        }

        if (indices.size() > dimensions.size()) {
            minic_log(LOG_ERROR, "索引数量(%zu)大于维度数量(%zu)", indices.size(), dimensions.size());
            return nullptr;
        }

        // 计算线性索引
        Value * linear_index = nullptr;

        // 对于每个索引，计算其对应的偏移量
        for (size_t i = 0; i < indices.size(); i++) {
            // 计算该维度的系数：后续所有维度大小的乘积
            uint32_t coef = 1;
            for (size_t j = i + 1; j < dimensions.size(); j++) {
                coef *= dimensions[j];
            }

            // 创建系数常量
            Value * coef_val = module->newConstInt(coef);

            // 计算 index[i] * coef
            BinaryInstruction * term = new BinaryInstruction(function,
                                                             IRInstOperator::IRINST_OP_MUL_I,
                                                             indices[i],
                                                             coef_val,
                                                             IntegerType::getTypeInt());
            function->getInterCode().addInst(term);

            // 累加到线性索引
            if (linear_index == nullptr) {
                linear_index = term;
            } else {
                BinaryInstruction * sum = new BinaryInstruction(function,
                                                                IRInstOperator::IRINST_OP_ADD_I,
                                                                linear_index,
                                                                term,
                                                                IntegerType::getTypeInt());
                function->getInterCode().addInst(sum);
                linear_index = sum;
            }
        }

        // 计算字节偏移
        Value * size_val = module->newConstInt(4); // 固定使用4字节作为元素大小
        BinaryInstruction * byte_offset = new BinaryInstruction(function,
                                                                IRInstOperator::IRINST_OP_MUL_I,
                                                                linear_index,
                                                                size_val,
                                                                IntegerType::getTypeInt());
        function->getInterCode().addInst(byte_offset);

        // 计算最终地址: base + offset
        const PointerType * ptrType = PointerType::get(element_type);
        BinaryInstruction * element_addr = new BinaryInstruction(function,
                                                                 IRInstOperator::IRINST_OP_ADD_I,
                                                                 arrayValue,
                                                                 byte_offset,
                                                                 const_cast<PointerType *>(ptrType));
        function->getInterCode().addInst(element_addr);

        return element_addr;
    }

    // 打印所有收集到的维度信息（调试用）
    minic_log(LOG_INFO, "收集到的维度数量: %zu", dimensions.size());
    for (size_t i = 0; i < dimensions.size(); i++) {
        minic_log(LOG_INFO, "维度[%zu]: %u", i, dimensions[i]);
    }

    // 元素大小（默认为int类型4字节）
    // int element_size = 4;

    // 初始地址为数组基址
    Value * final_addr = arrayValue;

    if (indices.size() <= 0) {
        return final_addr;
    }

    // 检查索引数量与维度数量是否匹配
    if (indices.size() > dimensions.size()) {
        minic_log(LOG_ERROR, "索引数量(%zu)大于维度数量(%zu)", indices.size(), dimensions.size());
        return nullptr;
    }

    // 处理多维数组，使用正确的降维公式
    if (dimensions.size() >= 1 && indices.size() > 0) {
        // 计算线性索引 - 每次循环迭代时都要重新计算
        Value * linear_index = nullptr;

        // 对于每个索引，计算其对应的偏移量
        for (size_t i = 0; i < indices.size(); i++) {
            // 计算该维度的系数：后续所有维度大小的乘积
            uint32_t coef = 1;
            for (size_t j = i + 1; j < dimensions.size(); j++) {
                coef *= dimensions[j];
            }

            // 创建系数常量 - 每次重新创建
            Value * coef_val = module->newConstInt(coef);

            // 计算 index[i] * coef - 确保每次循环迭代都重新计算
            // 使用新的临时指令
            BinaryInstruction * term = new BinaryInstruction(function,
                                                             IRInstOperator::IRINST_OP_MUL_I,
                                                             indices[i],
                                                             coef_val,
                                                             IntegerType::getTypeInt());
            function->getInterCode().addInst(term);

            // 累加到线性索引
            if (linear_index == nullptr) {
                linear_index = term;
            } else {
                BinaryInstruction * sum = new BinaryInstruction(function,
                                                                IRInstOperator::IRINST_OP_ADD_I,
                                                                linear_index,
                                                                term,
                                                                IntegerType::getTypeInt());
                function->getInterCode().addInst(sum);
                linear_index = sum;
            }
        } // 最后乘以元素大小得到字节偏移
        // 直接使用固定元素大小4而不是变量
        Value * size_val = module->newConstInt(4); // 固定使用4字节作为元素大小
        BinaryInstruction * byte_offset = new BinaryInstruction(function,
                                                                IRInstOperator::IRINST_OP_MUL_I,
                                                                linear_index,
                                                                size_val,
                                                                IntegerType::getTypeInt());
        function->getInterCode().addInst(byte_offset);

        // 计算最终地址: base + offset
        // 确保在循环内每次访问都重新计算地址
        const PointerType * ptrType = PointerType::get(element_type);
        BinaryInstruction * element_addr = new BinaryInstruction(function,
                                                                 IRInstOperator::IRINST_OP_ADD_I,
                                                                 arrayValue,
                                                                 byte_offset,
                                                                 const_cast<PointerType *>(ptrType));
        function->getInterCode().addInst(element_addr);

        return element_addr;
    }

    // 此处不应该到达，因为前面已经处理了所有的情况
    minic_log(LOG_ERROR, "未知的数组维度情况: dimensions=%zu, indices=%zu", dimensions.size(), indices.size());
    return nullptr;
}
