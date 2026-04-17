#ifndef DB_H
#define DB_H

#include <libpq-fe.h>

PGconn* db_connect(const char* conninfo);
void db_disconnect(PGconn* conn);

#endif