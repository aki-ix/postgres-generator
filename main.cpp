#include <iostream>
#include <string>
#include <cctype>
#include <thread>
#include <chrono>
#include "db.h"

bool is_valid_name(const std::string& text) {
    if (text.empty()) {
        return false;
    }

    for (char ch : text) {
        if (!std::isalnum(static_cast<unsigned char>(ch)) && ch != '_') {
            return false;
        }
    }

    return true;
}

std::string read_required_name(const std::string& prompt, const std::string& default_value) {
    std::string input;

    while (true) {
        std::cout << prompt << " (по умолчанию " << default_value << "): ";
        std::getline(std::cin, input);

        if (input.empty()) {
            return default_value;
        }

        if (is_valid_name(input)) {
            return input;
        }

        std::cout << "[ERROR] Некорректный ввод. Используйте только латинские буквы, цифры и символ _. Попробуйте снова.\n";
    }
}

std::string read_optional_name(const std::string& prompt) {
    std::string input;

    while (true) {
        std::cout << prompt;
        std::getline(std::cin, input);

        if (input.empty()) {
            return "";
        }

        if (is_valid_name(input)) {
            return input;
        }

        std::cout << "[ERROR] Некорректный ввод. Используйте только латинские буквы, цифры и символ _. Попробуйте снова.\n";
    }
}

double read_positive_double(const std::string& prompt, double default_value) {
    std::string input;

    while (true) {
        std::cout << prompt << " (по умолчанию " << default_value << "): ";
        std::getline(std::cin, input);

        if (input.empty()) {
            return default_value;
        }

        try {
            double value = std::stod(input);

            if (value > 0.0) {
                return value;
            }

            std::cout << "[ERROR] Число должно быть больше нуля. Попробуйте снова.\n";
        } catch (...) {
            std::cout << "[ERROR] Нужно ввести число. Попробуйте снова.\n";
        }
    }
}

int read_positive_int(const std::string& prompt, int default_value) {
    std::string input;

    while (true) {
        std::cout << prompt << " (по умолчанию " << default_value << "): ";
        std::getline(std::cin, input);

        if (input.empty()) {
            return default_value;
        }

        try {
            int value = std::stoi(input);

            if (value > 0) {
                return value;
            }

            std::cout << "[ERROR] Число должно быть больше нуля. Попробуйте снова.\n";
        } catch (...) {
            std::cout << "[ERROR] Нужно ввести целое число. Попробуйте снова.\n";
        }
    }
}

int main() {
    std::cout << "=== PostgreSQL Generator ===\n\n";

    std::string dbname = read_required_name(
        "Имя базы данных",
        "gen_test_db"
    );

    std::string user = read_optional_name(
        "Имя пользователя PostgreSQL (оставьте пустым для системного пользователя): "
    );

    std::string table_name = read_required_name(
        "Имя тестовой таблицы",
        "pggen_data"
    );

    double min_table_size_mb = read_positive_double(
        "Минимальный размер таблицы в MB",
        1.0
    );

    int max_iterations = read_positive_int(
        "Лимит итераций",
        500
    );

    std::string conninfo = "dbname=" + dbname;
    if (!user.empty()) {
        conninfo += " user=" + user;
    }

    std::cout << "\n[INFO] Конфигурация запуска:\n";
    std::cout << "[INFO] База данных: " << dbname << "\n";

    if (user.empty()) {
        std::cout << "[INFO] Пользователь: (системный)\n";
    } else {
        std::cout << "[INFO] Пользователь: " << user << "\n";
    }

    std::cout << "[INFO] Таблица: " << table_name << "\n";
    std::cout << "[INFO] Минимальный размер: " << min_table_size_mb << " MB\n";
    std::cout << "[INFO] Лимит итераций: " << max_iterations << "\n";

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

    std::cout << "[INFO] Начинаем генерацию нагрузки до достижения "
              << min_table_size_mb << " MB...\n";

    int iteration = 1;

    while (table_size_mb < min_table_size_mb && iteration <= max_iterations) {
        std::cout << "[INFO] Итерация " << iteration
                  << ": выполняется INSERT...\n" << std::flush;

        std::string sql =
            "INSERT INTO " + table_name +
            " (data) VALUES ("
            "md5(random()::text) || md5(random()::text) || "
            "md5(random()::text) || md5(random()::text) || "
            "md5(random()::text) || md5(random()::text) || "
            "md5(random()::text) || md5(random()::text)"
            ");";

        PGresult* sql_result = db_sql(conn, sql.c_str());
        if (!sql_result) {
            std::cout << "[ERROR] Ошибка при выполнении INSERT.\n";
            db_disconnect(conn);
            return 1;
        }

        PQclear(sql_result);

        table_size_bytes = get_table_size_bytes(conn, table_name);
        if (table_size_bytes < 0) {
            std::cout << "[ERROR] Не удалось получить размер таблицы.\n";
            db_disconnect(conn);
            return 1;
        }

        table_size_mb = table_size_bytes / (1024.0 * 1024.0);

        std::cout << "[INFO] Текущий размер таблицы: "
                  << table_size_bytes << " bytes ("
                  << table_size_mb << " MB)\n";

        std::cout << "[OK] Итерация " << iteration << " завершена.\n";

        iteration++;

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    if (table_size_mb >= min_table_size_mb) {
        std::cout << "[OK] Минимальный размер таблицы достигнут.\n";
    } else {
        std::cout << "[WARN] Достигнут лимит итераций до достижения целевого размера.\n";
    }

    db_disconnect(conn);
    std::cout << "[INFO] Работа программы завершена.\n";

    return 0;
}