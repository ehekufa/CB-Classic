// fbnet.hpp — настоящая сеть: Firebase Realtime Database по её REST-протоколу.
//
// Зачем свой транспорт, а не SDK: Firebase SDK на Android — это Gradle-зависимость
// `firebase-database` + `google-services.json` (ключ проекта), а игра собирается
// без Gradle-плагинов Google. REST-протокол RTDB — официальный и не требует ключей:
// он ходит по тем же правилам базы, что и SDK (см. FIREBASE_RULES.md).
//
// Устройство:
//   * записи (`PUT`/`PATCH`) складываются в очередь и уходят пачкой, чтобы не
//     дёргать сервер на каждый кадр;
//   * чтение — опрос узлов (`GET <узел>.json`): движок подписывается на узел
//     один раз, а результат раскладывается по ключам;
//   * всё сетевое живёт в отдельном потоке, кадр никогда не ждёт сеть.
#ifndef CB_FBNET_HPP
#define CB_FBNET_HPP

#include <condition_variable>
#include <deque>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <vector>

namespace cb {

// Значение, пришедшее с сервера: полный путь и JSON-текст ("12", "\"Гость\"").
struct FbUpdate {
    std::string path;
    std::string json;
};

class FbNet {
public:
    FbNet() = default;
    ~FbNet();
    FbNet(const FbNet &) = delete;
    FbNet &operator=(const FbNet &) = delete;

    // База проекта; можно менять из игры/CLI, пока сеть не запущена.
    static std::string default_url();

    void configure(const std::string &base);      // "https://host/" (слэш на конце не обязателен)
    std::string base_url() const;

    void set_enabled(bool on);                    // поднять/остановить рабочий поток
    bool enabled() const { return enabled_; }

    // Записать значение по пути (json — готовый JSON: 12 или "текст").
    void put(const std::string &path, const std::string &json);
    // Подписаться на узел целиком (один запрос на узел за цикл опроса).
    void watch(const std::string &node);

    // Забрать пришедшее обновление (не блокирует, вызывается из кадра).
    bool pop(FbUpdate *out);

    int requests() const { return requests_; }
    int failures() const { return failures_; }
    std::string last_error() const;

private:
    void loop();
    void send_puts(std::map<std::string, std::string> &batch);
    void fetch_node(const std::string &node);
    bool request(const std::string &method, const std::string &url, const std::string &body,
                 std::string *response);

    mutable std::mutex mu_;
    std::string base_;
    bool enabled_ = false;
    std::thread worker_;
    std::condition_variable cv_;
    bool stop_ = false;
    long long next_send_ = 0;
    std::map<std::string, std::string> puts_;          // путь -> json, ещё не отправлено
    std::set<std::string> watches_;                    // узлы, за которыми следим
    std::map<std::string, long long> watch_due_;       // когда опрашивать (мс)
    std::deque<FbUpdate> updates_;
    std::string last_error_;
    int requests_ = 0;
    int failures_ = 0;
};

// JSON-представление значения из игры: числа и true/false/null — как есть,
// всё остальное — строкой в кавычках.
std::string fb_json_of(const std::string &value);
// Обратно: JSON-значение -> обычный текст (для хранения в игре).
std::string fb_text_of(const std::string &json);
// Разложить ответ вида {"X1":10,"Y1":-20} в список пар «путь = значение».
void fb_flatten(const std::string &node, const std::string &json, std::vector<FbUpdate> *out);

}  // namespace cb

#endif  // CB_FBNET_HPP
