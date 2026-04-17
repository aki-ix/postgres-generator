#include <stdio.h>
#include "db.h"

int main() {
    const char* conninfo = "dbname=postgres user=aki";

    PGconn* conn = db_connect(conninfo);
    if (!conn) {
        return 1;
    }

    printf("Тест подключения прошёл успешно.\n");

    db_disconnect(conn);
    return 0;
}