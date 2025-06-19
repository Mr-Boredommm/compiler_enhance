#pragma once

#include "Type.h"

class ArrayType : public Type {
public:
    /// @brief 构造函数
    /// @param elementType 数组元素类型
    /// @param numElements 数组元素个数，0表示指针类型
    ArrayType(Type * elementType, uint32_t numElements);

    /// @brief 获取数组元素类型
    /// @return 数组元素类型
    Type * getElementType() const;

    /// @brief 获取数组元素个数
    /// @return 数组元素个数
    uint32_t getNumElements() const;

    /// @brief 创建一个数组类型
    /// @param elementType 数组元素类型
    /// @param numElements 数组元素个数
    /// @return 数组类型
    static ArrayType * get(Type * elementType, uint32_t numElements);

    /// @brief 获取类型字符串表示
    /// @return 类型字符串
    virtual std::string toString() const override;

    /// @brief 获得类型所占内存空间大小
    /// @return 大小（字节）
    virtual int32_t getSize() const override;

private:
    Type * elementType;   ///< 数组元素类型
    uint32_t numElements; ///< 数组元素个数，0表示指针类型
};
