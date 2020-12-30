#pragma once

/// <summary>
/// 将输入的数按位倒序
/// </summary>
/// <param name="input">待按位倒序的数</param>
/// <param name="width">CRC位宽</param>
/// <returns>按位倒序后的数</returns>
template<typename T> T reverseBit(T input, size_t width)
{
    T output = 0;
    for (size_t i = 0; i < width; i++)//根据CRC位宽确定循环次数
    {
        output <<= 1;//将output左移使上一次循环中确定的位变为高位，同时在本次循环中确定LSB
        if (input & 1)//根据当前input的LSB确定当前output的LSB
        {
            output |= 1;
        }
        input >>= 1;//将input右移以便在下一次循环中获取下一个高位
    }
    return output;
}
/// <summary>
/// 直接计算CRC校验值
/// </summary>
/// <param name="pData">待计算数据的头指针</param>
/// <param name="length">待计算数据长度</param>
/// <param name="width">CRC位宽</param>
/// <param name="poly">CRC多项式</param>
/// <param name="init">输入初始值</param>
/// <param name="refin">输入是否反转</param>
/// <param name="refout">输出是否反转</param>
/// <param name="xorout">输出异或值</param>
/// <returns>CRC校验值</returns>
template<typename T> T calculateCrcDirectly(
    unsigned char* pData,
    size_t length,
    size_t width,
    T poly,
    T init,
    bool refin,
    bool refout,
    T xorout)
{
    //计算掩码
    T mask = 0;
    for (size_t i = 0; i < width; i++)
    {
        mask = (mask << 1) | 1;
    }

    T ret = init;
    while (length-- > 0)//根据输入数据的字节数依次计算
    {
        if (refin)
        {
            ret ^= reverseBit<T>(*pData++, width);
        }
        else
        {
            ret ^= static_cast<T>(*pData++) << (width - 8);
        }
        for (size_t i = 0; i < 8; i++)//按输入字节的每一位进行计算
        {
            if (ret & (static_cast<T>(1) << (width - 1)))//若首位是1则进行左移并与多项式进行模二减法（异或运算）
            {
                ret = ((ret << 1) & mask) ^ poly;
            }
            else//否则继续左移
            {
                ret = ((ret << 1) & mask);
            }
        }
    }
    if (refout)
    {
        return reverseBit<T>(ret ^ xorout, width);
    }
    else
    {
        return ret ^ xorout;
    }
}
/// <summary>
/// 计算CRC余式表
/// </summary>
/// <param name="table">CRC余式表头指针</param>
/// <param name="width">CRC位宽</param>
/// <param name="poly">CRC多项式</param>
/// <param name="refin">输入是否反转</param>
/// <param name="refout">输出是否反转</param>
template<typename T> void calculateCrcTable(
    T* table,
    size_t width,
    T poly,
    bool refin,
    bool refout)
{
    for (size_t i = 0; i < 256; i++)
    {
        unsigned char data = static_cast<unsigned char>(i);
        table[i] = calculateCrcDirectly<T>(&data, sizeof(data), width, poly, 0, refin, refout, 0);
    }
}

/// <summary>
/// 计算CRC校验值
/// </summary>
/// <param name="pData">待计算数据的头指针</param>
/// <param name="length">待计算数据长度</param>
/// <param name="width">CRC位宽</param>
/// <param name="poly">CRC多项式</param>
/// <param name="init">输入初始值</param>
/// <param name="refin">输入是否反转</param>
/// <param name="refout">输出是否反转</param>
/// <param name="xorout">输出异或值</param>
/// <returns>CRC校验值</returns>
template<typename T> T calculateCrc(
    unsigned char* pData,
    size_t length,
    size_t width,
    T poly,
    T init,
    bool refin,
    bool refout,
    T xorout)
{
    //计算掩码
    T mask = 0;
    for (size_t i = 0; i < width; i++)
    {
        mask = (mask << 1) | 1;
    }

    T table[256];
    calculateCrcTable<T>(table, width, poly, refin, refout);

    T ret = init;
    while (length-- > 0)
    {
        if (refin)
        {
            ret = (ret >> 8) ^ table[(ret ^ *pData++) & 0xff];
        }
        else
        {
            ret = ((ret << 8) & mask) ^ table[(ret >> (width - 8)) ^ *pData++];
        }
    }
    return ret ^ xorout;
}