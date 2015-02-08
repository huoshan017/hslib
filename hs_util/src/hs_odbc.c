#include "hs_odbc.h"

struct hs_odbc*
hs_odbc_connect(const char* server_name, const char* user_name, const char* password)
{
	SQLHENV henv;
	SQLHDBC hdbc;

	// 分配环境句柄
	SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);

	// 设置环境属性
	SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);

	// 分配连接句柄
	SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);

	// 设置连接属性
	SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (void*)30, 0);

	SQLRETURN retcode = SQLConnect(hdbc, (SQLCHAR*)server_name, SQL_NTS, (SQLCHAR*)user_name, SQL_NTS, (SQLCHAR*)password, SQL_NTS);

	if (retcode == SQL_SUCCESS) {
		printf("hs_odbc_connect: 连接(server_name:%s, user_name:%s, password:%s)成功\n", server_name, user_name, password);
	} else if (retcode == SQL_SUCCESS_WITH_INFO) {
		printf("hs_odbc_connnect: SQLConnect - SQL_SUCCESS_WITH_INFO\n");
		SQLCHAR sqlstate[10];
		SQLCHAR message[100];
		SQLGetDiagRec(SQL_HANDLE_DBC, hdbc, 1, sqlstate, NULL, message, sizeof(message), NULL);
		printf("hs_odbc_connect: [%s - %s]\n", sqlstate, message);
	} else {
		printf("hs_odbc_connect: SQLConnect - error\n");
		SQLCHAR sqlstate[10];
		SQLCHAR message[100];
		SQLGetDiagRec(SQL_HANDLE_DBC, hdbc, 1, sqlstate, NULL, message, sizeof(message), NULL);
		printf("hs_odbc_connect: [%s - %s]\n", sqlstate, message);
		return NULL;
	}

	SQLHSTMT hstmt;
	SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	retcode = SQLSetStmtAttr(hstmt, SQL_ATTR_ASYNC_ENABLE, (SQLPOINTER)SQL_ASYNC_ENABLE_ON, 0);
	if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
		printf("hs_odbc_connect: ODBC not support async mode\n");
		return NULL;
	}

	struct hs_odbc* odbc = (struct hs_odbc*)malloc(sizeof(struct hs_odbc));
	odbc->henv = henv;
	odbc->hdbc = hdbc;
	odbc->hstmt = hstmt;

	return odbc;
}

void
hs_odbc_disconnect(struct hs_odbc* odbc, bool destroy)
{
	if (odbc) {
		SQLFreeStmt(odbc->hstmt, SQL_CLOSE);
		SQLFreeHandle(SQL_HANDLE_DBC, odbc->hdbc);
		SQLFreeHandle(SQL_HANDLE_ENV, odbc->henv);
		SQLDisconnect(odbc->hdbc);
		free(odbc);
	}
}

bool
hs_odbc_execute(struct hs_odbc* odbc, const char* sql, int sql_len)
{
	if (!odbc)
		return false;

	SQLRETURN retcode = SQLExecDirect(odbc->hstmt, (SQLCHAR*)sql, SQL_NTS);
	if (retcode == SQL_SUCCESS_WITH_INFO) {
		static SQLCHAR sqlstate[10];
		static SQLCHAR message[100];

		printf("hs_odbc_execute: SQLExecute - SQL_SUCCESS_WITH_INFO\n");
		SQLGetDiagRec(SQL_HANDLE_STMT, odbc->hstmt, 1, sqlstate, NULL, message, sizeof(message), NULL);
		printf("hs_odbc_execute: [%s - %s]\n", sqlstate, message);
	} else if (retcode != SQL_SUCCESS) {
		printf("hs_odbc_execute: 执行失败(retcode:%d)\n", retcode);
		return false;
	}

	printf("hs_odbc_execute: 执行成功\n");
	return true;
}

bool
hs_odbc_fetch_data(struct hs_odbc* odbc)
{
	if (!odbc)
		return false;

	return SQLFetch(odbc->hstmt) == SQL_SUCCESS;
}

bool
hs_odbc_data_row_count(struct hs_odbc* odbc, long* count)
{
	if (!odbc)
		return false;

	SQLLEN c = 0;
	if (SQLRowCount(odbc->hstmt, &c) == SQL_SUCCESS) {
		if (count) {
			*count = c;
		}
		return true;
	}

	return false;
}

bool
hs_odbc_data_string(struct hs_odbc* odbc, unsigned short column, char* string, size_t len)
{
	if (!odbc)
		return false;

	return SQLGetData(odbc->hstmt, column, SQL_C_CHAR, string, len, NULL) == SQL_SUCCESS;
}

bool
hs_odbc_data_short(struct hs_odbc* odbc, unsigned short column, short* value)
{
	if (!odbc)
		return false;

	return SQLGetData(odbc->hstmt, column, SQL_C_SHORT, value, 0, NULL) == SQL_SUCCESS;
}

bool
hs_odbc_data_double(struct hs_odbc* odbc, unsigned short column, double* value)
{
	if (!odbc)
		return false;

	return SQLGetData(odbc->hstmt, column, SQL_C_DOUBLE, value, 0, NULL) == SQL_SUCCESS;
}

bool
hs_odbc_data_float(struct hs_odbc* odbc, unsigned short column, float* value)
{
	if (!odbc)
		return false;

	return SQLGetData(odbc->hstmt, column, SQL_C_FLOAT, value, 0, NULL) == SQL_SUCCESS;
}

bool
hs_odbc_data_long(struct hs_odbc* odbc, unsigned short column, long* value)
{
	if (!odbc)
		return false;

	return SQLGetData(odbc->hstmt, column, SQL_C_LONG, value, 0, NULL) == SQL_SUCCESS;
}

bool
hs_odbc_data_date(struct hs_odbc* odbc, unsigned short column, struct hs_odbc_date* date)
{
	if (!odbc)
		return false;

	DATE_STRUCT odbc_date;
	SQLRETURN retcode = SQLGetData(odbc->hstmt, column, SQL_C_DATE, &odbc_date, sizeof(odbc_date), NULL);
	if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
		printf("hs_odbc_data_date: 获取日期数据错误(retcode:%d)\n", retcode);
		return false;
	}

	return true;
}

bool
hs_odbc_data_time(struct hs_odbc* odbc, unsigned short column, struct hs_odbc_time* time)
{
	if (!odbc)
		return false;

	TIME_STRUCT odbc_time;
	SQLRETURN retcode = SQLGetData(odbc->hstmt, column, SQL_C_TIME, &odbc_time, sizeof(odbc_time), NULL);
	if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
		printf("hs_odbc_data_time: 获取时间数据失败(retcode:%d)\n", retcode);
		return false;
	}

	return true;
}

bool
hs_odbc_data_timestamp(struct hs_odbc* odbc, unsigned short column, struct hs_odbc_timestamp* timestamp)
{
	if (!odbc)
		return false;

	TIMESTAMP_STRUCT odbc_timestamp;
	SQLRETURN retcode = SQLGetData(odbc->hstmt, column, SQL_C_TIMESTAMP, &odbc_timestamp, sizeof(odbc_timestamp), NULL);
	if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) {
		printf("hs_odbc_data_timestamp: 获取时间戳失败(retcode:%d)\n", retcode);
		return false;
	}

	return true;
}
