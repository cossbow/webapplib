/// \file waString.h
/// String类头文件
/// 继承自std::string的字符串类
/// <a href=std_string.html>基类std::string使用说明文档</a>

#ifndef _WEBAPPLIB_STRING_H_
#define _WEBAPPLIB_STRING_H_ 

#include <sys/stat.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>

using namespace std;

// WEB Application Library namaspace
namespace webapp {
	
////////////////////////////////////////////////////////////////////////////////	
// 空白字符列表
const char BLANK_CHARS[] = " \t\n\r\v\f";
// CGI敏感字符列表
const char CGI_SENSITIVE[] = "&;`'\\\"|*?~<>^()[]{}$\n\r\t\0#./%";

////////////////////////////////////////////////////////////////////////////////
/// long int转换为string
string itos( const long i, const ios::fmtflags base = ios::dec );
/// string转换为int
long stoi( const string &s, const ios::fmtflags base = ios::dec );
/// double转换为string
string ftos( const double f, const int ndigit = 2 );
/// string转换为double
double stof( const string &s );
/// 判断一个双字节字符是否是GBK编码汉字
bool isgbk( const unsigned char c1, const unsigned char c2 );
/// 可变参数字符串格式化，与va_start()、va_end()宏配合使用
string va_sprintf( va_list ap, const string &format );
/// 格式化字符串并返回
string va_str( const char *format, ... );

////////////////////////////////////////////////////////////////////////////////
/// 继承自std::string的字符串类
/// <a href="std_string.html">基类std::string使用说明文档</a>
class String : public string {
	public:
	
	////////////////////////////////////////////////////////////////////////////
	/// 默认构造函数
	String(){}
	
	/// 参数为char*的构造函数
	String( const char *s ) {
		if( s ) this->assign( s );
		else this->erase();
	}
	
	/// 参数为string的构造函数
	String( const string &s ) {
		this->assign( s );
	}
	
	/// 析构函数
	virtual ~String(){}
	
	////////////////////////////////////////////////////////////////////////////
	/// \enum 函数String::split()分割结果返回方式
	enum split_resmode {
		/// 忽略分割结果中的空子串
		SPLIT_ERASE_BLANK,
		/// 保留分割结果中的空子串
		SPLIT_KEEP_BLANK
	};	
	/// \enum 函数String::split()切分标记方式
	enum split_splmode {
		/// 将切分标记字符串作为完整切分依据
		SPLIT_SINGLE,
		/// 将切分标记字符串作为切分依据序列
		SPLIT_MULTI
	};
	/// \enum 函数String::escape_html()转换方式
	enum eschtml_mode {
		/// HTML代码转换为转义符
		ESCAPE_HTML,		
		/// 转义符转换为HTML代码
		UNESCAPE_HTML			
	};	

	////////////////////////////////////////////////////////////////////////////
	/// 返回 char* 型结果，调用者必须调用 delete[] 释放所返回内存
	char* c_char() const;
	
	/// 返回字符数量，支持全角字符
	string::size_type w_length() const;
	/// 截取子字符串，支持全角字符
	String w_substr( const string::size_type pos = 0, 
					 const string::size_type n = npos ) const;

	/// 清除左侧空白字符
	void trim_left( const string &blank = BLANK_CHARS );
	/// 清除右侧空白字符
	void trim_right( const string &blank = BLANK_CHARS );
	/// 清除两侧空白字符
	void trim( const string &blank = BLANK_CHARS );

	/// 从左边截取指定长度子串
	String left( const string::size_type n ) const;
	/// 从中间截取指定长度子串
	String mid( const string::size_type pos, 
				const string::size_type n = npos ) const;
	/// 从右边截取指定长度子串
	String right( const string::size_type n ) const;
	
	/// 调整字符串长度
	void resize( const string::size_type n );

	/// 统计指定子串出现的次数
	int count( const string &str ) const;
	
	/// 根据分割符分割字符串
	vector<String> split( const string &tag, 
						  const int limit = 0, 
						  const split_resmode ret = SPLIT_ERASE_BLANK,
						  const split_splmode spl = SPLIT_SINGLE ) const;
	
	/// 转换字符串为HASH结构(map<string,string>)
	map<string,string> tohash( const string &itemtag = "&", 
							   const string &exptag = "=" ) const;

	/// 组合字符串
	void join( const vector<string> &strings, const string &tag );
	void join( const vector<String> &strings, const string &tag );

	/// 格式化赋值
	bool sprintf( const char *format, ... );
	
	/// 替换
	int replace( const string &oldstr, const string &newstr );
	/// 替换
	inline int replace( const string &oldstr, const long &i ) {
		return this->replace( oldstr, itos(i) );
	}
	
	/// 全文替换
	int replace_all( const string &oldstr, const string &newstr );
	/// 全文替换
	inline int replace_all( const string &oldstr, const long &i ) {
		return this->replace_all( oldstr, itos(i) );
	}
	
	/// 转换为大写字母
	void upper();
	/// 转换为小写字母
	void lower();
	
	/// 字符串是否完全由数字组成
	bool isnum() const;
	
	/// 读取文件到字符串
	bool load_file( const string &filename );
	/// 保存字符串到文件
	bool save_file( const string &filename, 
					const ios::openmode mode = ios::trunc|ios::out,
					const mode_t permission = S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH ) const;
	
	/// 过滤字符
	void filter( const string &filter = CGI_SENSITIVE, 
				 const string &newstr = "" );
	
	/// HTML代码转义
	string escape_html( const eschtml_mode mode );
	
	/// 将字符串拆分为字符，支持全角字符
	vector<string> split_char() const;
};

} // namespace

#endif //_WEBAPPLIB_STRING_H_

