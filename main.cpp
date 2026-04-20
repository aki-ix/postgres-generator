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

int read_menu_choice(const std::string& prompt, int min_value, int max_value) {
    std::string input;

    while (true) {
        std::cout << prompt;
        std::getline(std::cin, input);

        try {
            int value = std::stoi(input);

            if (value >= min_value && value <= max_value) {
                return value;
            }

            std::cout << "[ERROR] Выберите число от " << min_value
                      << " до " << max_value << ". Попробуйте снова.\n";
        } catch (...) {
            std::cout << "[ERROR] Нужно ввести число. Попробуйте снова.\n";
        }
    }
}

std::string build_insert_sql(const std::string& table_name) {
    return
        "INSERT INTO " + table_name +
        " (data) VALUES ("
        "md5(random()::text) || md5(random()::text) || "
        "md5(random()::text) || md5(random()::text) || "
        "md5(random()::text) || md5(random()::text) || "
        "md5(random()::text) || md5(random()::text)"
        ");";
}

std::string build_select_sql(const std::string& table_name) {
    return
        "SELECT * FROM " + table_name +
        " ORDER BY id DESC LIMIT 10;";
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

    int program_mode = read_menu_choice(
        "\nВыберите режим работы:\n"
        "1 - подготовка таблицы до нужного размера\n"
        "2 - тест нагрузки\n"
        "Ваш выбор: ",
        1,
        2
    );

    int load_type = 1;
    if (program_mode == 2) {
        load_type = read_menu_choice(
            "\nВыберите тип нагрузки:\n"
            "1 - INSERT\n"
            "2 - SELECT\n"
            "3 - MIXED\n"
            "Ваш выбор: ",
            1,
            3
        );
    }

    double min_table_size_mb = 1.0;
    int max_iterations = 500;
    int test_duration_seconds = 30;

    if (program_mode == 1) {
        min_table_size_mb = read_positive_double(
            "Минимальный размер таблицы в MB",
            1.0
        );

        max_iterations = read_positive_int(
            "Лимит итераций",
            500
        );
    } else {
        int stop_mode = read_menu_choice(
            "\nВыберите условие остановки теста:\n"
            "1 - по времени\n"
            "2 - по количеству итераций\n"
            "Ваш выбор: ",
            1,
            2
        );

        if (stop_mode == 1) {
            test_duration_seconds = read_positive_int(
                "Длительность теста в секундах",
                30
            );
        } else {
            max_iterations = read_positive_int(
                "Количество итераций",
                500
            );
        }

        std::string conninfo_preview = "dbname=" + dbname;
        if (!user.empty()) {
            conninfo_preview += " user=" + user;
        }

        std::cout << "\n[INFO] Конфигурация запуска:\n";
        std::cout << "[INFO] База данных: " << dbname << "\n";

        if (user.empty()) {
            std::cout << "[INFO] Пользователь: (системный)\n";
        } else {
            std::cout << "[INFO] Пользователь: " << user << "\n";
        }

        std::cout << "[INFO] Таблица: " << table_name << "\n";
        std::cout << "[INFO] Режим: LOAD TEST\n";

        if (load_type == 1) {
            std::cout << "[INFO] Тип нагрузки: INSERT\n";
        } else if (load_type == 2) {
            std::cout << "[INFO] Тип нагрузки: SELECT\n";
        } else {
            std::cout << "[INFO] Тип нагрузки: MIXED\n";
        }

        if (stop_mode == 1) {
            std::cout << "[INFO] Условие остановки: по времени\n";
            std::cout << "[INFO] Длительность: " << test_duration_seconds << " sec\n";
        } else {
            std::cout << "[INFO] Условие остановки: по итерациям\n";
            std::cout << "[INFO] Лимит итераций: " << max_iterations << "\n";
        }

        std::cout << "\n[INFO] Подключение к PostgreSQL...\n";
        PGconn* conn = db_connect(conninfo_preview.c_str());
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

        std::cout << "[INFO] Начальный размер таблицы: "
                  << table_size_bytes << " bytes ("<< table_size_mb << " MB)\n";

        std::cout << "[INFO] Запускаем тест нагрузки...\n";

        int iteration = 1;
        int total_queries = 0;
        int stop_mode_copy = stop_mode;

        auto test_start = std::chrono::steady_clock::now();

        while (true) {
            if (stop_mode_copy == 1) {
                auto now = std::chrono::steady_clock::now();
                auto elapsed_seconds =
                    std::chrono::duration_cast<std::chrono::seconds>(now - test_start).count();

                if (elapsed_seconds >= test_duration_seconds) {
                    break;
                }
            } else {
                if (iteration > max_iterations) {
                    break;
                }
            }

            std::string sql;
            std::string current_action;

            if (load_type == 1) {
                sql = build_insert_sql(table_name);
                current_action = "INSERT";
            } else if (load_type == 2) {
                sql = build_select_sql(table_name);
                current_action = "SELECT";
            } else {
                if (iteration % 3 == 0) {
                    sql = build_select_sql(table_name);
                    current_action = "SELECT";
                } else {
                    sql = build_insert_sql(table_name);
                    current_action = "INSERT";
                }
            }

            std::cout << "[INFO] Итерация " << iteration
                      << ": выполняется " << current_action << "...\n";

            PGresult* sql_result = db_sql(conn, sql.c_str());
            if (!sql_result) {
                std::cout << "[ERROR] Ошибка при выполнении запроса.\n";
                db_disconnect(conn);
                return 1;
            }

            PQclear(sql_result);

            total_queries++;

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

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        auto test_end = std::chrono::steady_clock::now();
        double total_time_seconds =
            std::chrono::duration_cast<std::chrono::milliseconds>(test_end - test_start).count() / 1000.0;

        double queries_per_second = 0.0;
        if (total_time_seconds > 0.0) {
            queries_per_second = total_queries / total_time_seconds;
        }

        std::cout << "\n[REPORT] Тест завершён.\n";

        if (load_type == 1) {
            std::cout << "[REPORT] Тип нагрузки: INSERT\n";
        } else if (load_type == 2) {
            std::cout << "[REPORT] Тип нагрузки: SELECT\n";
        } else {
            std::cout << "[REPORT] Тип нагрузки: MIXED\n";
        }

        if (stop_mode_copy == 1) {
            std::cout << "[REPORT] Условие остановки: по времени\n";
        } else {
            std::cout << "[REPORT] Условие остановки: по итерациям\n";
        }

        std::cout << "[REPORT] Общее время: " << total_time_seconds << " сек\n";
        std::cout << "[REPORT] Выполнено запросов: " << total_queries << "\n";
        std::cout << "[REPORT] Итоговый размер таблицы: " << table_size_mb << " MB\n";
        std::cout << "[REPORT] Производительность: " << queries_per_second << " запросов/сек\n";

        db_disconnect(conn);
        std::cout << "[INFO] Работа программы завершена.\n";
        return 0;
    }

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
    std::cout << "[INFO] Режим: PREPARE TABLE\n";
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

    std::cout << "[INFO] Начинаем подготовку таблицы до достижения "
              << min_table_size_mb << " MB...\n";

    int iteration = 1;

    while (table_size_mb < min_table_size_mb && iteration <= max_iterations) {
        std::cout << "[INFO] Итерация " << iteration
                  << ": выполняется INSERT...\n" << std::flush;

        std::string sql = build_insert_sql(table_name);

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

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
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