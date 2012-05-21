/// \file waTemplate.cpp
/// HTML模板处理类实现文件

#include <cstdio>
#include <cstdarg>
#include <ctime>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iterator>
#include <algorithm>
#include "waTemplate.h"

using namespace std;

// WEB Application Library namaspace
namespace webapp {
	
////////////////////////////////////////////////////////////////////////////
// set functions

/// 读取HTML模板文件
/// \param tmpl_file 模板路径文件名
/// \retval true 读取成功
/// \retval false 失败
bool Template::load( const string &tmpl_file ) {
	if ( _tmpl.load_file(tmpl_file) ) {
		_tmplfile = tmpl_file;
		return true;
	} else {
		_tmplfile = "Error: Can't open file " + tmpl_file;
		this->error_log( 0, _tmplfile );
		return false;
	}
}

/// 设置HTML模板内容
/// \param tmpl 模板内容字符串
void Template::tmpl( const string &tmpl ) {
	_tmplfile = "Read from string";
	_tmpl = tmpl;
}

/// 设置替换规则
/// \param name 模板域名称
/// \param value 替换值
void Template::set( const string &name, const string &value ) {
	if ( name != "" )
		_sets[name] = value;
}

/// 取消替换规则
/// \param name 模板域名称
void Template::unset( const string &name ) {
	if ( name != "" )
		_sets.erase( name );
}

/// 新建表格
/// \param table 表格名称
/// \param field_0 field_0及field_0之后为字段名称列表,最后一个参数必须是NULL
/// \param ... 字段名称列表,最后一个参数必须是NULL
void Template::table( const string &table, const char* field_0, ... ) {
	va_list ap;
	const char *p;
	string field;
	strings fields;
	size_t cols = 0;
	
	// check same name table
	if ( _tables.find(table) != _tables.end() )
		this->error_log( 0, "Warning: table name \""+table+"\" redefined" );
	
	// get fields
	va_start( ap, field_0 );
	for ( p=field_0; p; p=va_arg(ap,const char*) ) {
		if ( (field=p) != "" ) {
			fields.push_back( field );
			_tables[table].fieldspos[field] = cols; // for speed
			++cols;
		}
	}
	va_end( ap );

	// for waTemplate old version templet script ( < v0.7 )
	if ( _sets.find(table) == _sets.end() )
		set( table, table );

	// init table
	_tables[table].fields = fields;
	for ( size_t i=0; i<_tables[table].datas.size(); ++i )
		_tables[table].datas[i].clear();
	_tables[table].datas.clear();
	_tables[table].cursor = 0;
	_tables[table].rows = 0;
	_tables[table].cols = cols;
}

/// 取消表格
/// 执行成功后将删除该表格所有设置数据
/// \param table 表格名称
void Template::unset_table( const string &table ) {
	if ( table != "" )
		_tables.erase( table );
}

/// 添加一行数据到表格
/// 必须先调用Template::table()初始化表格字段定义,否则中止
/// \param table 表格名称
/// \param value_0 value_0及value_0之后为字段名称列表,最后一个参数必须是NULL
/// \param ... 字段名称列表,最后一个参数必须是NULL
void Template::set_row( const string &table, const char* value_0, ... ) {
	// table must exist
	if ( _tables.find(table) == _tables.end() ) {
		this->error_log( 0, "Error: Call table() to init $"+table+" first, in set_row()" );
		return;
	}
	
	// get values
	va_list ap;
	const char *p;
	string value;
	strings values;
	int cols = 0;
	
	va_start( ap, value_0 );
	for ( p=value_0; p; p=va_arg(ap,const char*) ) {
		value = p;
		values.push_back( value );
		++cols;
		
		// enough now
		if ( cols >= _tables[table].cols )
			break;
	}
	va_end( ap );

	// fill blank if not enough
	if( cols < _tables[table].cols ) {
		for ( int i=cols; i<_tables[table].cols; ++i )
			values.push_back( "" );
	}
	
	// insert into table
	_tables[table].datas.push_back( values );
	++_tables[table].rows;
}

/// 添加一行指定格式的数据到表格
/// 必须先调用Template::table()初始化表格字段定义,否则中止
/// \param table 表格名称
/// \param format 字段列表格式定义,"%d,%s,..."格式
/// \param ... 第三个参数起为字段值列表,
/// 字段值参数个数不能少于格式定义参数format中指定的个数
void Template::format_row( const string &table, const char* format, ... ) {
	// table must exist
	if ( _tables.find(table) == _tables.end() ) {
		this->error_log( 0, "Error: Call table() to init $"+table+" first, in format_row()" );
		return;
	}
	
	// split format string
	String fmtstr = format;
	vector<String> fmtlist = fmtstr.split( TEMPLATE_SPLIT );
	
	// get values
	va_list ap;
	string value;
	strings values;
	int cols = 0;
	
	va_start( ap, format );
	for ( size_t i=0; i<fmtlist.size(); ++i ) {
		// read
		fmtlist[i].trim();
		if ( fmtlist[i] == TEMPLATE_FMTSTR )
			value = va_arg( ap, const char* ); // %s
		else
			value = itos( va_arg(ap,long) ); // %d or other
			
		// push data
		values.push_back( value );
		++cols;

		// enough now
		if ( cols >= _tables[table].cols )
			break;
	}
	va_end( ap );

	// fill blank if not enough
	if( cols < _tables[table].cols ) {
		for ( int i=cols; i<_tables[table].cols; ++i )
			values.push_back( "" );
	}
	
	// insert into table
	_tables[table].datas.push_back( values );
	++_tables[table].rows;
}

/// 设置表格指定位置值
/// \param table 表格名称
/// \param row 位置,即第几行
/// \param field 字段名称,即第几列
/// \param value 要设置为的值
void Template::set( const string &table, const int row, 
			   const string &field, const string &value ) {
	// table must exist
	if ( _tables.find(table) == _tables.end() ) {
		this->error_log( 0, "Error: Call table() to init $"+table+" first, in set()" );
		return;
	}

	// row out of range
	if ( row > _tables[table].rows ) {
		this->error_log( 0, "Error: $"+table+" row out of range, in set()" );
		return;
	}

	int col = this->field_pos( table, field );
	if ( col != -1 )
		_tables[table].datas[row][col] = value;
}

/// 取消表格指定行
/// \param table 表格名称
/// \param row 行位置
void Template::unset_row( const string &table, const int row ) {
	// table must exist
	if ( _tables.find(table) == _tables.end() ) {
		this->error_log( 0, "Error: Call table() to init $"+table+" first, in unset_row()" );
		return;
	}
	
	if ( row <= _tables[table].rows ) {
		// delete row
		vector<strings>::iterator pos = _tables[table].datas.begin();
		std::advance( pos, row-1 );
		_tables[table].datas.erase( pos );
		--_tables[table].rows;
	}
}

/// 表格行排序
/// \param table 表格名称
/// \param field 排序字段
/// \param mode 排序模式
/// - Template::TEMPLATE_SORT_ASCE 升序
/// - Template::TEMPLATE_SORT_DESC 降序
/// - 默认为升序
/// \param cmp 排序比较模式,
/// 可以为字符串比较(Template::TEMPLATE_CMP_STR)或者整数比较(Template::TEMPLATE_CMP_INT),默认为字符串比较
void Template::sort_table( const string &table, const string &field, 
					  const sort_mode mode, const cmp_mode cmp ) {
	// table must exist
	if ( _tables.find(table) == _tables.end() ) {
		this->error_log( 0, "Error: Call table() to init $"+table+
					  	 " first, in sort_table()" );
		return;
	}

	// get field position
	int col = this->field_pos( table, field );
	if ( col == -1 ) {
		this->error_log( 0, "Warning: field .$"+field+" not defined in tabel \""+
					  		table+"\", in sort_table()" );
		return;
	}

	// sort
	bool swaped;
	bool needswap;
	string curr, next;
	int curr_int, next_int;
	
	do {
		swaped = false;
		for ( int i=0; i<_tables[table].rows-1; ++i ) {
			curr = _tables[table].datas[i][col];
			next = _tables[table].datas[i+1][col];

			if ( cmp == TEMPLATE_CMP_STR ) {
				// string compare
				if ( mode == TEMPLATE_SORT_ASCE )
					needswap = ( curr>next ) ? true : false;
				else
					needswap = ( curr<next ) ? true : false;
			} else {
				// integer compare
				curr_int = stoi( curr );
				next_int = stoi( next );
				
				if ( mode == TEMPLATE_SORT_ASCE )
					needswap = ( curr_int>next_int ) ? true : false;
				else
					needswap = ( curr_int<next_int ) ? true : false;
			}
				
			// swap
			if ( needswap ) {
				std::swap( _tables[table].datas[i], _tables[table].datas[i+1] );
				swaped = true;
			}
		}
	} while ( swaped == true );
}

/// 清空所有替换规则
/// 包括所有表格替换规则
void Template::clear_set() {
	_sets.clear();
	_tables.clear();
}

////////////////////////////////////////////////////////////////////////////
// parse functions

/// 返回字段位置
/// \param table 表格名称
/// \param field 字段名称
/// \return 找到返回字段位置,否则返回-1
int Template::field_pos( const string &table, const string &field ) {
	if ( _tables[table].fieldspos.find(field) == _tables[table].fieldspos.end() )
		return -1;
	return _tables[table].fieldspos[field];
}

/// 读取指定位置的模板脚本类型及表达式
/// \param tmpl 模板字符串
/// \param pos 开始分析的位置
/// \param exp 读取出的表达式字符串
/// \param type 分析出的脚本语句类型
/// \return 返回值为本次分析的字符串长度,若出错返回-1
int Template::parse_script( const string &tmpl, const size_t pos, 
					   string &exp, int &type ) {
	// find TEMPLATE_END
	size_t begin = pos + TEMPLATE_BEGIN_LEN;
	size_t end;
	if ( (end=tmpl.find(TEMPLATE_END,begin)) == tmpl.npos )
		return -1;	// can not find TEMPLATE_END

	// script type and content
	String content = tmpl.substr( begin, end-begin );
	content.trim();

	if ( strncmp(content.c_str(),TEMPLATE_VALUE,TEMPLATE_VALUE_LEN) == 0 ) {
		// simple value: $xxx
		type = TEMPLATE_S_VALUE;
		
	} else if ( strncmp(content.c_str(),TEMPLATE_TBLVALUE,TEMPLATE_TBLVALUE_LEN) == 0 ) {
		// current value in table: .$xxx
		type = TEMPLATE_S_TBLVALUE;
		
	} else if ( strncmp(content.c_str(),TEMPLATE_FOR,TEMPLATE_FOR_LEN) == 0 ) {
		// for begin: #FOR xxx
		type = TEMPLATE_S_FOR;
		content = tmpl.substr( begin+TEMPLATE_FOR_LEN, end-begin-TEMPLATE_FOR_LEN );
		content.trim();
		
	} else if ( strcmp(content.c_str(),TEMPLATE_ENDFOR) == 0 ) {
		// for end: #ENDFOR
		type = TEMPLATE_S_ENDFOR;
		
	} else if ( strncmp(content.c_str(),TEMPLATE_IF,TEMPLATE_IF_LEN) == 0 ) {
		// if begin: #IF xxx
		type = TEMPLATE_S_IF;
		content = tmpl.substr( begin+TEMPLATE_IF_LEN, end-begin-TEMPLATE_IF_LEN );
		content.trim();
	
	} else if ( strncmp(content.c_str(),TEMPLATE_ELSIF,TEMPLATE_ELSIF_LEN) == 0 ) {
		// elseif: #ELSIF xxx
		type = TEMPLATE_S_ELSIF;
		content = tmpl.substr( begin+TEMPLATE_ELSIF_LEN, end-begin-TEMPLATE_ELSIF_LEN );
		content.trim();
	
	} else if ( strcmp(content.c_str(),TEMPLATE_ELSE) == 0 ) {
		// else: #ELSE
		type = TEMPLATE_S_ELSE;
	
	} else if ( strcmp(content.c_str(),TEMPLATE_ENDIF) == 0 ) {
		// if end: #ENDIF
		type = TEMPLATE_S_ENDIF;
	
	} else if ( strncmp(content.c_str(),TEMPLATE_CURSOR,TEMPLATE_CURSOR_LEN) == 0 ) {
		// current table cursor: %CURSOR
		type = TEMPLATE_S_CURSOR;
	
	} else if ( strncmp(content.c_str(),TEMPLATE_ROWS,TEMPLATE_ROWS_LEN) == 0 ) {
		// current table cursor: %ROWS
		type = TEMPLATE_S_ROWS;

	} else if ( strcmp(content.c_str(),TEMPLATE_DATE) == 0 ) {
		// date: %DATE
		type = TEMPLATE_S_DATE;
	
	} else if ( strcmp(content.c_str(),TEMPLATE_TIME) == 0 ) {
		// time: %TIME
		type = TEMPLATE_S_TIME;
	
	} else if ( strcmp(content.c_str(),TEMPLATE_SPACE) == 0 ) {
		// space char: %SPACE
		type = TEMPLATE_S_SPACE;
	
	} else if ( strcmp(content.c_str(),TEMPLATE_BLANK) == 0 ) {
		// blank string: %BLANK
		type = TEMPLATE_S_BLANK;
	
	} else  {
		type = TEMPLATE_S_UNKNOWN;
	}
	
	// return parsed length
	exp = content;
	return ( end-pos+TEMPLATE_END_LEN );
}

/// 分析表达式的值
/// \param exp 表达式字符串
/// \return 返回值为该表达式的值,若表达式非法则返回表达式字符串
string Template::exp_value( const string &exp ) {
	if ( strncmp(exp.c_str(),TEMPLATE_VALUE,TEMPLATE_VALUE_LEN) == 0 ) {
		// simple value: $xxx
		string val = exp.substr( TEMPLATE_VALUE_LEN );
		return _sets[val];
		
	} else if ( strncmp(exp.c_str(),TEMPLATE_TBLVALUE,TEMPLATE_TBLVALUE_LEN) == 0 ) {
		// current value in table: .$xxx
		string val = exp.substr( TEMPLATE_TBLVALUE_LEN );
		return this->table_value( val );
		
	} else if ( strncmp(exp.c_str(),TEMPLATE_CURSOR,TEMPLATE_CURSOR_LEN) == 0 ) {
		// current table cursor: %CURSOR
		size_t pos = exp.find( TEMPLATE_TBLSCOPE );
		if ( pos != exp.npos ) {
			string table_name = this->exp_value( exp.substr(pos+TEMPLATE_TBLSCOPE_LEN) );
			int cursor = _tables[table_name].cursor;
			return itos(cursor+1);
		} else {
			return itos(_cursor+1);
		}
	
	} else if ( strncmp(exp.c_str(),TEMPLATE_ROWS,TEMPLATE_ROWS_LEN) == 0 ) {
		// current table cursor: %ROWS
		size_t pos = exp.find( TEMPLATE_TBLSCOPE );
		if ( pos != exp.npos ) {
			string table_name = this->exp_value( exp.substr(pos+TEMPLATE_TBLSCOPE_LEN) );
			return itos( _tables[table_name].rows );
		} else {
			return itos( _tables[_table].rows );
		}

	} else if ( strcmp(exp.c_str(),TEMPLATE_DATE) == 0 ) {
		// date: %DATE
		return _date;
	
	} else if ( strcmp(exp.c_str(),TEMPLATE_TIME) == 0 ) {
		// time: %TIME
		return _time;
	
	} else if ( strcmp(exp.c_str(),TEMPLATE_SPACE) == 0 ) {
		// space char: %SPACE
		return " ";
	
	} else if ( strcmp(exp.c_str(),TEMPLATE_BLANK) == 0 ) {
		// blank string: %BLANK
		return "";
	
	} else  {
		// string
		return exp;
	}
}

/// 分析处理模板
/// \param tmpl 模板字符串
/// \param output 分析处理结果输出流
void Template::parse( const string &tmpl, ostream &output ) {
	// init datetime
	struct tm stm;
	time_t tt = time( 0 );
	localtime_r( &tt, &stm );
	snprintf( _date, 15, "%d-%d-%d", stm.tm_year+1900, stm.tm_mon+1, stm.tm_mday );
	snprintf( _time, 15, "%d:%d:%d", stm.tm_hour, stm.tm_min, stm.tm_sec );

	// confirm if inited
	if ( _tmpl == "" ) {
		this->error_log( 0, "Error: Templet not initialized" );
		return;
	}
	
	// parse init
	_table = "";
	_cursor = 0;
	_lines = 0;
	
	size_t lastpos = 0;
	size_t currpos = 0;

	// for parse_script()
	string exp;
	int type;
	int parsed;
	
	// search TEMPLATE_BEGIN in tmpl
	while( (currpos=tmpl.find(TEMPLATE_BEGIN,lastpos)) != tmpl.npos ) {
		// output html before TEMPLATE_BEGIN
		output << tmpl.substr( lastpos, currpos-lastpos );
		
		// log current position
		if ( _debug == TEMPLATE_OUTPUT_DEBUG ) {
			String orightml = tmpl.substr( lastpos, currpos-lastpos );
			_lines += orightml.count( TEMPLATE_NEWLINE );
		}
		
		// get script content between TEMPLATE_BEGIN and TEMPLATE_END
		parsed = this->parse_script( tmpl, currpos, exp, type );
		
		if ( parsed < 0 ) {
			// can't find TEMPLATE_END
			lastpos = currpos;
			this->error_log( _lines, "Error: Can't find TEMPLATE_END" );
			break;
		}
		
		// parse by script type
		switch ( type ) {
			case TEMPLATE_S_VALUE:
				// replace
			case TEMPLATE_S_DATE:
				// replace with date
			case TEMPLATE_S_TIME:
				// replace with time
			case TEMPLATE_S_SPACE:
				// replace with space char
			case TEMPLATE_S_BLANK:
				// replace with blank string
				output << this->exp_value( exp );
				break;

			case TEMPLATE_S_IF:
				// condition replace
				parsed = this->parse_if( tmpl.substr(currpos), output, true, exp, parsed );
				break;
				
			case TEMPLATE_S_FOR:
				// cycle replace
				parsed = this->parse_for( tmpl.substr(currpos), output, true, exp, parsed );
				// restore table status
				_table = "";
				_cursor = 0;
				break;

			case TEMPLATE_S_UNKNOWN:
				// unknown script, maybe html code
				this->error_log( _lines, "Warning: Unknown script, in parse()" );
				
				// for syntax error
				size_t backlen;
				if ( (backlen=exp.find(TEMPLATE_BEGIN)) != exp.npos )
					parsed = backlen+TEMPLATE_BEGIN_LEN;
					
				output << tmpl.substr( currpos, parsed );
				break;
				
			default:
				// syntax error
				this->error_log( _lines, "Error: Unexpected script, in parse()" );
		}

		// location to next position
		lastpos = currpos + parsed;
	}
	
	// output tail html
	output << tmpl.substr( lastpos );
}

/// 检查条件语句表达式是否成立
/// \param exp 参数为条件表达式,
/// 若表达式为字符串,则值不为""并且不为"0"时返回true,否则返回false,
/// 若为比较表达式,成立返回true,否则返回false
/// \retval true 条件表达式成立
/// \retval false 条件表达式不成立
bool Template::compare( const string &exp ) {
	// read compare type
	// supported: ==,!=,<=,<,>=,>
	string cmpop;
	size_t oppos;
	template_cmptype optype;
	
	if ( (oppos=exp.find(TEMPLATE_EQ)) != exp.npos ) {
		// ==
		cmpop = TEMPLATE_EQ;
		optype = TEMPLATE_C_EQ;
	
	} else if ( (oppos=exp.find(TEMPLATE_NE)) != exp.npos ) {
		// !=
		cmpop = TEMPLATE_NE;
		optype = TEMPLATE_C_NE;
	
	} else if ( (oppos=exp.find(TEMPLATE_LE)) != exp.npos ) {
		// <=
		cmpop = TEMPLATE_LE;
		optype = TEMPLATE_C_LE;
	
	} else if ( (oppos=exp.find(TEMPLATE_LT)) != exp.npos ) {
		// <
		cmpop = TEMPLATE_LT;
		optype = TEMPLATE_C_LT;
	
	} else if ( (oppos=exp.find(TEMPLATE_GE)) != exp.npos ) {
		// >=
		cmpop = TEMPLATE_GE;
		optype = TEMPLATE_C_GE;
	
	} else if ( (oppos=exp.find(TEMPLATE_GT)) != exp.npos ) {
		// >
		cmpop = TEMPLATE_GT;
		optype = TEMPLATE_C_GT;
	
	} else {
		// read value, compare and return
		string val = this->exp_value( exp );
		if ( val!="" && val!="0" )
			return true;
		else
			return false;
	}

	// split exp by compare operator
	String lexp = exp.substr( 0, oppos );
	String rexp = exp.substr( oppos+cmpop.length() );
	
	// read value
	lexp.trim(); rexp.trim();
	lexp = this->exp_value( lexp );
	rexp = this->exp_value( rexp );

	// compare
	int cmp;
	if ( lexp.isnum() && rexp.isnum() ) {
		int lv = atoi( lexp.c_str() );
		int rv = atoi( rexp.c_str() );
		cmp = ( lv>rv ) ? 1 : ( lv==rv ) ? 0 : -1;
	} else {
		cmp = strcmp( lexp.c_str(), rexp.c_str() );
	}

	// return
	switch ( optype ) {
		case TEMPLATE_C_EQ:
			return ( cmp==0 ) ? true : false;
		case TEMPLATE_C_NE:
			return ( cmp!=0 ) ? true : false;
		case TEMPLATE_C_LE:
			return ( cmp<=0 ) ? true : false;
		case TEMPLATE_C_LT:
			return ( cmp<0 ) ? true : false;
		case TEMPLATE_C_GE:
			return ( cmp>=0 ) ? true : false;
		case TEMPLATE_C_GT:
			return ( cmp>0 ) ? true : false;
		default:
			return false;
	}
}

/// 检查条件是否成立	
/// \param exp 参数为条件表达式及其组合
/// \retval true 条件表达式成立
/// \retval false 条件表达式不成立
bool Template::check_if( const string &exp ) {
	template_logictype exp_type = TEMPLATE_L_NONE;
	String exps;

	// check expression type
	if ( strncmp(exp.c_str(),TEMPLATE_AND,TEMPLATE_AND_LEN) == 0 ) {
		exp_type = TEMPLATE_L_AND;
		exps = exp.substr( TEMPLATE_AND_LEN );
	} else if ( strncmp(exp.c_str(),TEMPLATE_OR,TEMPLATE_OR_LEN) == 0 ) {
		exp_type = TEMPLATE_L_OR;
		exps = exp.substr( TEMPLATE_OR_LEN );
	}

	// none logic expression
	if ( exp_type == TEMPLATE_L_NONE )
		return this->compare( exp );

	// check TEMPLATE_SUBBEGIN/TEMPLATE_SUBEND
	exps.trim();
	size_t explen = exps.length();
	if ( exps.substr(0,TEMPLATE_SUBBEGIN_LEN)!=TEMPLATE_SUBBEGIN ||
		 exps.substr(explen-TEMPLATE_SUBEND_LEN)!=TEMPLATE_SUBEND ) {
		this->error_log( _lines, "Warning: Maybe wrong TEMPLATE_AND or TEMPLATE_OR script" );
		return this->compare( exp );
	}
		
	// split expressions list
	exps = exps.substr( TEMPLATE_SUBBEGIN_LEN, explen-TEMPLATE_SUBBEGIN_LEN-TEMPLATE_SUBEND_LEN );
	vector<String> explist = exps.split( TEMPLATE_SPLIT );

	// judge
	if ( exp_type == TEMPLATE_L_AND ) {
		// TEMPLATE_AND
		for ( unsigned int i=0; i<explist.size(); i++ ) {
			explist[i].trim();
			if ( !this->compare(explist[i]) )
				return false;
		}
		return true;
		
	} else {
		// TEMPLATE_OR
		for ( size_t i=0; i<explist.size(); i++ ) {
			explist[i].trim();
			if ( this->compare(explist[i]) )
				return true;
		}
		return false;
	}
	
	return false; // for warning
}

/// 处理条件类型模板
/// \param tmpl 模板字符串
/// \param output 分析处理结果输出流
/// \param parent_state 调用该函数时的条件状态
/// \param parsed_exp 已分析的条件脚本表达式
/// \param parsed_length 已分析的条件脚本表达式长度
/// \return 返回值为本次分析的字符串长度
size_t Template::parse_if( const string &tmpl, ostream &output, 
					  const bool parent_state, const string &parsed_exp,
					  const int parsed_length ) {
	// parsed length
	size_t length = parsed_length;
		
	// check status
	bool status;
	bool effected;
	if ( parent_state == false ) {
		status = false;
		effected = true;
	} else if ( this->check_if(parsed_exp) ) {
		status = true;
		effected = true;
	} else {
		status = false;
		effected = false;
	}
		
	// parse
	size_t lastpos = parsed_length;
	size_t currpos = parsed_length;

	// for parse_script()
	string exp;
	int type;
	int parsed;
	
	while( (currpos=tmpl.find(TEMPLATE_BEGIN,lastpos)) != tmpl.npos ) {
		// output html before TEMPLATE_BEGIN if status valid
		if ( status )
			output << tmpl.substr( lastpos, currpos-lastpos );
		length += ( currpos-lastpos );
		
		// log current position
		if ( _debug == TEMPLATE_OUTPUT_DEBUG ) {
			String orightml = tmpl.substr( lastpos, currpos-lastpos );
			_lines += orightml.count( TEMPLATE_NEWLINE );
		}
		
		// get script content between TEMPLATE_BEGIN and TEMPLATE_END
		parsed = this->parse_script( tmpl, currpos, exp, type );
		
		if ( parsed < 0 ) {
			// can't find TEMPLATE_END
			this->error_log( _lines, "Error: Can't find TEMPLATE_END" );
			lastpos = currpos;
			break;
		}
		
		// parse by script type
		switch ( type ) {
			case TEMPLATE_S_VALUE:
				// replace if status is true
			case TEMPLATE_S_TBLVALUE:
				// replace with table value if status is true
			case TEMPLATE_S_CURSOR:
				// replace with cursor
			case TEMPLATE_S_ROWS:
				// replace with rows
			case TEMPLATE_S_DATE:
				// replace with date
			case TEMPLATE_S_TIME:
				// replace with time
			case TEMPLATE_S_SPACE:
				// replace with space char
			case TEMPLATE_S_BLANK:
				// replace with blank string
				if ( status )
					output << this->exp_value( exp );
				break;

			case TEMPLATE_S_ELSIF:
				// check and set status
				if ( effected ) {
					// something effected before
					status = false;
				} else if ( this->check_if(exp) ) {
					// nothing effected and status is true now
					status = true;
					effected = true;
				} else {
					// nothing effected, status is false now
					status = false;
				}
				break;

			case TEMPLATE_S_ELSE:
				// check and set status
				if ( effected ) {
					// something effected before
					status = false;
				} else {
					// nothing effected, then status is true
					status = true;
					effected = true;
				}
				break;

			case TEMPLATE_S_ENDIF:
				// parsed length
				length += parsed;
				// exit function
				return length;
					
			case TEMPLATE_S_IF:
				// sub condition replace
				parsed = this->parse_if( tmpl.substr(currpos), output, status, exp, parsed );
				break;
				
			case TEMPLATE_S_FOR: { // make compiler happy
				// backup current table status
				string parent_table = _table;
				int parent_cursor = _cursor;
				
				// sub cycle replace
				parsed = this->parse_for( tmpl.substr(currpos), output, status, exp, parsed );
				
				// restore table status
				_table = parent_table;
				_cursor = parent_cursor;
				_tables[_table].cursor = _cursor;
				}
				break;

			case TEMPLATE_S_UNKNOWN:
				// unknown script, maybe html code
				this->error_log( _lines, "Warning: Unknown script, in parse_if()" );

				// for syntax error
				size_t backlen;
				if ( (backlen=exp.find(TEMPLATE_BEGIN)) != exp.npos )
					parsed = backlen+TEMPLATE_BEGIN_LEN;

				if ( status )
					output << tmpl.substr( currpos, parsed );
				break;

			default:
				// syntax error
				error_log( _lines, "Error: Unexpected script, in parse_if()" );
		}

		// parsed length
		length += parsed;
		// location to next position
		lastpos = currpos + parsed;
	}
	
	// can not find TEMPLATE_ENDIF
	this->error_log( _lines, "Error: Can't find TEMPLATE_ENDIF" );

	// output tail html
	if ( status )
		output << tmpl.substr( lastpos );
	length += ( tmpl.size()-lastpos );
	
	return length;
}

/// 检查循环语句是否有效
/// \param table 循环表格名称
/// \retval true 表格已定义
/// \retval false 未定义
bool Template::check_for( const string &tablename ) {
	string table = this->exp_value( tablename );
	
	if ( _tables.find(table) != _tables.end() && _tables[table].rows > 0 ) {
		return true;
	} else {
		this->error_log( _lines, "Warning: table " + tablename + " \""+table+
						   		 "\" not defined or not set data" );
		return false;
	}
}

/// 返回表格中指定位置字段的值
/// \param fleid 表格变量字段名
/// \return 若读取成功返回值字符串,否则返回空字符串
string Template::table_value( const string &field ) {
	// get table info
	size_t pos = field.find( TEMPLATE_TBLSCOPE );
	if ( pos != field.npos ) {
		string table_name = this->exp_value( field.substr(pos+TEMPLATE_TBLSCOPE_LEN) );
		string field_name = field.substr( 0, pos );
		int cursor = _tables[table_name].cursor;

		// return value
		int col = this->field_pos( table_name, field_name );
		if ( col!=-1 && cursor<_tables[table_name].rows )
			return _tables[table_name].datas[cursor][col];
		else
			return string( "" );
	} else {
		// return value
		int col = this->field_pos( _table, field );
		if ( col!=-1 && _cursor<_tables[_table].rows )
			return _tables[_table].datas[_cursor][col];
		else
			return string( "" );
	}
}

/// 处理循环类型模板
/// \param tmpl 模板字符串
/// \param output 分析处理结果输出流
/// \param parent_state 调用该函数时的条件状态
/// \param parsed_exp 已分析的循环脚本表达式
/// \param parsed_length 已分析的循环脚本表达式长度
/// \return 返回值为本次分析的字符串长度
size_t Template::parse_for( const string &tmpl, ostream &output, 
					   const bool parent_state, const string &parsed_exp,
					   const int parsed_length ) {
	// parsed length
	size_t length = parsed_length;
		
	// check status
	bool status;
	if ( parent_state == false )
		status = false;
	else if ( this->check_for(parsed_exp) )
		status = true;
	else
		status = false;
		
	// init
	bool cycled = false;
	int cursor = 0;
	size_t start_pos = parsed_length;
	size_t start_len = parsed_length;
	size_t lastpos = parsed_length;
	size_t currpos = parsed_length;
	
	// current table name
	string table = this->exp_value( parsed_exp );
	_table = table;
	_tables[_table].cursor = 0;

	// for parse_script()		
	string exp;
	int type;
	int parsed;

	while( (currpos=tmpl.find(TEMPLATE_BEGIN,lastpos)) != tmpl.npos ) {
		// output html before TEMPLATE_BEGIN if status valid
		if ( status )
			output << tmpl.substr( lastpos, currpos-lastpos );
		length += ( currpos-lastpos );
		
		// log current position
		if ( !cycled && _debug==TEMPLATE_OUTPUT_DEBUG ) {
			String orightml = tmpl.substr( lastpos, currpos-lastpos );
			_lines += orightml.count( TEMPLATE_NEWLINE );
		}
		
		// get script content between TEMPLATE_BEGIN and TEMPLATE_END
		parsed = this->parse_script( tmpl, currpos, exp, type );
		
		if ( parsed < 0 ) {
			// can't find TEMPLATE_END
			if ( !cycled )
				this->error_log( _lines, "Error: Can't find TEMPLATE_END" );
			lastpos = currpos;
			break;
		}
		
		// current table cursor
		_cursor = cursor;

		// parse by script type
		switch ( type ) {
			case TEMPLATE_S_VALUE:
				// replace
			case TEMPLATE_S_TBLVALUE:
				// replace with table value in table
			case TEMPLATE_S_CURSOR:
				// replace with cursor
			case TEMPLATE_S_ROWS:
				// replace with rows				
			case TEMPLATE_S_DATE:
				// replace with date
			case TEMPLATE_S_TIME:
				// replace with time
			case TEMPLATE_S_SPACE:
				// replace with space char
			case TEMPLATE_S_BLANK:
				// replace with blank string
				if ( status )
					output << this->exp_value( exp );
				break;

			case TEMPLATE_S_ENDFOR:
				// at the end of this cycle
				++cursor; // data cursor
				_tables[_table].cursor = cursor;
				if ( status && cursor<_tables[table].rows ) {
					// next cycle
					length = start_len;		// reset parsed length
					lastpos = start_pos;	// reset start position
					cycled = true;			// do not add current position value
					continue;
				} else {
					// parsed length
					length += parsed;
					// exit function
					return length;
				}
					
			case TEMPLATE_S_IF:
				// sub condition replace
				parsed = this->parse_if( tmpl.substr(currpos), output, status, exp, parsed );
				break;
				
			case TEMPLATE_S_FOR:
				// sub cycle replace
				parsed = this->parse_for( tmpl.substr(currpos), output, status, exp, parsed );
				// restore table status
				_table = table;
				_cursor = cursor;
				_tables[_table].cursor = _cursor;
				break;

			case TEMPLATE_S_UNKNOWN:
				// unknown script, maybe html code
				if ( !cycled )
					this->error_log( _lines, "Warning: Unknown script, in parse_for()" );

				// for syntax error
				size_t backlen;
				if ( (backlen=exp.find(TEMPLATE_BEGIN)) != exp.npos )
					parsed = backlen+TEMPLATE_BEGIN_LEN;

				if ( status )
					output << tmpl.substr( currpos, parsed );
				break;

			default:
				// syntax error
				if ( !cycled )
					this->error_log( _lines, "Error: Unexpected script, in parse_for()" );
		}

		// parsed length
		length += parsed;
		// location to next position
		lastpos = currpos + parsed;
	}
	
	// can not find TEMPLATE_ENDFOR
	this->error_log( _lines, "Error: Can't find TEMPLATE_ENDFOR" );

	// output tail html
	if ( status )
		output << tmpl.substr( lastpos );
	length += ( tmpl.size()-lastpos );

	return length;
}

////////////////////////////////////////////////////////////////////////////
// output functions

/// 模板分析错误纪录
/// \param pos 模板错误行位置
/// \param error 错误描述
void Template::error_log( const size_t lines, const string &error ) {
	if ( error != "" )
		_errlog.insert( multimap<int,string>::value_type(lines,error) );
}

/// 返回模板分析纪录
/// \param output 分析处理结果输出流
void Template::parse_log( ostream &output ) {
	output << endl;
	output << "<!-- Generated by waTemplate " << _date << " " << _time << endl
		   << "  Templet source: " << _tmplfile << endl
		   << "  Tables: " << _tables.size() << endl;

	for ( map<string,template_table>::const_iterator i=_tables.begin(); i!=_tables.end(); ++i ) {
		if ( i->first != "" ) {
			output << "    Table " << i->first
				   << "\t\t" << (i->second).cursor << " rows" << endl;
		}
	}

	output << "  Errors: " << _errlog.size() << endl;
	for ( multimap<int,string>::const_iterator i=_errlog.begin(); i!=_errlog.end(); ++i ) {
		output << "    Line " << i->first+1
			   << "\t\t" << i->second << endl;
	}
			   
	output << "-->";
	_errlog.clear();
}

/// 返回HTML字符串
/// \return 返回模板分析处理结果
string Template::html() {
	ostringstream result;
	this->parse( _tmpl, result );
	result << ends;
	return result.str();
}

/// 输出HTML到stdout
/// \param mode 是否输出调试信息
/// - Template::TEMPLATE_OUTPUT_DEBUG 输出调试信息
/// - Template::TEMPLATE_OUTPUT_RELEASE 不输出调试信息
/// - 默认为不输出调试信息
void Template::print( const output_mode mode ) {
	_debug = mode;
	this->parse( _tmpl, std::cout );
	if ( _debug == TEMPLATE_OUTPUT_DEBUG ) 
		this->parse_log( std::cout );
}

/// 输出HTML到文件
/// \param file 输出文件名
/// \param mode 是否输出调试信息
/// - Template::TEMPLATE_OUTPUT_DEBUG 输出调试信息
/// - Template::TEMPLATE_OUTPUT_RELEASE 不输出调试信息
/// - 默认为不输出调试信息
/// \param permission 文件属性参数，默认为0666
/// \retval true 文件输出成功
/// \retval false 失败
bool Template::print( const string &file, const output_mode mode,
				 const mode_t permission ) {
	ofstream outfile( file.c_str(), ios::trunc|ios::out );
	if ( outfile ) {
		// parse
		_debug = mode;
		this->parse( _tmpl, outfile );
		if ( _debug == TEMPLATE_OUTPUT_DEBUG ) 
			this->parse_log( outfile );
		outfile.close();
		
		// chmod
		mode_t mask = umask( 0 );
		chmod( file.c_str(), permission );
		umask( mask );

		return true;
	}
	
	return false;
}

} // namespace


