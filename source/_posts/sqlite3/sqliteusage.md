---
title: sqliteusage
categories:
  - 技术文章
date: 2019-10-20 09:15:21
tags:
---

# 脚本
下面的脚本可以创建含有自定义条记录的数据库
```bash
#!/bin/sh

rm -rf test2.db
sqlite3 test2.db "CREATE TABLE info (name TEXT, age INT, salary INT)"
rand(){
  min=$1
  max=$(($2-$min+1))
  num=$(date +%s%N)
  echo $(($num%$max+$min))
}

randname(){
  which=$(rand 0 10)
  #echo $which
  if [ $which -eq 0 ]
  then
    echo \'ruby\'
  elif [ $which -eq 1 ]
  then
    echo \'lily\'
  elif [ $which -eq 2 ]
  then
    echo \'jhon\'
  elif [ $which -eq 3 ]
  then
    echo \'jobs\'
  elif [ $which -eq 4 ]
  then
    echo \'cook\'
  elif [ $which -eq 5 ]
  then
    echo \'sam\'
  elif [ $which -eq 6 ]
  then
    echo \'bill\'
  elif [ $which -eq 7 ]
  then
    echo \'tom\'
  elif [ $which -eq 8 ]
  then
    echo \'jerry\'
  elif [ $which -eq 9 ]
  then
    echo \'hank\'
  else
    echo \'doubi\'
  fi
}

for i in `seq 1 1000`
do
  echo "count:$i"
  name=$(randname)
  echo "name:$name"
  age=$(rand 20 30)
  echo "age:$age"
  salary=`expr $(rand 10 20) \* 1000`
  echo "salary:$salary"
  sqlite3 test.db "INSERT INTO info VALUES ($name, $age, $salary)"
done
```


# SQLite索引
索引(Index) 是一种特殊的查找表,数据库搜索引擎用来加快数据检索。简单地说，索引是一个指向表中数据的指针。一个数据库中的索引与一本书的索引目录是非常相似的。
例如，想在一本讨论某个话题的书中引用所有页面，首先需要指向索引，索引按字母顺序累出了所有主题，然后指向一个或多个特定的页码。

索引有助于加快SELECT查询和WHERE子句，但它会减慢使用UPDATE和INSERT语句时的数据输入。索引可以创建或删除，但不会影响数据。

使用CREATE INDEX 语句创建索引，它允许命名索引，指定表及要索引的一列或多列，并指示索引是升序排列还是降序排列。

索引也可以是唯一的。与UNIQUE约束类似，在列上或列组合上防止重复条目。
## 使用索引
1.创建索引
创建索引通过CREATE INDEX命令完成，基本语法如下：
```sql
CREATE INDEX index_name ON table_name;
```
2.单列索引
可以只基于表的一个列创建索引，基本语法如下：
```sql
CREATE INDEX index_name ON table_name (column_name);
```
3.唯一索引
使用唯一索引不仅是为了性能，同时也为了数据的完整性。唯一索引不允许任何重复的值插入到表中，基本语法如下：
```sql
CREATE UNIQUE INDEX index_name ON table_name (column_name);
```
4.组合索引
组合索引是基于一个表的两个或多个列上创建的索引，基本语法如下：
```sql
CREATE INDEX index_name ON table_name(column1, column2);
```
是否要创建一个单列索引还是组合索引，要考虑到您在作为查询过滤条件的 WHERE 子句中使用非常频繁的列。

如果值使用到一个列，则选择使用单列索引。如果在作为过滤的 WHERE 子句中有两个或多个列经常使用，则选择使用组合索引。

5.隐式索引
隐式索引是在创建对象时，由数据库服务器自动创建的索引。索引自动创建为主键约束和唯一约束。

6.删除索引
一个索引可以使用 SQLite 的 DROP 命令删除。当删除索引时应特别注意，因为性能可能会下降或提高。
删除索引的基本语法如下：
```sql
DROP INDEX index_name;
```

## 实例
```sql
batman@DESKTOP-970MMSR:~$ sqlite3 indexusage.db
SQLite version 3.11.0 2016-02-15 17:29:24
Enter ".help" for usage hints.
sqlite>
sqlite> CREATE TABLE info(name TEXT, age INT, salary INT);
sqlite> INSERT INTO info VALUES('ruby', 22, 10000);
sqlite> INSERT INTO info VALUES('jhon', 25, 12000);
sqlite> INSERT INTO info VALUES('lily', 23, 18000);
sqlite> INSERT INTO info VALUES('lucy', 31, 16000);
sqlite> INSERT INTO info VALUES('joe', 24, 13000);
sqlite> INSERT INTO info VALUES('henry', 26, 23000);
sqlite> INSERT INTO info VALUES('mike', 28, 13000);
sqlite> INSERT INTO info VALUES('tom', 22, 13000);
sqlite> INSERT INTO info VALUES('jack', 24, 10000);
sqlite> INSERT INTO info VALUES('jerry', 28, 14000);
sqlite> .header on
sqlite> SELECT * FROM info;
name|age|salary
ruby|22|10000
jhon|25|12000
lily|23|18000
lucy|31|16000
joe|24|13000
henry|26|23000
mike|28|13000
tom|22|13000
jack|24|10000
jerry|28|14000
sqlite> .indexes info

# 在info表的name列上创建一个索引
sqlite> CREATE INDEX age_index ON info(age);
sqlite> .indexes info
age_index

# 列出数据库范围的所有索引，如下所示：
sqlite> SELECT * FROM sqlite_master WHERE type='index';
type|name|tbl_name|rootpage|sql
index|age_index|info|3|CREATE INDEX age_index ON info(age)
```

## 什么情况下要避免使用索引？
虽然索引的目的在于提高数据库的性能，但这里有几个情况需要避免使用索引。使用索引时，应重新考虑下列准则：

索引不应该使用在较小的表上。

索引不应该使用在有频繁的大批量的更新或插入操作的表上。

索引不应该使用在含有大量的 NULL 值的列上。

索引不应该使用在频繁操作的列上。

# SQLite触发器(Trigger)
触发器（trigger）是SQL server 提供给程序员和数据分析员来保证数据完整性的一种方法，它是与表事件相关的特殊的存储过程，它的执行不是由程序调用，也不是手工启动，而是由事件来触发，比如当对一个表进行操作（ insert，delete， update）时就会激活它执行。触发器经常用于加强数据的完整性约束和业务规则等。 触发器可以从 DBA_TRIGGERS ，USER_TRIGGERS 数据字典中查到

触发器可以查询其他表，而且可以包含复杂的SQL语句。它们主要用于强制服从复杂的业务规则或要求。例如：您可以根据客户当前的帐户状态，控制是否允许插入新订单。
触发器也可用于强制引用完整性，以便在多个表中添加、更新或删除行时，保留在这些表之间所定义的关系。然而，强制引用完整性的最好方法是在相关表中定义主键和外键约束。如果使用数据库关系图，则可以在表之间创建关系以自动创建外键约束。
触发器与存储过程的唯一区别是触发器不能执行EXECUTE语句调用，而是在用户执行Transact-SQL语句时自动触发执行。

触发器有如下作用：
- 可在写入数据表前，强制检验或转换数据。
- 触发器发生错误时，异动的结果会被撤销。
- 部分数据库管理系统可以针对数据定义语言（DDL）使用触发器，称为DDL触发器。
- 可依照特定的情况，替换异动的指令 (INSTEAD OF)。

SQLite 触发器（Trigger）是数据库的回调函数，它会在指定的数据库事件发生时自动执行/调用。
- SQLite 的触发器（Trigger）可以指定在特定的数据库表发生 DELETE、INSERT 或 UPDATE 时触发，或在一个或多个指定表的列发生更新时触发。
- SQLite 只支持 FOR EACH ROW 触发器（Trigger），没有 FOR EACH STATEMENT 触发器（Trigger）。因此，明确指定 FOR EACH ROW 是可选的。
- WHEN 子句和触发器（Trigger）动作可能访问使用表单 NEW.column-name 和 OLD.column-name 的引用插入、删除或更新的行元素，其中 column-name 是从与触发器关联的表的列的名称。
- 如果提供 WHEN 子句，则只针对 WHEN 子句为真的指定行执行 SQL 语句。如果没有提供 WHEN 子句，则针对所有行执行 SQL 语句。
- BEFORE 或 AFTER 关键字决定何时执行触发器动作，决定是在关联行的插入、修改或删除之前或者之后执行触发器动作。
- 当触发器相关联的表删除时，自动删除触发器（Trigger）。
- 要修改的表必须存在于同一数据库中，作为触发器被附加的表或视图，且必须只使用 tablename，而不是 database.tablename。
- 一个特殊的 SQL 函数 RAISE() 可用于触发器程序内抛出异常。

# SQLite视图(View)
视图(View)是通过相关的名称存储在数据库中的一个SQLite语句，它实际上是一个以预定义的SQLite查询形式存在的表的组合。
视图可以包含一个表的所有行或从一个或多个表选定行，它可以从一个表或多个表创建，这取决于要创建视图的SQLite查询。
视图是一种虚表，允许用户实现以下几点：
- 用户或用户组查找结构数据的方式更自然或直观。
- 限制数据访问，用户只能看到有限的数据，而不是完整的表。
- 汇总各种表中的数据，用于生成报告。

SQLite视图是只读的，因此可能无法在视图上执行DELETE、INSERT或UPDATE语句。但是可以在视图上创建一个触发器，当尝试DELETE、INSERT或UPDATE视图时触发，需要做的动作在触发器内容中定义

## 使用视图
1.创建视图
可以通过CREATE VIEW语句创建视图，创建视图的基本语法如下：
```sql
CREATE [TEMP | TEMPORARY] VIEW view_name AS
SELECT column1, colum2......
FROM table_name
WHERE [condition];
```
SQLite视图可以从单一的表、多个表或者其他视图创建。可以在SELECT语句中包含多个表,这与在正常的 SQL SELECT 查询中的方式非常相似。如果使用了可选的 TEMP 或 TEMPORARY 关键字，则将在临时数据库中创建视图。

2.删除视图
可以通过DROP VIEW语句删除视图，删除视图的基本语法如下：
```sql
DROP VIEW view_name
```

## 实例
```sql

```

 
