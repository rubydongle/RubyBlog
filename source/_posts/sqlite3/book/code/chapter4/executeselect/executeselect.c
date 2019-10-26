#include<stdio.h>
#include<stdlib.h>
#include"sqlite3.h"
#include<string.h>

int main(int argc,char **argv)
{
    // 校验参数有效性
    char *dbname = argv[1];
    char *tablename = argv[2];
    if (argc < 3) {
        printf("Error: Must specific a db file and select table name\n");
	printf("\nUsage:\n\t%s file.db table_name\n", argv[0]);
	exit(1);
    }
    printf("db file %s:SELECT * FROM %s\n", dbname, tablename);

    int rc, i, ncols;
    sqlite3 *db;
    sqlite3_stmt *stmt;
    char *sql;
    const char*tail;

    // 打开数据
    rc = sqlite3_open(dbname, &db);
    if(rc) {
        fprintf(stderr,"Can'topendatabase:%sn",sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }

    // 构造执行语句
    sql = malloc(strlen("SELECT * FROM ") + strlen(tablename));
    if (sql == NULL) {
        sqlite3_close(db);
        exit(1);
    }
    sprintf(sql, "SELECT * FROM %s", tablename);

    // 预处理
    rc = sqlite3_prepare(db, sql, (int)strlen(sql), &stmt, &tail);
    if (rc != SQLITE_OK){
        fprintf(stderr, "SQLerror:%s\n", sqlite3_errmsg(db));
    }
    rc = sqlite3_step(stmt);
    ncols = sqlite3_column_count(stmt);
    while (rc == SQLITE_ROW){
        for (i=0; i<ncols; i++){
            fprintf(stderr, "'%s'", sqlite3_column_text(stmt, i));
        }
        fprintf(stderr, "\n");
        rc = sqlite3_step(stmt);
    }

    // 释放statement
    sqlite3_finalize(stmt);

    // 关闭数据库
    sqlite3_close(db);
    printf("\n");
    return(0);
}
 
