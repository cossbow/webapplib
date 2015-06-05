代码托管和后续更新均迁移到：https://github.com/pi1ot/webapplib

WebAppLib所有的类、函数、常量都声明于webapp命名空间内，由以下部分组成：

|String|继承并兼容与std::string的字符串类，增加了开发中常用的字符串处理函数|
|:-----|:----------------------------------------------------------------------------------------------|
|Cgi   |支持文件上传的CGI参数读取类                                                        |
|Cookie|HTTP Cookie设置与读取类                                                                  |
|MysqlClient|MySQL数据库连接类，MySQL连接处理C函数接口的C++封装                           |
|MysqlData|MySQL查询结果数据集类，MySQL查询结果数据提取C函数接口的C++封装         |
|Template|支持在模板中嵌入条件跳转、循环输出脚本的 HTML 模板类                    |
|HttpClient|HTTP/1.1通信协议客户端类                                                               |
|DateTime|日期时间运算、格式化输出类                                                        |
|TextFile|固定分隔符文本文件读取解析类                                                     |
|ConfigFile|INI格式配置文件解析类                                                                 |
|FileSystem|文件系统操作函数库                                                                    |
|Encode|字符串编码解码函数库                                                                 |
|Utility|系统调用与工具函数库                                                                 |

  * WebAppLib（Web Application Library）是一系列主要用于类Unix操作系统环境下Web开发的C++类库
  * 开发目的：通过接口简单、使用方便、互相独立的C++类和函数来简化Web和CGI程序开发过程中的常见操作，提高开发和维护效率
  * 背景介绍：https://code.google.com/p/webapplib/wiki/Introduction
  * 简单范例：https://code.google.com/p/webapplib/wiki/example_code
  * 编译本类库要求使用g++编译器，版本不低于v3.4.0
  * 经过测试确认的操作系统有Linux(CentOS v4.0以上版本)，Solaris(v10以上版本)，FreeBSD系统未经严格测试
  * 可以通过Cygwin环境运行于Windows操作系统
  * 类库详细使用说明可参见类库参考手册 help.chm 或者 help.pdf