#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include "db.h"

int main() {
    std::string dbname = "gen_test_db";
    std::string user = "aki";
    std::string table_name = "pggen_data";
    std::string input;

    std::cout << "=== PostgreSQL Generator ===\n\n";

    std::cout << "Имя базы данных (по умолчанию gen_test_db): ";
    std::getline(std::cin, input);
    if (!input.empty()) {
        dbname = input;
    }

    std::cout << "Имя пользователя (по умолчанию aki): ";
    std::getline(std::cin, input);
    if (!input.empty()) {
        user = input;
    }

    std::cout << "Имя тестовой таблицы (по умолчанию pggen_data): ";
    std::getline(std::cin, input);
    if (!input.empty()) {
        table_name = input;
    }

    std::string conninfo = "dbname=" + dbname + " user=" + user;

    std::cout << "\n[INFO] Подключение к PostgreSQL...\n";
    PGconn* conn = db_connect(conninfo.c_str());
    if (!conn) {
        std::cout << "[ERROR] Не удалось подключиться к базе данных.\n";
        return 1;
    }
    std::cout << "[OK] Подключение успешно.\n";

    std::cout << "[INFO] Проверка существования таблицы " << table_name << "...\n";
    bool exists = table_exists(conn, table_name);

    if (exists) {
        std::cout << "[OK] Таблица уже существует.\n";
    } else {
        std::cout << "[INFO] Таблица не найдена. Создание...\n";
        if (!create_test_table(conn, table_name)) {
            std::cout << "[ERROR] Не удалось создать таблицу.\n";
            db_disconnect(conn);
            return 1;
        }
        std::cout << "[OK] Таблица успешно создана.\n";
    }

    long long table_size_bytes = get_table_size_bytes(conn, table_name);

    if (table_size_bytes < 0) {
        std::cout << "[ERROR] Не удалось получить размер таблицы.\n";
        db_disconnect(conn);
        return 1;
    }

    double table_size_mb = table_size_bytes / (1024.0 * 1024.0);
    std::cout << "[INFO] Текущий размер таблицы: "
          << table_size_bytes << " bytes ("
          << table_size_mb << " MB)\n";
    std::cout << "[INFO] Начинаем тестовую генерацию нагрузки...\n";

    for (int i = 1; i <= 10; i++) {
        std::cout << "[INFO] Итерация " << i << ": выполняется INSERT...\n" << std::flush;

        std::string sql =
            "INSERT INTO " + table_name +
            " (data) VALUES ("
            "md5(random()::text) || md5(random()::text) || "
            "md5(random()::text) || md5(random()::text) || "
            "md5(random()::text) || md5(random()::text) || "
            "md5(random()::text) || md5(random()::text)"
            ");";

        PGresult* res = db_sql(conn, sql.c_str());
        if (!res) {
            std::cout << "[ERROR] Ошибка при выполнении INSERT.\n";
            db_disconnect(conn);
            return 1;
        }

        PQclear(res);

        long long table_size_bytes = get_table_size_bytes(conn, table_name);
        if (table_size_bytes < 0) {
            std::cout << "[ERROR] Не удалось получить размер таблицы.\n";
            db_disconnect(conn);
            return 1;
        }

        double table_size_mb = table_size_bytes / (1024.0 * 1024.0);
        std::cout << "[INFO] Текущий размер таблицы: "
          << table_size_bytes << " bytes ("
          << table_size_mb << " MB)\n";

        std::cout << "[OK] Итерация " << i << " завершена.\n" << std::flush;

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    db_disconnect(conn);
    std::cout << "[INFO] Работа программы завершена.\n";

    return 0;
}