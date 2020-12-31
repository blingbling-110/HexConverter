#pragma once

#include <map>
#include "Segment.h"
#include <fstream>
#include <ios>

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
	bool append(Segment segment);
	/// <summary>
	/// 解析bin文件
	/// </summary>
	/// <param name="path">bin文件路径</param>
    /// <param name="startAddress">起始地址</param>
    void parseBin(const char* path, size_t startAddress);
};
