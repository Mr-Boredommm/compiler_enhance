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
        // 普通数组类型
        std::string result = "[" + std::to_string(numElements) + " x ";

        // 递归获取元素类型
        Type * currElemType = elementType;
        if (currElemType->getTypeID() == Type::ArrayTyID) {
            // 如果元素类型也是数组，则递归获取其字符串表示
            result += currElemType->toString();
        } else {
            // 基本类型
            result += currElemType->toString();
        }

        result += "]";
        return result;
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
