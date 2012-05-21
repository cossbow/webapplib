/// \file example.cpp
/// 示例文件
/// 演示waString,MysqlClient,MysqlData,Encode,Cgi,DateTime,Template,HttpClient的使用

#include <iostream>
#include "webapplib.h"

using namespace webapp;

int main() {
	////////////////////////////////////////////////////////////////////////////
	// 演示完整的CGI应用程序流程,模拟WEB查询动作
	
	// 根据cookie判断用户身份;
	// 调用远端CGI验证用户是否有效;
	// 取得CGI参数data_name;
	// 拼凑为SQL语句;
	// 连接数据库并执行查询;
	// 纪录本次查询请求;
	// 在用户浏览器写入cookie;
	// 显示查询结果.

	////////////////////////////////////////////////////////////////////////////
	// 读取cookie "username"
	
	Cookie cookie; // Cookie读取对象
	String username = cookie["username"];;
	if ( username == "" ) {
		// 读不到cookie值,假定为终端调试模式,手动输入继续执行
		cout << "*** Terminal Debug Mode ***" << endl;
		cout << "Input value of cookie 'username': " << flush;
		cin >> username;
	}
	
	////////////////////////////////////////////////////////////////////////////
	// 通过调用远端CGI接口验证用户名是否有效
	
	String checkuser = "http://members.sina.com.cn/cgi-bin/check.cgi?username=" + username;
	HttpClient www; // HTTP请求对象
	www.request( checkuser );
	if ( !www.done() ) {
		// 执行HTTP请求失败,假定为终端调试模式,继续执行
		cout << "HTTP request failure, continue..." << endl;
	}
	
	if ( www.content() != "CHECK OK!" ) {
		// 返回结果错误,假定为终端调试模式,继续执行
		cout << "Check username error, continue..." << endl;
	}
	
	////////////////////////////////////////////////////////////////////////////
	// 取得CGI参数
	
	Cgi cgi;  // CGI读取对象
	String data_name = cgi["data_name"];
	
	// 根据输入参数拼凑SQL语句
	String sql;
	sql.sprintf( "SELECT data_value FROM data_table WHERE data_name='%s'", 
				 escape_sql(data_name).c_str() );
	
	////////////////////////////////////////////////////////////////////////////
	// 连接数据库
	
	MysqlClient mysql;  // Mysql数据库连接对象
	MysqlData mysqlres; // MysqlRes数据库结果集对象
	String datavalue;

	// 连接数据库
	mysql.connect( "127.0.0.1", "user", "pwd", "database" );
	if ( !mysql.is_connected() ) {
		// 无法连接数据库,假定为终端调试模式,继续执行
		cout << "Connect DB failure, continue..." << endl;
	}

	// 执行查询
	if ( mysql.query(sql,mysqlres) ) {
		// 取得查询结果的第一行记录中的"data_value"字段值
		datavalue = mysqlres( 0, "data_value" );
	} else {
		// 查询数据库错误,假定为终端调试模式,继续执行
		cout << "Query DB error, input value of 'data_value':" << flush;
		cin >> datavalue;
	}

	////////////////////////////////////////////////////////////////////////////
	// 纪录本次查询请求
	
	DateTime now; // DateTime对象,默认初始化为当前时间

	String log;
	// 当前日期时间,用户名,SQL语句,数据库错误信息
	log = now.datetime() + "\t" + username + "\t" + sql + "\t" + mysql.error() + "\n";
	// 追加写入到日志文件
	log.save_file( "example.log", ios::app, 0666 );

	////////////////////////////////////////////////////////////////////////////
	// 在用户浏览器写入用户名cookie
	
	// 有效期为三天
	DateTime expires = now + ( TIME_ONE_DAY*3 );
	// 出于安全性考虑，用户口令等敏感cookie内容应当进行加密
	cookie.set_cookie( "username", username, expires.gmt_datetime() );
	
	////////////////////////////////////////////////////////////////////////////
	// 输出结果页面
	
	// 读取HTML结果页面模板并替换结果内容
	Template page;
	if ( !page.load("template.htt") ) {
		// 无法读取模板文件,假定为终端调试模式,继续执行
		String input_words;
		cout << "Load template file failure, input content of 'welcome word':" << endl;
		cin >> input_words;
		String template_text = input_words + " ([$username])\n" +
							   "Value of '" + data_name + "' is: ([$datavalue])\n";
		page.tmpl( template_text );
	}
	
	page.set( "username", username );
	page.set( "datavalue", datavalue );
	
	// 显示查询结果
	http_head();
	page.print();
}

