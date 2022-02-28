#include "handle.h"
namespace whh
{
	handle::handle(int argc, char *argv[])
	{
		cmdline::parser par;
		par.add<std::string>("dir", 'd', "需要处理的目录路径", false);
		par.add<std::string>("name", 'n', "匹配文件名的通配符表达式", false);
		par.add<std::string>("from", 'f', "源编码格式", false);
		par.add<std::string>("to", 't', "目标编码格式", false);
		par.add<bool>("bom", 'b', "UTF-8 with BOM", false, false);
		par.add<bool>("up", 'u', "文件路径转大写", false, false);
		par.parse_check(argc, argv);
		auto &d = par.get<std::string>("dir");
		from = par.get<std::string>("from"), to = par.get<std::string>("to");
		transform(from.begin(), from.end(), from.begin(), ::toupper);
		transform(to.begin(), to.end(), to.begin(), ::toupper);
		if (d.empty())
		{
			dir = fs::current_path();
		}
		else
		{
			dir = d;
		}
		name = par.get<std::string>("name");
		to_utf8 = need_conveter = false; //默认值

		if (!to.empty() && !from.empty())
		{
			need_conveter = true;
			if (to == "UTF-8")
			{
				to_utf8 = true;
			}
		}
		utf8_bom = par.get<bool>("bom");
		up = par.get<bool>("up");
		has_up = par.exist("up");
		// std::cout << std::boolalpha << utf8_bom << to_utf8 << std::endl;
	}
	int handle::match_string(const std::string &m_str) // match wildcard 通配符
	{
		if (name.empty())
		{
			return 1;
		}
		int m_len = m_str.size();
		int w_len = name.size();
		bdp b_dp(w_len + 1, arr(m_len + 1, 0));
		//多加一行一列作为初始初值所用
		b_dp[0][0] = 1;
		for (int i = 1; i <= w_len; i++)
		{
			char ch = name[i - 1];
			//设置每次循环的初值，即当星号不出现在首位时，匹配字符串的初值都为false
			b_dp[i][0] = b_dp[i - 1][0] && (ch == '*');
			for (int j = 1; j <= m_len; j++)
			{
				char ch2 = m_str[j - 1];
				if (ch == '*')
					b_dp[i][j] = b_dp[i - 1][j] || b_dp[i][j - 1]; //当匹配字符为*号时，状态取决于上面状态和左边状态的值
				else
					b_dp[i][j] = b_dp[i - 1][j - 1] && (ch == '?' || ch2 == ch); //决定于上一次和本次
			}
		}
		return b_dp[w_len][m_len];
	}
	handle::ErrorCode handle::encode(const std::filesystem::directory_entry &entry) const
	{
		std::cout << "正在转码:" << entry << ' ' << from << "->" << to;
		std::ifstream in(entry.path());
		std::ostringstream tmp;
		tmp << in.rdbuf();
		in.close();
		std::string input = tmp.str();
		size_t charInPutLen = input.length();
		if (charInPutLen == 0)
		{
			std::cout << "空文件" << std::endl;
			return ErrorCode::SUCCESS;
		}

		size_t charOutPutLen = 2 * charInPutLen + 1; //输出内容大小
		char *pTemp = new char[charOutPutLen];		 //输出内容指针临时变量
		memset(pTemp, 0, charOutPutLen);
		char *pSource = (char *)input.c_str(); //源内容指针
		char *pOut = pTemp;					   //输出内容指针
		auto conveter = iconv_open(to.c_str(), from.c_str());
		if (conveter == (iconv_t)-1)
		{
			return ErrorCode::NOT_SUPPORT_ENCODE;
		}
		iconv(conveter, &pSource, &charInPutLen, &pTemp, &charOutPutLen);
		iconv_close(conveter);
		string output = pOut;
		delete[] pOut; //注意这里，不能使用delete []pTemp, iconv函数会改变指针pTemp的值

		std::ofstream ofs(entry.path()); //文件是utf8编码
		if (to_utf8 && utf8_bom)
		{ //需要带上bom
			std::cout << " with BOM";
			ofs << (char)0xEF << (char)0xBB << (char)0xBF;
		}
		ofs << output;
		ofs.close();
		std::cout << " 完成" << std::endl;
		return ErrorCode::SUCCESS;
	}
	void handle::rename(const std::filesystem::path &path) const
	{
		if (!fs::exists(path))
		{
			return;
		}
		auto fun = up ? (::toupper) : (::tolower);
		auto dir = path.parent_path();			   //父级目录
		auto file_name = path.filename().string(); //文件名
		transform(file_name.begin(), file_name.end(), file_name.begin(), fun);
		auto new_path = path; //新路径
		new_path.replace_filename(file_name);
		if (fs::is_directory(path))
		{
			for (auto &entry : fs::directory_iterator(path))
			{
				rename(entry.path()); //递归
			}
		}
		if (path == new_path)
		{
			return;
		}
		fs::rename(path, new_path);
		std::cout << path << " -> " << new_path << std::endl;
	}
	handle::ErrorCode handle::operator()()
	{

		if (!fs::exists(dir))
		{
			return ErrorCode::FILE_NOT_EXIST;
		}
		if (!fs::is_directory(dir))
		{
			return ErrorCode::NOT_DIR;
		}
		//需要转换文件名大小写
		if (has_up)
		{
			//需要排除自己
			for (auto &entry : fs::directory_iterator(dir))
			{
				rename(entry.path());
			}
		}

		for (auto &entry : fs::recursive_directory_iterator(dir))
		{
			if (!fs::is_regular_file(entry))
			{ //不是普通文件
				continue;
			}
			if (match_string(entry.path().filename().string()) != 1)
			{ //文件名不匹配
				continue;
			}
			if (need_conveter)
			{
				ErrorCode r = encode(entry); //重新编码
				if (r != ErrorCode::SUCCESS)
				{
					return r;
				}
			}
		}
		return ErrorCode::SUCCESS;
	}

	handle::~handle()
	{
	}
} // namespace whh
