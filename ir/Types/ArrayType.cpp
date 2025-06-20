#include "ArrayType.h"

/// @brief 构造函数
/// @param elementType 数组元素类型
/// @param numElements 数组元素个数，0表示指针类型
ArrayType::ArrayType(Type * elementType, uint32_t numElements)
    : Type(Type::ArrayTyID), elementType(elementType), numElements(numElements)
{}

/// @brief 获取数组元素类型
/// @return 数组元素类型
Type * ArrayType::getElementType() const
{
    return elementType;
}

/// @brief 获取数组元素个数
/// @return 数组元素个数
uint32_t ArrayType::getNumElements() const
{
    return numElements;
}

/// @brief 创建一个数组类型
/// @param elementType 数组元素类型
/// @param numElements 数组元素个数
/// @return 数组类型
ArrayType * ArrayType::get(Type * elementType, uint32_t numElements)
{
    return new ArrayType(elementType, numElements);
}

/// @brief 获取类型字符串表示
/// @return 类型字符串
std::string ArrayType::toString() const
{
    if (numElements == 0) {
        // 指针类型，用于函数形参
        return elementType->toString() + "*";
    } else {
        // 对于数组类型，只返回基本类型，维度信息将在变量名后面添加
        // 例如：i32 @a[10][20]

        // 先找到最底层的元素类型
        Type * baseType = elementType;

        // 递归查找最基本的类型
        while (baseType->getTypeID() == Type::ArrayTyID) {
            ArrayType * arrType = static_cast<ArrayType *>(baseType);
            baseType = arrType->getElementType();
        }

        // 只返回基本类型
        return baseType->toString();
    }
}

/// @brief 获得类型所占内存空间大小
/// @return 大小（字节）
int32_t ArrayType::getSize() const
{
    if (numElements == 0) {
        // 指针类型，大小为4（32位系统）
        return 4;
    } else {
        // 数组大小 = 元素大小 * 元素个数
        return elementType->getSize() * numElements;
    }
}

/// @brief 获取数组的所有维度大小
/// @return 包含所有维度大小的向量
std::vector<uint32_t> ArrayType::getAllDimensions() const
{
    std::vector<uint32_t> dimensions;

    // 添加当前维度
    dimensions.push_back(numElements);

    // 递归添加子维度
    Type * currType = elementType;
    while (currType && currType->getTypeID() == Type::ArrayTyID) {
        ArrayType * arrayType = static_cast<ArrayType *>(currType);
        dimensions.push_back(arrayType->getNumElements());
        currType = arrayType->getElementType();
    }

    return dimensions;
}
