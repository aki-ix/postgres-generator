#include <stdio.h>
#include "db.h"

PGconn* db_connect(const char* conninfo) {
    PGconn* conn = PQconnectdb(conninfo);

    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "Ошибка подключения: %s\n", PQerrorMessage(conn));
        PQfinish(conn);
        return NULL;
    }

    printf("Подключение успешно!\n");
    return conn;
}

void db_disconnect(PGconn* conn) {
    PQfinish(conn);
    printf("Соединение закрыто.\n");
}