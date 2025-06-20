///
/// @file GlobalVariable.h
/// @brief 全局变量描述类
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

#include "GlobalValue.h"
#include "IRConstant.h"
#include "Types/ArrayType.h"
#include <vector>

///
/// @brief 全局变量，寻址时通过符号名或变量名来寻址
///
class GlobalVariable : public GlobalValue {

public:
    ///
    /// @brief 构建全局变量，默认对齐为4字节
    /// @param _type 类型
    /// @param _name 名字
    ///
    explicit GlobalVariable(Type * _type, std::string _name) : GlobalValue(_type, _name)
    {
        // 设置对齐大小
        setAlignment(4);
    }

    ///
    /// @brief  检查是否是函数
    /// @return true 是函数
    /// @return false 不是函数
    ///
    [[nodiscard]] bool isGlobalVarible() const override
    {
        return true;
    }

    ///
    /// @brief 是否属于BSS段的变量，即未初始化过的变量，或者初值都为0的变量
    /// @return true
    /// @return false
    ///
    [[nodiscard]] bool isInBSSSection() const
    {
        return this->inBSSSection;
    }

    ///
    /// @brief 取得变量所在的作用域层级
    /// @return int32_t 层级
    ///
    int32_t getScopeLevel() override
    {
        return 0;
    }

    ///
    /// @brief 对该Value进行Load用的寄存器编号
    /// @return int32_t 寄存器编号
    ///
    int32_t getLoadRegId() override
    {
        return this->loadRegNo;
    }

    ///
    /// @brief 对该Value进行Load用的寄存器编号
    /// @return int32_t 寄存器编号
    ///
    void setLoadRegId(int32_t regId) override
    {
        this->loadRegNo = regId;
    }

    ///
    /// @brief Declare指令IR显示
    /// @param str
    ///
    void toDeclareString(std::string & str)
    {
        // 修改为DragonIR风格
        // 示例: declare i32 @a[10] 而不是 declare [10 x i32] @a = 0
        Type * type = getType();
        std::string typeName;

        // 处理数组类型
        if (type->getTypeID() == Type::ArrayTyID) {
            ArrayType * arrayType = static_cast<ArrayType *>(type);
            Type * elementType = arrayType->getElementType();

            // 获取所有维度信息
            std::vector<uint32_t> dimensions;
            Type * currentType = type;
            while (currentType && currentType->getTypeID() == Type::ArrayTyID) {
                ArrayType * arrType = static_cast<ArrayType *>(currentType);
                dimensions.push_back(arrType->getNumElements());
                currentType = arrType->getElementType();
            }

            // 使用基础元素类型
            typeName = elementType->toString();

            // 构建声明字符串
            str = "declare " + typeName + " " + getIRName();

            // 添加维度信息
            for (size_t i = 0; i < dimensions.size(); i++) {
                str += "[" + std::to_string(dimensions[i]) + "]";
            }
        } else {
            // 非数组类型
            str = "declare " + type->toString() + " " + getIRName() + " = 0";
        }
    }

private:
    ///
    /// @brief 变量加载到寄存器中时对应的寄存器编号
    ///
    int32_t loadRegNo = -1;

    ///
    /// @brief 默认全局变量在BSS段，没有初始化，或者即使初始化过，但都值都为0
    ///
    bool inBSSSection = true;
};
