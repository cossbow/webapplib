/// \file waTemplate.h
/// HTML模板处理类头文件
/// 支持条件、循环脚本的HTML模板处理类
/// 依赖于 waString
/// <a href="wa_template.html">使用说明文档及简单范例</a>

#ifndef _WEBAPPLIB_TEMPLATE_H_
#define _WEBAPPLIB_TEMPLATE_H_ 

#include <cstring>
#include <string>
#include <vector>
#include <map>
#include "waString.h"

using namespace std;

// WEB Application Library namaspace
namespace webapp {
	
/// 支持条件、循环脚本的HTML模板处理类
/// <a href="wa_template.html">使用说明文档及简单范例</a>
class Template {
	public:
	
	/// 默认构造函数
	Template(){};
	
	/// 构造函数
	/// \param tmpl_file 模板文件
	Template( const string tmpl_file ) {
		this->load( tmpl_file );
	}
	
	/// 构造函数
	/// \param tmpl_dir 模板目录
	/// \param tmpl_file 模板文件
	Template( const string tmpl_dir, const string tmpl_file ) {
		this->load( tmpl_dir, tmpl_file );
	}
	
	/// 析构函数
	virtual ~Template(){};
	
	/// \enum 函数Htt::sort_table()排序方式
	enum sort_mode {
		/// 升序
		TEMPLATE_SORT_ASCE,	
		/// 降序
		TEMPLATE_SORT_DESC	
	};

	/// \enum 函数Htt::sort_table()排序比较方式
	enum cmp_mode {
		/// 字符串比较
		TEMPLATE_CMP_STR,	
		/// 整数比较
		TEMPLATE_CMP_INT	
	};

	/// \enum 输出时是否包括调试信息
	enum output_mode {
		/// 显示调试信息
		TEMPLATE_OUTPUT_DEBUG,	
		/// 不显示
		TEMPLATE_OUTPUT_RELEASE	
	};

	/// 读取HTML模板文件
	bool load( const string &tmpl_file );

	/// 读取模板
	/// \param tmpl_dir 模板目录
	/// \param tmpl_file 模板文件
	/// \retval true 读取成功
	/// \retval false 失败
	inline bool load( const string &tmpl_dir, const string &tmpl_file ) {
		return this->load( tmpl_dir + "/" + tmpl_file );
	}
	
	/// 设置HTML模板内容
	void tmpl( const string &tmpl );

	/// 设置替换规则
	void set( const string &name, const string &value );
	/// 设置替换规则
	/// \param name 模板域名称
	/// \param value 替换值
	inline void set( const string &name, const long value ) {
		this->set( name, itos(value) );
	}
	/// 取消替换规则
	void unset( const string &name );
	
	/// 新建表格
	void table( const string &table, const char* field_0, ... );
	/// 取消表格
	void unset_table( const string &table );
	/// 添加一行数据到表格
	void set_row( const string &table, const char* value_0, ... );
	/// 添加一行指定格式的数据到表格
	void format_row( const string &table, const char* format, ... );
	/// 取消表格指定行
	void unset_row( const string &table, const int row );
	/// 设置表格指定位置值
	void set( const string &table, const int row, 
			  const string &field, const string &value );
	/// 表格行排序
	void sort_table( const string &table, const string &field, 
					 const sort_mode mode = TEMPLATE_SORT_ASCE,
					 const cmp_mode cmp = TEMPLATE_CMP_STR );
	
	/// 清空所有替换规则
	void clear_set();

	/// 返回HTML字符串
	string html();
	/// 输出HTML到stdout
	void print( const output_mode mode = TEMPLATE_OUTPUT_RELEASE );
	/// 输出HTML到文件
	bool print( const string &file, const output_mode mode = TEMPLATE_OUTPUT_RELEASE,
				const mode_t permission = S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH );
	
	////////////////////////////////////////////////////////////////////////////
	private:
	
	/// 读取指定位置的模板脚本类型及表达式
	int parse_script( const string &tmpl, const unsigned int pos,
					  string &exp, int &type );

	/// 分析表达式的值
	string exp_value( const string &expression );

	/// 分析处理模板
	void parse( const string &tmpl, ostream &output );
	
	/// 检查条件语句表达式是否成立
	bool compare( const string &exp );
	
	/// 检查条件是否成立
	bool check_if( const string &exp );

	/// 处理条件类型模板
	size_t parse_if( const string &tmpl, ostream &output, 
					 const bool parent_state, const string &parsed_exp,
					 const int parsed_length );

	/// 返回字段位置
	int field_pos( const string &table, const string &field );

	/// 检查循环语句是否有效
	bool check_for( const string &tablename );
	
	/// 返回表格中指定位置字段的值
	string table_value( const string &field );

	/// 处理循环类型模板
	size_t parse_for( const string &tmpl, ostream &output, 
					  const bool parent_state, const string &parsed_exp,
					  const int parsed_length );
							
	/// 模板分析错误纪录
	void error_log( const size_t lines, const string &error );
	/// 模板分析纪录
	void parse_log( ostream &output );

	// 数据定义
	typedef vector<string> strings;		// 字符串列表
	typedef struct {					// 表格模板设置结构
		int cols;						// 表格字段数量
		int rows;						// 表格数据行数
		int cursor;						// 当前光标位置
		strings fields;					// 表格字段定义列表
		map<string,int> fieldspos;		// 表格字段位置,for speed
		vector<strings> datas;			// 表格数据
	} template_table;

	// 模板数据
	String _tmpl;						// HTML模板内容
	map<string,string> _sets;			// 替换规则列表 <模板域名称,模板域值>
	map<string,template_table> _tables;	// 表格替换规则列表 <表格名称,表格模板设置结构>
	
	// 分析过程数据
	string _table;						// 当前表格名称
	int _cursor;						// 当前表格光标位置
	int _lines;							// 已处理模板行数

	string _tmplfile;					// HTML模板文件名
	char _date[15];						// 当前日期
	char _time[15];						// 当前时间
	output_mode _debug;					// 分析模式
	multimap<int,string> _errlog;		// 分析错误纪录 <错误位置行数,错误描述信息>
};

// 模板语法格式定义
const char TEMPLATE_BEGIN[]		= "{{";		const int TEMPLATE_BEGIN_LEN 	= strlen(TEMPLATE_BEGIN);
const char TEMPLATE_END[]		= "}}";		const int TEMPLATE_END_LEN 		= strlen(TEMPLATE_END);
const char TEMPLATE_SUBBEGIN[]	= "(";		const int TEMPLATE_SUBBEGIN_LEN = strlen(TEMPLATE_SUBBEGIN);
const char TEMPLATE_SUBEND[]	= ")";		const int TEMPLATE_SUBEND_LEN 	= strlen(TEMPLATE_SUBEND);
const char TEMPLATE_SPLIT[]		= ",";		const int TEMPLATE_SPLIT_LEN 	= strlen(TEMPLATE_SPLIT);
const char TEMPLATE_NEWLINE[]	= "\n";		const int TEMPLATE_NEWLINE_LEN 	= strlen(TEMPLATE_NEWLINE);

const char TEMPLATE_VALUE[]		= "$";		const int TEMPLATE_VALUE_LEN 	= strlen(TEMPLATE_VALUE);
const char TEMPLATE_DATE[]		= "%DATE";	const int TEMPLATE_DATE_LEN 	= strlen(TEMPLATE_DATE);
const char TEMPLATE_TIME[]		= "%TIME";	const int TEMPLATE_TIME_LEN 	= strlen(TEMPLATE_TIME);
const char TEMPLATE_SPACE[]		= "%SPACE";	const int TEMPLATE_SPACE_LEN	= strlen(TEMPLATE_SPACE);
const char TEMPLATE_BLANK[]		= "%BLANK";	const int TEMPLATE_BLANK_LEN 	= strlen(TEMPLATE_BLANK);
                                        
const char TEMPLATE_FOR[]		= "#FOR";	const int TEMPLATE_FOR_LEN 		= strlen(TEMPLATE_FOR);
const char TEMPLATE_ENDFOR[]	= "#ENDFOR";const int TEMPLATE_ENDFOR_LEN 	= strlen(TEMPLATE_ENDFOR);
const char TEMPLATE_TBLVALUE[]	= ".$";		const int TEMPLATE_TBLVALUE_LEN = strlen(TEMPLATE_TBLVALUE);
const char TEMPLATE_TBLSCOPE[]	= "@";		const int TEMPLATE_TBLSCOPE_LEN = strlen(TEMPLATE_TBLSCOPE);
const char TEMPLATE_CURSOR[]	= "%CURSOR";const int TEMPLATE_CURSOR_LEN	= strlen(TEMPLATE_CURSOR);
const char TEMPLATE_ROWS[]		= "%ROWS";	const int TEMPLATE_ROWS_LEN		= strlen(TEMPLATE_ROWS);

const char TEMPLATE_IF[]		= "#IF";	const int TEMPLATE_IF_LEN 		= strlen(TEMPLATE_IF);
const char TEMPLATE_ELSIF[]		= "#ELSIF";	const int TEMPLATE_ELSIF_LEN 	= strlen(TEMPLATE_ELSIF);
const char TEMPLATE_ELSE[]		= "#ELSE";	const int TEMPLATE_ELSE_LEN 	= strlen(TEMPLATE_ELSE);
const char TEMPLATE_ENDIF[]		= "#ENDIF";	const int TEMPLATE_ENDIF_LEN 	= strlen(TEMPLATE_ENDIF);

// 比较操作符定义
const char TEMPLATE_AND[]		= "AND";	const int TEMPLATE_AND_LEN 		= strlen(TEMPLATE_AND);
const char TEMPLATE_OR[]		= "OR";		const int TEMPLATE_OR_LEN 		= strlen(TEMPLATE_OR);
const char TEMPLATE_EQ[]		= "==";		const int TEMPLATE_EQ_LEN 		= strlen(TEMPLATE_EQ);
const char TEMPLATE_NE[]		= "!=";		const int TEMPLATE_NE_LEN 		= strlen(TEMPLATE_NE);
const char TEMPLATE_LE[]		= "<=";		const int TEMPLATE_LE_LEN 		= strlen(TEMPLATE_LE);
const char TEMPLATE_LT[]		= "<";		const int TEMPLATE_LT_LEN 		= strlen(TEMPLATE_LT);
const char TEMPLATE_GE[]		= ">=";		const int TEMPLATE_GE_LEN 		= strlen(TEMPLATE_GE);
const char TEMPLATE_GT[]		= ">";		const int TEMPLATE_GT_LEN 		= strlen(TEMPLATE_GT);

// Htt::format_row()格式定义
const char TEMPLATE_FMTSTR[]	= "%s";		const int TEMPLATE_FMTSTR_LEN 	= strlen(TEMPLATE_FMTSTR);
const char TEMPLATE_FMTINT[]	= "%d";		const int TEMPLATE_FMTINT_LEN 	= strlen(TEMPLATE_FMTINT);

// 脚本语句类型
enum template_scripttype {
	TEMPLATE_S_VALUE,	
	TEMPLATE_S_TBLVALUE,	
	TEMPLATE_S_FOR,	
	TEMPLATE_S_ENDFOR,	
	TEMPLATE_S_IF,		
	TEMPLATE_S_ELSIF,	
	TEMPLATE_S_ELSE,		
	TEMPLATE_S_ENDIF,
	TEMPLATE_S_CURSOR,
	TEMPLATE_S_ROWS,
	TEMPLATE_S_DATE,
	TEMPLATE_S_TIME,
	TEMPLATE_S_SPACE,
	TEMPLATE_S_BLANK,
	TEMPLATE_S_UNKNOWN
};

// 逻辑运算类型
enum template_logictype {
	TEMPLATE_L_NONE,	
	TEMPLATE_L_AND,
	TEMPLATE_L_OR
};

// 比较运算类型
enum template_cmptype {
	TEMPLATE_C_EQ,
	TEMPLATE_C_NE,
	TEMPLATE_C_LE,
	TEMPLATE_C_LT,
	TEMPLATE_C_GE,
	TEMPLATE_C_GT
};

} // namespace

#endif //_WEBAPPLIB_TEMPLATE_H_

