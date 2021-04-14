#pragma once

#include <map>
#include "Segment.h"
#include <fstream>
#include <ios>
#include <string>
#include <iomanip>
#include <sstream>

using namespace std;

#define READ_EACH_TIME 1024

class FileModel
{
public:
	/// <summary>
	/// 文件中所有刷新段的起始地址与刷新段的映射
	/// </summary>
	std::map<size_t, Segment> segments;
	/// <summary>
	/// 新增一个刷新段
	/// </summary>
	/// <param name="segment">待增加的刷新段</param>
	/// <returns>是否增加成功</returns>
    void append(Segment segment);
	/// <summary>
	/// 解析bin文件
	/// </summary>
	/// <param name="path">bin文件路径</param>
    /// <param name="startAddr">起始地址</param>
    void parseBin(const char* path, size_t startAddr);
    /// <summary>
    /// 生成bin文件
    /// </summary>
    /// <param name="path">bin文件路径</param>
    /// <returns>生成成功与否</returns>
    bool generateBin(const char* path);
    /// <summary>
    /// 解析hex文件
    /// </summary>
    /// <param name="path">hex文件路径</param>
    void parseHex(const char* path);
    /// <summary>
    /// 生成hex文件
    /// </summary>
    /// <param name="path">hex文件路径</param>
    /// <returns>生成成功与否</returns>
    bool generateHex(const char* path);
    /// <summary>
    /// 进行段间填充
    /// </summary>
    /// <param name="paddingValue">填充值</param>
    void padding(unsigned char paddingValue);
    /// <summary>
    /// 对所有段进行过滤
    /// </summary>
    /// <param name="startAddr">过滤起始地址</param>
    /// <param name="endAddr">过滤结束地址</param>
    /// <param name="padding">是否填充</param>
    /// <param name="paddingValue">填充值</param>
    void filter(size_t startAddr, size_t endAddr, bool padding, unsigned char paddingValue);
    /// <summary>
    /// 为各段计算并添加CRC校验值
    /// </summary>
    /// <param name="width">CRC位宽</param>
    /// <param name="poly">CRC多项式</param>
    /// <param name="init">输入初始值</param>
    /// <param name="refin">输入是否反转</param>
    /// <param name="refout">输出是否反转</param>
    /// <param name="xorout">输出异或值</param>
    void addCrc(
            rsize_t width,
            unsigned long long poly,
            unsigned long long init,
            bool refin,
            bool refout,
            unsigned long long xorout);
};
