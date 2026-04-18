#include <iostream>
#include <string>
#include "db.h"

PGconn* db_connect(const char* conninfo) {
    PGconn* conn = PQconnectdb(conninfo);

    if (PQstatus(conn) != CONNECTION_OK) {
        std::cerr << "Ошибка подключения: " << PQerrorMessage(conn) << '\n';
        PQfinish(conn);
        return nullptr;
    }

    std::cout << "Подключение успешно!\n";
    return conn;
}

void db_disconnect(PGconn* conn) {
    if (conn) {
        PQfinish(conn);
    }
    std::cout << "Соединение закрыто.\n";
}

PGresult* db_sql(PGconn* conn, const char* sql) {
    PGresult* res = PQexec(conn, sql);

    ExecStatusType status = PQresultStatus(res);
    if (status != PGRES_TUPLES_OK && status != PGRES_COMMAND_OK) {
        std::cerr << "Ошибка выполнения запроса: " << PQerrorMessage(conn) << '\n';
        PQclear(res);
        return nullptr;
    }

    return res;
}

bool table_exists(PGconn* conn, const std::string& table_name) {
    std::string sql =
        "SELECT EXISTS ("
        "SELECT 1 FROM information_schema.tables "
        "WHERE table_schema = 'public' "
        "AND table_name = '" + table_name + "');";

    PGresult* res = db_sql(conn, sql.c_str());
    if (!res) {
        return false;
    }

    char* value = PQgetvalue(res, 0, 0);
    bool exists = (value[0] == 't');

    PQclear(res);
    return exists;
}

bool create_test_table(PGconn* conn, const std::string& table_name) {
    std::string sql =
        "CREATE TABLE IF NOT EXISTS " + table_name + " ("
        "id BIGSERIAL PRIMARY KEY, "
        "created_at TIMESTAMP DEFAULT NOW(), "
        "data TEXT NOT NULL"
        ");";

    PGresult* res = db_sql(conn, sql.c_str());
    if (!res) {
        return false;
    }

    PQclear(res);
    return true;
}

long long get_table_size_bytes(PGconn* conn, const std::string& table_name) {
    std::string sql =
        "SELECT pg_total_relation_size('" + table_name + "');";

    PGresult* sql_result = db_sql(conn, sql.c_str());
    if (!sql_result) {
        return -1;
    }

    char* size_text = PQgetvalue(sql_result, 0, 0);
    long long table_size_bytes = atoll(size_text);

    PQclear(sql_result);
    return table_size_bytes;
}