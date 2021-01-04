#pragma once
#include <vector>

/// <summary>
/// 刷新段
/// </summary>
class Segment
{
	/// <summary>
	/// 段的起始地址
	/// </summary>
    size_t startAddr;
    /// <summary>
    /// 段中的数据
    /// </summary>
    std::vector<unsigned char> data;
	/// <summary>
	/// 段的CRC校验值
	/// </summary>
	unsigned long long crc;

public:
	/// <summary>
	/// 设置段的起始地址
	/// </summary>
	/// <param name="startAddr">段的起始地址</param>
	void setStartAddress(size_t startAddr);
	/// <summary>
	/// 获取段的起始地址
	/// </summary>
	/// <returns>段的起始地址</returns>
	size_t getStartAddress();
	/// <summary>
	/// 将数据写入段中
	/// </summary>
    /// <param name="buffer">待写入的缓冲</param>
    /// <param name="length">待写入缓冲的大小</param>
    void append(unsigned char* buffer, size_t length);
    /// <summary>
    /// 将单个字节写入段中
    /// </summary>
    /// <param name="oneByte">待写入的单个字节</param>
    void append(unsigned char oneByte);
	/// <summary>
	/// 获取段中数据长度
	/// </summary>
	/// <returns>段中数据的字节数</returns>
	size_t getLength();
	/// <summary>
	/// 获取段中数据的头指针
	/// </summary>
	/// <returns>段中数据的头指针，指向第一个字节</returns>
	unsigned char* getFrontPointer();
	/// <summary>
	/// 计算段的CRC校验值
	/// </summary>
	/// <param name="width">CRC位宽</param>
	/// <param name="poly">CRC多项式</param>
	/// <param name="init">输入初始值</param>
	/// <param name="refin">输入是否反转</param>
	/// <param name="refout">输出是否反转</param>
	/// <param name="xorout">输出异或值</param>
	void calculateSegmentCrc(
		rsize_t width,
		unsigned long long poly,
		unsigned long long init,
		bool refin,
		bool refout,
		unsigned long long xorout);
	/// <summary>
	/// 获取段的CRC校验值。注意要先调用calculateSegmentCrc进行计算
	/// </summary>
	/// <returns>段的CRC校验值</returns>
	unsigned long long getSegmentCrc();
};
