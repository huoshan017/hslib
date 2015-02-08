#ifndef __HS_ODBC_H__
#define __HS_ODBC_H__

#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

struct hs_odbc_date {
	short year;
	unsigned char month;
	unsigned char day;
};

struct hs_odbc_time {
	unsigned char hour;
	unsigned char minute;
	unsigned char second;
};

struct hs_odbc_timestamp {
	short year;
	unsigned char month;
	unsigned char day;
	unsigned char hour;
	unsigned char minute;
	unsigned char second;
	unsigned int fraction;
};

struct hs_odbc {
	SQLHENV henv;
	SQLHDBC hdbc;
	SQLHSTMT hstmt;
};

struct hs_odbc*
hs_odbc_connect(const char* server_name, const char* user_name, const char* password);

void
hs_odbc_disconnect(struct hs_odbc* odbc, bool destroy);

bool
hs_odbc_execute(struct hs_odbc* odbc, const char* sql, int sql_len);

bool
hs_odbc_fetch_data(struct hs_odbc* odbc);

bool
hs_odbc_data_row_count(struct hs_odbc* odbc, long* count);

bool
hs_odbc_data_string(struct hs_odbc* odbc, unsigned short column, char* string, size_t len);

bool
hs_odbc_data_short(struct hs_odbc* odbc, unsigned short column, short* value);

bool
hs_odbc_data_double(struct hs_odbc* odbc, unsigned short column, double* value);

bool
hs_odbc_data_float(struct hs_odbc* odbc, unsigned short column, float* value);

bool
hs_odbc_data_long(struct hs_odbc* odbc, unsigned short column, long* value);

bool
hs_odbc_data_date(struct hs_odbc* odbc, unsigned short column, struct hs_odbc_date* date);

bool
hs_odbc_data_time(struct hs_odbc* odbc, unsigned short column, struct hs_odbc_time* time);

bool
hs_odbc_data_timestamp(struct hs_odbc* odbc, unsigned short column, struct hs_odbc_timestamp* timestamp);

#endif
