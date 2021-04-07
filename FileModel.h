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
    /// <param name="padding">是否填充</param>
    /// <param name="paddingValue">填充值</param>
    void generateBin(const char* path, const bool padding, unsigned char paddingValue);
    /// <summary>
    /// 解析hex文件
    /// </summary>
    /// <param name="path">hex文件路径</param>
    void parseHex(const char* path);
    /// <summary>
    /// 生成hex文件
    /// </summary>
    /// <param name="path">hex文件路径</param>
    /// <param name="padding">是否填充</param>
    /// <param name="paddingValue">填充值</param>
    void generateHex(const char* path, const bool padding, unsigned char paddingValue);
    /// <summary>
    /// 进行段间填充
    /// </summary>
    /// <param name="paddingValue">填充值</param>
    void padding(unsigned char paddingValue);
};
