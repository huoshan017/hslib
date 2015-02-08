#include "hs_odbc.h"

#include <stdio.h>
#include <string.h>
#include <memory.h>

int main() {
	struct hs_odbc* odbc = hs_odbc_connect("mysql_sc", "root", "");
	if (!odbc) {
		return -1;
	}

	const char* sql_text = "INSERT INTO hs_test.account(name, password) VALUES ('huoshan017', MD5('111111'));";
	int len = strlen(sql_text);
	if (!hs_odbc_execute(odbc, sql_text, len)) {
		//return -1;
	}

	sql_text = "SELECT id, name, password FROM hs_test.account";
	len = strlen(sql_text);
	bool ret = hs_odbc_execute(odbc, sql_text, len);
	if (!ret) {
		return -1;
	}

	long row_count = 0;
	ret = hs_odbc_data_row_count(odbc, &row_count);
	if (!ret) {
		printf("get_row_count failed\n");
		return -1;
	}

	printf("get_row_count = %d\n", row_count);

	long id = 0;
	char buffer[64];
	int i = 1;
	while (row_count > 0) {
		hs_odbc_fetch_data(odbc);
		hs_odbc_data_long(odbc, 1, &id);
		printf("id = %d", id);
		hs_odbc_data_string(odbc, 2, buffer, sizeof(buffer)-1);
		printf(" name = %s", buffer);
		hs_odbc_data_string(odbc, 3, buffer, sizeof(buffer)-1);
		printf(" password = %s\n", buffer);
		row_count -= 1;
	}

	/*while (1) {
		usleep(1000);
	}*/

	hs_odbc_disconnect(odbc, true);

	return 0;
}
