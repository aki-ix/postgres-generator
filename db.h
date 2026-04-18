#ifndef DB_H
#define DB_H

#include <string>

extern "C" {
#include <libpq-fe.h>
}

PGconn* db_connect(const char* conninfo);
void db_disconnect(PGconn* conn);
PGresult* db_sql(PGconn* conn, const char* sql);

bool table_exists(PGconn* conn, const std::string& table_name);
bool create_test_table(PGconn* conn, const std::string& table_name);

#endif