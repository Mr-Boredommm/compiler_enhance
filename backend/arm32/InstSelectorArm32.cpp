///
/// @file InstSelectorArm32.cpp
/// @brief 指令选择器-ARM32的实现
/// @author zenglj (zenglj@live.com)
/// @version 1.0
/// @date 2024-11-21
///
/// @copyright Copyright (c) 2024
///
/// @par 修改日志:
/// <table>
/// <tr><th>Date       <th>Version <th>Author  <th>Description
/// <tr><td>2024-11-21 <td>1.0     <td>zenglj  <td>新做
/// </table>
///
#include <cstdio>

#include "Common.h"
#include "ILocArm32.h"
#include "InstSelectorArm32.h"
#include "PlatformArm32.h"

#include "PointerType.h"
#include "RegVariable.h"
#include "Function.h"

#include "LabelInstruction.h"
#include "GotoInstruction.h"
#include "FuncCallInstruction.h"
#include "MoveInstruction.h"
#include "IcmpInstruction.h"
#include "BcInstruction.h"

/// @brief 构造函数
/// @param _irCode 指令
/// @param _iloc ILoc
/// @param _func 函数
InstSelectorArm32::InstSelectorArm32(vector<Instruction *> & _irCode,
                                     ILocArm32 & _iloc,
                                     Function * _func,
                                     SimpleRegisterAllocator & allocator)
    : ir(_irCode), iloc(_iloc), func(_func), simpleRegisterAllocator(allocator)
{
    translator_handlers[IRInstOperator::IRINST_OP_ENTRY] = &InstSelectorArm32::translate_entry;
    translator_handlers[IRInstOperator::IRINST_OP_EXIT] = &InstSelectorArm32::translate_exit;

    translator_handlers[IRInstOperator::IRINST_OP_LABEL] = &InstSelectorArm32::translate_label;
    translator_handlers[IRInstOperator::IRINST_OP_GOTO] = &InstSelectorArm32::translate_goto;

    translator_handlers[IRInstOperator::IRINST_OP_ASSIGN] = &InstSelectorArm32::translate_assign;

    translator_handlers[IRInstOperator::IRINST_OP_ADD_I] = &InstSelectorArm32::translate_add_int32;
    translator_handlers[IRInstOperator::IRINST_OP_SUB_I] = &InstSelectorArm32::translate_sub_int32;
    translator_handlers[IRInstOperator::IRINST_OP_MUL_I] = &InstSelectorArm32::translate_mul_int32;
    translator_handlers[IRInstOperator::IRINST_OP_DIV_I] = &InstSelectorArm32::translate_div_int32;
    translator_handlers[IRInstOperator::IRINST_OP_MOD_I] = &InstSelectorArm32::translate_mod_int32;
    translator_handlers[IRInstOperator::IRINST_OP_NEG_I] = &InstSelectorArm32::translate_neg_int32;

    // 添加对条件比较和条件分支指令的支持
    translator_handlers[IRInstOperator::IRINST_OP_ICMP] = &InstSelectorArm32::translate_icmp;
    translator_handlers[IRInstOperator::IRINST_OP_BC] = &InstSelectorArm32::translate_bc;

    translator_handlers[IRInstOperator::IRINST_OP_FUNC_CALL] = &InstSelectorArm32::translate_call;
    translator_handlers[IRInstOperator::IRINST_OP_ARG] = &InstSelectorArm32::translate_arg;
}

///
/// @brief 析构函数
///
InstSelectorArm32::~InstSelectorArm32()
{}

/// @brief 指令选择执行
void InstSelectorArm32::run()
{
    for (auto inst: ir) {

        // 逐个指令进行翻译
        if (!inst->isDead()) {
            translate(inst);
        }
    }
}

/// @brief 指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate(Instruction * inst)
{
    // 操作符
    IRInstOperator op = inst->getOp();

    map<IRInstOperator, translate_handler>::const_iterator pIter;
    pIter = translator_handlers.find(op);
    if (pIter == translator_handlers.end()) {
        // 没有找到，则说明当前不支持
        printf("Translate: Operator(%d) not support", (int) op);
        return;
    }

    // 开启时输出IR指令作为注释
    if (showLinearIR) {
        outputIRInstruction(inst);
    }

    (this->*(pIter->second))(inst);
}

///
/// @brief 输出IR指令
///
void InstSelectorArm32::outputIRInstruction(Instruction * inst)
{
    std::string irStr;
    inst->toString(irStr);
    if (!irStr.empty()) {
        iloc.comment(irStr);
    }
}

/// @brief NOP翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_nop(Instruction * inst)
{
    (void) inst;
    iloc.nop();
}

/// @brief Label指令指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_label(Instruction * inst)
{
    Instanceof(labelInst, LabelInstruction *, inst);

    iloc.label(labelInst->getName());
}

/// @brief goto指令指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_goto(Instruction * inst)
{
    Instanceof(gotoInst, GotoInstruction *, inst);

    // 无条件跳转
    iloc.jump(gotoInst->getTarget()->getName());
}

/// @brief 函数入口指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_entry(Instruction * inst)
{
    // 查看保护的寄存器
    auto & protectedRegNo = func->getProtectedReg();
    auto & protectedRegStr = func->getProtectedRegStr();

    bool first = true;
    for (auto regno: protectedRegNo) {
        if (first) {
            protectedRegStr = PlatformArm32::regName[regno];
            first = false;
        } else {
            protectedRegStr += "," + PlatformArm32::regName[regno];
        }
    }

    if (!protectedRegStr.empty()) {
        iloc.inst("push", "{" + protectedRegStr + "}");
    }

    // 为fun分配栈帧，含局部变量、函数调用值传递的空间等
    iloc.allocStack(func, ARM32_TMP_REG_NO);
}

/// @brief 函数出口指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_exit(Instruction * inst)
{
    if (inst->getOperandsNum()) {
        // 存在返回值
        Value * retVal = inst->getOperand(0);

        // 赋值给寄存器R0
        iloc.load_var(0, retVal);
    }

    // 恢复栈空间
    iloc.inst("mov", "sp", "fp");

    // 保护寄存器的恢复
    auto & protectedRegStr = func->getProtectedRegStr();
    if (!protectedRegStr.empty()) {
        iloc.inst("pop", "{" + protectedRegStr + "}");
    }

    iloc.inst("bx", "lr");
}

/// @brief 赋值指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_assign(Instruction * inst)
{
    Value * result = inst->getOperand(0);
    Value * arg1 = inst->getOperand(1);

    int32_t arg1_regId = arg1->getRegId();
    int32_t result_regId = result->getRegId();

    if (arg1_regId != -1) {
        // 寄存器 => 内存
        // 寄存器 => 寄存器

        // r8 -> rs 可能用到r9
        iloc.store_var(arg1_regId, result, ARM32_TMP_REG_NO);
    } else if (result_regId != -1) {
        // 内存变量 => 寄存器

        iloc.load_var(result_regId, arg1);
    } else {
        // 内存变量 => 内存变量

        int32_t temp_regno = simpleRegisterAllocator.Allocate();

        // arg1 -> r8
        iloc.load_var(temp_regno, arg1);

        // r8 -> rs 可能用到r9
        iloc.store_var(temp_regno, result, ARM32_TMP_REG_NO);

        simpleRegisterAllocator.free(temp_regno);
    }
}

/// @brief 二元操作指令翻译成ARM32汇编
/// @param inst IR指令
/// @param operator_name 操作码
/// @param rs_reg_no 结果寄存器号
/// @param op1_reg_no 源操作数1寄存器号
/// @param op2_reg_no 源操作数2寄存器号
void InstSelectorArm32::translate_two_operator(Instruction * inst, string operator_name)
{
    Value * result = inst;
    Value * arg1 = inst->getOperand(0);
    Value * arg2 = inst->getOperand(1);

    int32_t arg1_reg_no = arg1->getRegId();
    int32_t arg2_reg_no = arg2->getRegId();
    int32_t result_reg_no = inst->getRegId();
    int32_t load_result_reg_no, load_arg1_reg_no, load_arg2_reg_no;

    // 看arg1是否是寄存器，若是则寄存器寻址，否则要load变量到寄存器中
    if (arg1_reg_no == -1) {

        // 分配一个寄存器r8
        load_arg1_reg_no = simpleRegisterAllocator.Allocate(arg1);

        // arg1 -> r8，这里可能由于偏移不满足指令的要求，需要额外分配寄存器
        iloc.load_var(load_arg1_reg_no, arg1);
    } else {
        load_arg1_reg_no = arg1_reg_no;
    }

    // 看arg2是否是寄存器，若是则寄存器寻址，否则要load变量到寄存器中
    if (arg2_reg_no == -1) {

        // 分配一个寄存器r9
        load_arg2_reg_no = simpleRegisterAllocator.Allocate(arg2);

        // arg2 -> r9
        iloc.load_var(load_arg2_reg_no, arg2);
    } else {
        load_arg2_reg_no = arg2_reg_no;
    }

    // 看结果变量是否是寄存器，若不是则需要分配一个新的寄存器来保存运算的结果
    if (result_reg_no == -1) {
        // 分配一个寄存器r10，用于暂存结果
        load_result_reg_no = simpleRegisterAllocator.Allocate(result);
    } else {
        load_result_reg_no = result_reg_no;
    }

    // r8 + r9 -> r10
    iloc.inst(operator_name,
              PlatformArm32::regName[load_result_reg_no],
              PlatformArm32::regName[load_arg1_reg_no],
              PlatformArm32::regName[load_arg2_reg_no]);

    // 结果不是寄存器，则需要把rs_reg_name保存到结果变量中
    if (result_reg_no == -1) {

        // 这里使用预留的临时寄存器，因为立即数可能过大，必须借助寄存器才可操作。

        // r10 -> result
        iloc.store_var(load_result_reg_no, result, ARM32_TMP_REG_NO);
    }

    // 释放寄存器
    simpleRegisterAllocator.free(arg1);
    simpleRegisterAllocator.free(arg2);
    simpleRegisterAllocator.free(result);
}

/// @brief 整数加法指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_add_int32(Instruction * inst)
{
    translate_two_operator(inst, "add");
}

/// @brief 整数减法指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_sub_int32(Instruction * inst)
{
    translate_two_operator(inst, "sub");
}

/// @brief 整数乘法指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_mul_int32(Instruction * inst)
{
    translate_two_operator(inst, "mul");
}

/// @brief 整数除法指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_div_int32(Instruction * inst)
{
    Value * result = inst;
    Value * arg1 = inst->getOperand(0);
    Value * arg2 = inst->getOperand(1);

    // 分配寄存器
    int32_t arg1_reg = arg1->getRegId() != -1 ? arg1->getRegId() : simpleRegisterAllocator.Allocate(arg1);
    int32_t arg2_reg = arg2->getRegId() != -1 ? arg2->getRegId() : simpleRegisterAllocator.Allocate(arg2);
    int32_t result_reg = result->getRegId() != -1 ? result->getRegId() : simpleRegisterAllocator.Allocate(result);

    // 加载操作数到寄存器
    if (arg1->getRegId() == -1) {
        iloc.load_var(arg1_reg, arg1);
    }
    if (arg2->getRegId() == -1) {
        iloc.load_var(arg2_reg, arg2);
    }

    // 执行除法运算
    iloc.inst("sdiv",
              PlatformArm32::regName[result_reg],
              PlatformArm32::regName[arg1_reg],
              PlatformArm32::regName[arg2_reg]);

    // 保存结果
    if (result->getRegId() == -1) {
        iloc.store_var(result_reg, result, ARM32_TMP_REG_NO);
    }

    // 释放寄存器
    simpleRegisterAllocator.free(arg1);
    simpleRegisterAllocator.free(arg2);
    simpleRegisterAllocator.free(result);
}

/// @brief 整数取模指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_mod_int32(Instruction * inst)
{
    // ARM没有直接取模指令，需要先除法再乘法最后减法
    Value * result = inst;
    Value * arg1 = inst->getOperand(0);
    Value * arg2 = inst->getOperand(1);

    // 分配临时寄存器
    int32_t temp1 = simpleRegisterAllocator.Allocate();
    int32_t temp2 = simpleRegisterAllocator.Allocate();
    int32_t result_reg = result->getRegId() != -1 ? result->getRegId() : simpleRegisterAllocator.Allocate(result);

    // 确保操作数已加载到寄存器
    int32_t arg1_reg = arg1->getRegId() != -1 ? arg1->getRegId() : simpleRegisterAllocator.Allocate(arg1);
    int32_t arg2_reg = arg2->getRegId() != -1 ? arg2->getRegId() : simpleRegisterAllocator.Allocate(arg2);

    if (arg1->getRegId() == -1) {
        iloc.load_var(arg1_reg, arg1);
    }
    if (arg2->getRegId() == -1) {
        iloc.load_var(arg2_reg, arg2);
    }

    // 确保操作数已加载到寄存器
    if (arg1->getRegId() == -1) {
        iloc.load_var(arg1_reg, arg1);
    }
    if (arg2->getRegId() == -1) {
        iloc.load_var(arg2_reg, arg2);
    }

    // 计算商 = arg1 / arg2
    iloc.inst("sdiv",
              PlatformArm32::regName[temp1],
              PlatformArm32::regName[arg1_reg],
              PlatformArm32::regName[arg2_reg]);

    // 计算商*除数
    iloc.inst("mul", PlatformArm32::regName[temp2], PlatformArm32::regName[temp1], PlatformArm32::regName[arg2_reg]);

    // 计算余数 = 被除数 - (商*除数)
    iloc.inst("sub",
              PlatformArm32::regName[result_reg],
              PlatformArm32::regName[arg1_reg],
              PlatformArm32::regName[temp2]);

    // 保存结果
    if (result->getRegId() == -1) {
        iloc.store_var(result_reg, result, ARM32_TMP_REG_NO);
    }

    // 释放临时寄存器
    simpleRegisterAllocator.free(temp1);
    simpleRegisterAllocator.free(temp2);
    if (arg1->getRegId() == -1) {
        simpleRegisterAllocator.free(arg1);
    }
    if (arg2->getRegId() == -1) {
        simpleRegisterAllocator.free(arg2);
    }
    if (result->getRegId() == -1) {
        simpleRegisterAllocator.free(result);
    }
}

/// @brief 整数负号指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_neg_int32(Instruction * inst)
{
    // 使用RSB指令实现负号运算: rsb rd, rn, #0 等价于 rd = 0 - rn
    Value * result = inst;
    Value * arg = inst->getOperand(0);

    int32_t arg_reg = arg->getRegId() != -1 ? arg->getRegId() : simpleRegisterAllocator.Allocate(arg);
    int32_t result_reg = result->getRegId() != -1 ? result->getRegId() : simpleRegisterAllocator.Allocate(result);

    // 执行负号运算
    iloc.inst("rsb", PlatformArm32::regName[result_reg], PlatformArm32::regName[arg_reg], "#0");

    // 保存结果
    if (result->getRegId() == -1) {
        iloc.store_var(result_reg, result, ARM32_TMP_REG_NO);
    }

    // 释放寄存器
    simpleRegisterAllocator.free(arg);
    simpleRegisterAllocator.free(result);
}

/// @brief 函数调用指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_call(Instruction * inst)
{
    FuncCallInstruction * callInst = dynamic_cast<FuncCallInstruction *>(inst);

    int32_t operandNum = callInst->getOperandsNum();

    if (operandNum != realArgCount) {

        // 两者不一致 也可能没有ARG指令，正常
        if (realArgCount != 0) {

            minic_log(LOG_ERROR, "ARG指令的个数与调用函数个数不一致");
        }
    }

    if (operandNum) {

        // 强制占用这几个寄存器参数传递的寄存器
        simpleRegisterAllocator.Allocate(0);
        simpleRegisterAllocator.Allocate(1);
        simpleRegisterAllocator.Allocate(2);
        simpleRegisterAllocator.Allocate(3);

        // 前四个的后面参数采用栈传递
        int esp = 0;
        for (int32_t k = 4; k < operandNum; k++) {

            auto arg = callInst->getOperand(k);

            // 新建一个内存变量，用于栈传值到形参变量中
            MemVariable * newVal = func->newMemVariable((Type *) PointerType::get(arg->getType()));
            newVal->setMemoryAddr(ARM32_SP_REG_NO, esp);
            esp += 4;

            Instruction * assignInst = new MoveInstruction(func, newVal, arg);

            // 翻译赋值指令
            translate_assign(assignInst);

            delete assignInst;
        }

        for (int32_t k = 0; k < operandNum && k < 4; k++) {

            auto arg = callInst->getOperand(k);

            // 检查实参的类型是否是临时变量。
            // 如果是临时变量，该变量可更改为寄存器变量即可，或者设置寄存器号
            // 如果不是，则必须开辟一个寄存器变量，然后赋值即可

            Instruction * assignInst = new MoveInstruction(func, PlatformArm32::intRegVal[k], arg);

            // 翻译赋值指令
            translate_assign(assignInst);

            delete assignInst;
        }
    }

    iloc.call_fun(callInst->getName());

    if (operandNum) {
        simpleRegisterAllocator.free(0);
        simpleRegisterAllocator.free(1);
        simpleRegisterAllocator.free(2);
        simpleRegisterAllocator.free(3);
    }

    // 赋值指令
    if (callInst->hasResultValue()) {

        // 新建一个赋值操作
        Instruction * assignInst = new MoveInstruction(func, callInst, PlatformArm32::intRegVal[0]);

        // 翻译赋值指令
        translate_assign(assignInst);

        delete assignInst;
    }

    // 函数调用后清零，使得下次可正常统计
    realArgCount = 0;
}

///
/// @brief 实参指令翻译成ARM32汇编
/// @param inst
///
void InstSelectorArm32::translate_arg(Instruction * inst)
{
    // 翻译之前必须确保源操作数要么是寄存器，要么是内存，否则出错。
    Value * src = inst->getOperand(0);

    // 当前统计的ARG指令个数
    int32_t regId = src->getRegId();

    if (realArgCount < 4) {
        // 前四个参数
        if (regId != -1) {
            if (regId != realArgCount) {
                // 肯定寄存器分配有误
                minic_log(LOG_ERROR, "第%d个ARG指令对象寄存器分配有误: %d", argCount + 1, regId);
            }
        } else {
            minic_log(LOG_ERROR, "第%d个ARG指令对象不是寄存器", argCount + 1);
        }
    } else {
        // 必须是内存分配，若不是则出错
        int32_t baseRegId;
        bool result = src->getMemoryAddr(&baseRegId);
        if ((!result) || (baseRegId != ARM32_SP_REG_NO)) {

            minic_log(LOG_ERROR, "第%d个ARG指令对象不是SP寄存器寻址", argCount + 1);
        }
    }

    realArgCount++;
}

/// @brief 条件比较指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_icmp(Instruction * inst)
{
    // 将IcmpInstruction转换为正确的类型
    IcmpInstruction * icmpInst = dynamic_cast<IcmpInstruction *>(inst);
    if (!icmpInst) {
        fprintf(stderr, "Error: Invalid ICMP instruction\n");
        return;
    }

    // 获取操作数和结果
    Value * result = inst;
    Value * left = icmpInst->getLeft();
    Value * right = icmpInst->getRight();
    std::string cmpType = icmpInst->getCmpType();

    if (!left || !right) {
        fprintf(stderr, "Error: Null operand in ICMP instruction\n");
        // 使用默认值设置结果为0
        int32_t result_reg = result->getRegId() != -1 ? result->getRegId() : simpleRegisterAllocator.Allocate(result);
        iloc.inst("mov", PlatformArm32::regName[result_reg], "#0");

        // 如果结果不是寄存器变量，需要保存到内存
        if (result->getRegId() == -1) {
            iloc.store_var(result_reg, result, ARM32_TMP_REG_NO);
            simpleRegisterAllocator.free(result_reg);
        }
        return;
    }

    // 确保操作数在寄存器中，处理可能的异常
    int32_t left_reg = -1, right_reg = -1, result_reg = -1;

    try {
        left_reg = left->getRegId() != -1 ? left->getRegId() : simpleRegisterAllocator.Allocate(left);
    } catch (const std::exception & e) {
        fprintf(stderr, "Error: Failed to allocate register for left operand: %s\n", e.what());
        left_reg = ARM32_TMP_REG_NO;
        iloc.inst("mov", PlatformArm32::regName[left_reg], "#0");
    }

    try {
        right_reg = right->getRegId() != -1 ? right->getRegId() : simpleRegisterAllocator.Allocate(right);
    } catch (const std::exception & e) {
        fprintf(stderr, "Error: Failed to allocate register for right operand: %s\n", e.what());
        right_reg = ARM32_TMP_REG_NO;
        iloc.inst("mov", PlatformArm32::regName[right_reg], "#0");
    }

    try {
        result_reg = result->getRegId() != -1 ? result->getRegId() : simpleRegisterAllocator.Allocate(result);
    } catch (const std::exception & e) {
        fprintf(stderr, "Error: Failed to allocate register for result: %s\n", e.what());
        result_reg = ARM32_TMP_REG_NO;
    }

    // 先加载左操作数到寄存器
    if (left->getRegId() == -1) {
        try {
            iloc.load_var(left_reg, left);
        } catch (const std::exception & e) {
            fprintf(stderr, "Error: Failed to load left operand: %s\n", e.what());
            iloc.inst("mov", PlatformArm32::regName[left_reg], "#0");
        }
    }

    // 再加载右操作数到寄存器
    if (right->getRegId() == -1) {
        try {
            iloc.load_var(right_reg, right);
        } catch (const std::exception & e) {
            fprintf(stderr, "Error: Failed to load right operand: %s\n", e.what());
            iloc.inst("mov", PlatformArm32::regName[right_reg], "#0");
        }
    }

    // 使用cmp指令进行比较
    iloc.inst("cmp", PlatformArm32::regName[left_reg], PlatformArm32::regName[right_reg]);

    // 根据比较类型设置条件码
    std::string condCode;
    if (cmpType == "eq") {
        condCode = "eq"; // 等于
    } else if (cmpType == "ne") {
        condCode = "ne"; // 不等于
    } else if (cmpType == "gt") {
        condCode = "gt"; // 大于
    } else if (cmpType == "ge") {
        condCode = "ge"; // 大于等于
    } else if (cmpType == "lt") {
        condCode = "lt"; // 小于
    } else if (cmpType == "le") {
        condCode = "le"; // 小于等于
    } else {
        fprintf(stderr, "Error: Unknown comparison type: %s\n", cmpType.c_str());
        iloc.inst("mov", PlatformArm32::regName[result_reg], "#0");

        // 如果结果不是寄存器变量，需要保存到内存
        if (result->getRegId() == -1) {
            iloc.store_var(result_reg, result, ARM32_TMP_REG_NO);
        }

        // 释放临时寄存器
        if (left->getRegId() == -1 && left_reg != ARM32_TMP_REG_NO) {
            try {
                simpleRegisterAllocator.free(left_reg);
            } catch (...) {
            }
        }
        if (right->getRegId() == -1 && right_reg != ARM32_TMP_REG_NO) {
            try {
                simpleRegisterAllocator.free(right_reg);
            } catch (...) {
            }
        }
        if (result->getRegId() == -1 && result_reg != ARM32_TMP_REG_NO) {
            try {
                simpleRegisterAllocator.free(result_reg);
            } catch (...) {
            }
        }
        return;
    }

    // 将比较结果存入result寄存器，使用条件移动指令
    // 先设置为0
    iloc.inst("mov", PlatformArm32::regName[result_reg], "#0");
    // 如果条件成立，设置为1
    iloc.inst("mov" + condCode, PlatformArm32::regName[result_reg], "#1");

    // 如果结果不是寄存器变量，需要保存到内存
    if (result->getRegId() == -1) {
        try {
            iloc.store_var(result_reg, result, ARM32_TMP_REG_NO);
        } catch (const std::exception & e) {
            fprintf(stderr, "Error: Failed to store result: %s\n", e.what());
        }
    }

    // 释放临时寄存器
    if (left->getRegId() == -1 && left_reg != ARM32_TMP_REG_NO) {
        try {
            simpleRegisterAllocator.free(left_reg);
        } catch (const std::exception & e) {
            fprintf(stderr, "Error: Failed to free left register: %s\n", e.what());
        }
    }
    if (right->getRegId() == -1 && right_reg != ARM32_TMP_REG_NO) {
        try {
            simpleRegisterAllocator.free(right_reg);
        } catch (const std::exception & e) {
            fprintf(stderr, "Error: Failed to free right register: %s\n", e.what());
        }
    }
    if (result->getRegId() == -1 && result_reg != ARM32_TMP_REG_NO) {
        try {
            simpleRegisterAllocator.free(result_reg);
        } catch (const std::exception & e) {
            fprintf(stderr, "Error: Failed to free result register: %s\n", e.what());
        }
    }
}

/// @brief 条件分支指令翻译成ARM32汇编
/// @param inst IR指令
void InstSelectorArm32::translate_bc(Instruction * inst)
{
    // 将BcInstruction转换为正确的类型
    BcInstruction * bcInst = dynamic_cast<BcInstruction *>(inst);
    if (!bcInst) {
        fprintf(stderr, "Error: Invalid BC instruction\n");
        return;
    }

    // 获取条件值
    Value * condition = bcInst->getCondition();
    if (!condition) {
        fprintf(stderr, "Error: Null condition in BC instruction\n");
        return;
    }

    // 获取真分支和假分支标签
    LabelInstruction * trueLabel = dynamic_cast<LabelInstruction *>(bcInst->getTrueLabel());
    LabelInstruction * falseLabel = dynamic_cast<LabelInstruction *>(bcInst->getFalseLabel());

    if (!trueLabel || !falseLabel) {
        fprintf(stderr, "Error: Invalid labels in BC instruction\n");
        return;
    }

    // 确保条件值在寄存器中
    int32_t cond_reg = -1;
    try {
        cond_reg = condition->getRegId() != -1 ? condition->getRegId() : simpleRegisterAllocator.Allocate(condition);
    } catch (const std::exception & e) {
        fprintf(stderr, "Error: Failed to allocate register for condition: %s\n", e.what());
        // 使用一个默认寄存器，设置为0
        cond_reg = ARM32_TMP_REG_NO;
        iloc.inst("mov", PlatformArm32::regName[cond_reg], "#0");
    }

    // 加载条件值到寄存器，处理可能的异常
    if (condition->getRegId() == -1) {
        try {
            iloc.load_var(cond_reg, condition);
        } catch (const std::exception & e) {
            fprintf(stderr, "Error: Failed to load condition value: %s\n", e.what());
            // 设置为0
            iloc.inst("mov", PlatformArm32::regName[cond_reg], "#0");
        }
    }

    // 比较条件值与0
    iloc.inst("cmp", PlatformArm32::regName[cond_reg], "#0");

    // 如果条件为真（非0），跳转到真分支，否则跳转到假分支
    iloc.inst("bne", trueLabel->getName());
    iloc.inst("b", falseLabel->getName());

    // 释放临时寄存器
    if (condition->getRegId() == -1 && cond_reg != ARM32_TMP_REG_NO) {
        try {
            simpleRegisterAllocator.free(cond_reg);
        } catch (const std::exception & e) {
            fprintf(stderr, "Error: Failed to free register: %s\n", e.what());
        }
    }
}
