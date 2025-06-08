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

/// @brief 生成IR标签名称的计数器
static int label_counter = 0;

/// @brief 生成唯一的标签名称
/// @return 唯一的标签名称
static std::string generate_label()
{
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

    // 为每个函数参数创建对应的局部变量，并生成赋值指令
    auto & params = newFunc->getParams();
    for (auto formalParam: params) {
        // 为参数创建一个对应的局部变量
        LocalVariable * localVar = static_cast<LocalVariable *>(module->newVarValue(formalParam->getType()));
        localVar->setName(formalParam->getName()); // 使用相同的名称

        // 生成从参数到局部变量的移动指令
        MoveInstruction * moveInst = new MoveInstruction(newFunc, localVar, formalParam);
        node->blockInsts.addInst(moveInst);

        // 调试信息
        minic_log(LOG_INFO, "为参数 %s 创建局部变量并生成赋值指令", formalParam->getName().c_str());
    }

    // 新建一个Value，用于保存函数的返回值，如果没有返回值可不用申请
    LocalVariable * retValue = nullptr;
    if (!type_node->type->isVoidType()) {

        // 保存函数返回值变量到函数信息中，在return语句翻译时需要设置值到这个变量中
        retValue = static_cast<LocalVariable *>(module->newVarValue(type_node->type));
    }
    newFunc->setReturnValue(retValue);

    // 功能要求6和7：为int类型函数初始化返回值变量，特别是main函数初始化为0
    if (retValue) {
        ConstInt * initValue;
        if (name_node->name == "main") {
            // 功能要求7：main函数初始化返回值为0，避免进程退出状态的随机值问题
            initValue = module->newConstInt(0);
        } else {
            // 其他int类型函数也初始化为0
            initValue = module->newConstInt(0);
        }

        // 创建初始化指令，将返回值变量初始化
        MoveInstruction * initRetInst = new MoveInstruction(newFunc, retValue, initValue);
        node->blockInsts.addInst(initRetInst);
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
        for (auto son: paramsNode->sons) {

            // 遍历Block的每个语句，进行显示或者运算
            ast_node * temp = ir_visit_ast_node(son);
            if (!temp) {
                return false;
            }

            realParams.push_back(temp->val);
            node->blockInsts.addInst(temp->blockInsts);
        }
    }

    // TODO 这里请追加函数调用的语义错误检查，这里只进行了函数参数的个数检查等，其它请自行追加。
    if (realParams.size() != calledFunction->getParams().size()) {
        // 函数参数的个数不一致，语义错误
        minic_log(LOG_ERROR, "第%lld行的被调用函数(%s)未定义或声明", (long long) lineno, funcName.c_str());
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

    // 首先检查左侧是否为参数，如果是，需要预先创建覆盖变量
    Function * currentFunc = module->getCurrentFunction();
    if (son1_node->node_type == ast_operator_type::AST_OP_LEAF_VAR_ID && currentFunc != nullptr) {
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

    // 赋值运算符的左侧操作数
    ast_node * left = ir_visit_ast_node(son1_node);
    if (!left) {
        // 某个变量没有定值
        // 这里缺省设置变量不存在则创建，因此这里不会错误
        return false;
    }

    // 赋值运算符的右侧操作数
    ast_node * right = ir_visit_ast_node(son2_node);
    if (!right) {
        // 某个变量没有定值
        return false;
    }

    // 这里只处理整型的数据，如需支持实数，则需要针对类型进行处理

    MoveInstruction * movInst = new MoveInstruction(module->getCurrentFunction(), left->val, right->val);

    // 调试信息：输出移动指令
    std::string infoStr = "生成移动指令: 从 ";
    infoStr += right->val->getName();
    infoStr += " 到 ";
    infoStr += left->val->getName();
    minic_log(LOG_INFO, "%s", infoStr.c_str());

    // 创建临时变量保存IR的值，以及线性IR指令
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(movInst);

    // 这里假定赋值的类型是一致的
    node->val = movInst;

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
        node->blockInsts.addInst(new MoveInstruction(currentFunc, currentFunc->getReturnValue(), right->val));

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
    // 不需要做什么，直接从节点中获取即可。

    return true;
}

/// @brief 标识符叶子节点翻译成线性中间IR，变量声明的不走这个语句
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_leaf_node_var_id(ast_node * node)
{
    Value * val;

    // 查找ID型Value
    // 变量，则需要在符号表中查找对应的值

    // 首先检查是否有参数覆盖变量
    Function * currentFunc = module->getCurrentFunction();
    if (currentFunc != nullptr) {
        LocalVariable * overrideVar = currentFunc->findParamOverride(node->name);
        if (overrideVar != nullptr) {
            // 找到参数覆盖变量，优先使用它
            node->val = overrideVar;
            return true;
        }
    }

    val = module->findVarValue(node->name);

    // 检查是否找到变量
    if (val == nullptr) {
        // 输出错误信息
        std::cerr << "Error: Undefined variable '" << node->name << "' at line " << node->line_no << std::endl;
        return false;
    }

    node->val = val;

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
    // 共有两个孩子，第一个类型，第二个变量名
    std::string varName = node->sons[1]->name;
    Type * varType = node->sons[0]->type;

    minic_log(LOG_INFO, "正在声明变量: %s", varName.c_str());

    // 检查是否已经存在同名的变量（包括形参）
    Value * existingVar = module->findVarValue(varName);
    if (existingVar != nullptr) {
        // 如果是形参，直接使用它，不创建新的局部变量
        FormalParam * formalParam = dynamic_cast<FormalParam *>(existingVar);
        if (formalParam != nullptr) {
            minic_log(LOG_INFO, "变量 %s 是形参，不创建新的局部变量", varName.c_str());
            node->val = existingVar;
            return true;
        }
        minic_log(LOG_INFO, "找到现有变量 %s，类型为: %s", varName.c_str(), typeid(*existingVar).name());
    }

    // TODO 这里可强化类型等检查
    node->val = module->newVarValue(varType, varName);

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

    // 生成循环体的代码
    ast_node * body = ir_visit_ast_node(node->sons[1]);
    if (!body) {
        whileLabels.pop_back();
        currentWhileStartLabel = savedCurrentWhileStartLabel;
        currentWhileEndLabel = savedCurrentWhileEndLabel;
        return false;
    }

    node->blockInsts.addInst(body->blockInsts);

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
