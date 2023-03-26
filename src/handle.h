#pragma once
#include <string>
#include <filesystem>
#include <vector>
#include <iconv.h>
#include <fstream>
#include "cmdline.h"
namespace whh
{

	namespace fs = std::filesystem;
	class handle
	{

		using string = std::string;
		using arr = std::vector<int>;
		using bdp = std::vector<arr>;

	public:
		enum class ErrorCode : unsigned char
		{
			SUCCESS = 0,		// 没有错误
			FILE_NOT_EXIST,		// 文件不存在
			NOT_DIR,			// 不是目录
			NOT_SUPPORT_ENCODE, // 不支持的编码
			OPTION_CONFLICT,	// 选项冲突
		};

	private:
		fs::path dir;
		string name;
		string from, to;
		bool need_conveter; // 需要转换编码
		bool to_utf8, utf8_bom, up, has_up;
		int match_string(const std::string &m_str);
		ErrorCode encode(const std::filesystem::directory_entry &) const;
		void rename(const std::filesystem::path &) const;

	public:
		handle(int, char *[]);
		string getDir() const
		{
			return dir.string();
		}
		ErrorCode operator()();
		~handle();
	};
};
