// fbnet.cpp — сетевой слой Firebase Realtime Database (REST).
//
// Транспорт:
//   Android — Java HttpURLConnection через JNI (там же лежит TLS и системный
//             прокси; см. android/app/src/main/java/org/cbclassic/FirebaseNet.java);
//   десктоп — обычный HTTP/1.1 по сокетам (для локального эмулятора в тестах,
//             tests/firebase_mock.py). HTTPS на десктопе не поддерживается.
#include "fbnet.hpp"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <sstream>

#if defined(__ANDROID__) && defined(CB_USE_SDL2)
#include <jni.h>
#include <SDL2/SDL_system.h>
#elif !defined(__ANDROID__)
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace cb {
namespace {

constexpr int kTickMs = 60;        // шаг рабочего потока
constexpr int kPollMs = 350;       // как часто опрашивать узел
constexpr int kSendMs = 150;       // не чаще одной пачки записей на 150 мс
constexpr int kRequestTimeoutMs = 4000;

long long now_ms()
{
    using namespace std::chrono;
    static const steady_clock::time_point start = steady_clock::now();
    return duration_cast<milliseconds>(steady_clock::now() - start).count();
}

std::string trimmed(const std::string &s)
{
    size_t b = 0;
    size_t e = s.size();
    while (b < e && std::isspace(static_cast<unsigned char>(s[b]))) {
        ++b;
    }
    while (e > b && std::isspace(static_cast<unsigned char>(s[e - 1]))) {
        --e;
    }
    return s.substr(b, e - b);
}

bool looks_like_number(const std::string &s)
{
    if (s.empty()) {
        return false;
    }
    char *end = nullptr;
    std::strtod(s.c_str(), &end);
    return end && *end == '\0';
}

// Кодирование пути для URL: сегменты разделяются '/', остальное — %XX.
std::string encode_path(const std::string &path)
{
    static const char *hex = "0123456789ABCDEF";
    std::string out;
    for (unsigned char c : path) {
        if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~' || c == '/') {
            out.push_back(static_cast<char>(c));
        } else {
            out.push_back('%');
            out.push_back(hex[c >> 4]);
            out.push_back(hex[c & 0x0f]);
        }
    }
    return out;
}

std::string percent_decode(const std::string &s)
{
    std::string out;
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '%' && i + 2 < s.size()) {
            auto digit = [](char c) -> int {
                if (c >= '0' && c <= '9') return c - '0';
                if (c >= 'a' && c <= 'f') return c - 'a' + 10;
                if (c >= 'A' && c <= 'F') return c - 'A' + 10;
                return -1;
            };
            int hi = digit(s[i + 1]);
            int lo = digit(s[i + 2]);
            if (hi >= 0 && lo >= 0) {
                out.push_back(static_cast<char>(hi * 16 + lo));
                i += 2;
                continue;
            }
        }
        out.push_back(s[i]);
    }
    return out;
}

void skip_ws(const std::string &s, size_t &i)
{
    while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) {
        ++i;
    }
}

// Строковый литерал JSON целиком, вместе с кавычками.
std::string take_string(const std::string &s, size_t &i)
{
    size_t start = i;
    ++i;  // открывающая кавычка
    while (i < s.size()) {
        if (s[i] == '\\') {
            i += 2;
            continue;
        }
        if (s[i] == '"') {
            ++i;
            break;
        }
        ++i;
    }
    if (i > s.size()) {
        i = s.size();
    }
    return s.substr(start, i - start);
}

void append_utf8(std::string *out, unsigned cp)
{
    if (cp < 0x80) {
        out->push_back(static_cast<char>(cp));
    } else if (cp < 0x800) {
        out->push_back(static_cast<char>(0xc0 | (cp >> 6)));
        out->push_back(static_cast<char>(0x80 | (cp & 0x3f)));
    } else if (cp < 0x10000) {
        out->push_back(static_cast<char>(0xe0 | (cp >> 12)));
        out->push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3f)));
        out->push_back(static_cast<char>(0x80 | (cp & 0x3f)));
    } else {
        out->push_back(static_cast<char>(0xf0 | (cp >> 18)));
        out->push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3f)));
        out->push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3f)));
        out->push_back(static_cast<char>(0x80 | (cp & 0x3f)));
    }
}

std::string unquote(const std::string &raw)
{
    if (raw.size() < 2 || raw.front() != '"') {
        return raw;
    }
    std::string out;
    for (size_t i = 1; i + 1 < raw.size(); ++i) {
        char c = raw[i];
        if (c != '\\' || i + 2 >= raw.size()) {
            out.push_back(c);
            continue;
        }
        char n = raw[++i];
        switch (n) {
        case 'n': out.push_back('\n'); break;
        case 't': out.push_back('\t'); break;
        case 'r': out.push_back('\r'); break;
        case 'b': out.push_back('\b'); break;
        case 'f': out.push_back('\f'); break;
        case 'u': {
            if (i + 4 < raw.size()) {
                unsigned cp = 0;
                for (int k = 1; k <= 4; ++k) {
                    char h = raw[i + k];
                    cp <<= 4;
                    if (h >= '0' && h <= '9') cp |= static_cast<unsigned>(h - '0');
                    else if (h >= 'a' && h <= 'f') cp |= static_cast<unsigned>(h - 'a' + 10);
                    else if (h >= 'A' && h <= 'F') cp |= static_cast<unsigned>(h - 'A' + 10);
                }
                i += 4;
                append_utf8(&out, cp);
            }
            break;
        }
        default: out.push_back(n); break;
        }
    }
    return out;
}

void flatten_value(const std::string &prefix, const std::string &s, size_t &i,
                   std::vector<FbUpdate> *out)
{
    skip_ws(s, i);
    if (i >= s.size()) {
        return;
    }
    const char c = s[i];
    if (c == '{') {
        ++i;
        skip_ws(s, i);
        if (i < s.size() && s[i] == '}') {
            ++i;
            return;
        }
        while (i < s.size()) {
            skip_ws(s, i);
            if (i >= s.size() || s[i] != '"') {
                break;
            }
            const std::string key = unquote(take_string(s, i));
            skip_ws(s, i);
            if (i < s.size() && s[i] == ':') {
                ++i;
            }
            const std::string path = prefix.empty() ? key : prefix + "/" + key;
            flatten_value(path, s, i, out);
            skip_ws(s, i);
            if (i < s.size() && s[i] == ',') {
                ++i;
                continue;
            }
            if (i < s.size() && s[i] == '}') {
                ++i;
            }
            break;
        }
        return;
    }
    if (c == '[') {
        // массивов в базе игры нет — отдаём как есть, одним значением
        const size_t start = i;
        int depth = 0;
        while (i < s.size()) {
            if (s[i] == '[') {
                ++depth;
            } else if (s[i] == ']') {
                --depth;
                if (depth == 0) {
                    ++i;
                    break;
                }
            }
            ++i;
        }
        if (!prefix.empty()) {
            out->push_back(FbUpdate{prefix, s.substr(start, i - start)});
        }
        return;
    }
    if (c == '"') {
        if (!prefix.empty()) {
            out->push_back(FbUpdate{prefix, take_string(s, i)});
        } else {
            take_string(s, i);
        }
        return;
    }
    const size_t start = i;
    while (i < s.size() && s[i] != ',' && s[i] != '}' && s[i] != ']') {
        ++i;
    }
    const std::string token = trimmed(s.substr(start, i - start));
    if (token == "null") {
        return;  // «нет значения» — как отсутствие ключа
    }
    if (!token.empty() && !prefix.empty()) {
        out->push_back(FbUpdate{prefix, token});
    }
}

// --- транспорт -----------------------------------------------------------

#if defined(__ANDROID__) && defined(CB_USE_SDL2)
// Java-мост: статический FirebaseNet.request(method, url, body) -> тело ответа
// (или null при ошибке). Класс ищем через ClassLoader активности, потому что
// из нативного потока FindClass не видит классы приложения.
bool java_request(const std::string &method, const std::string &url, const std::string &body,
                  std::string *response, std::string *error)
{
    JNIEnv *env = static_cast<JNIEnv *>(SDL_AndroidGetJNIEnv());
    jobject activity = static_cast<jobject>(SDL_AndroidGetActivity());
    if (!env || !activity) {
        *error = "нет JNI-окружения";
        return false;
    }
    if (env->PushLocalFrame(16) != 0) {
        *error = "нет локального кадра JNI";
        return false;
    }
    bool ok = false;
    jclass cls = nullptr;
    for (int attempt = 0; attempt < 2 && !cls; ++attempt) {
        jclass cls_act = env->GetObjectClass(activity);
        jmethodID mid_loader = env->GetMethodID(cls_act, "getClassLoader", "()Ljava/lang/ClassLoader;");
        jobject loader = mid_loader ? env->CallObjectMethod(activity, mid_loader) : nullptr;
        if (loader) {
            jclass cls_loader = env->GetObjectClass(loader);
            jmethodID mid_load = env->GetMethodID(cls_loader, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
            if (mid_load) {
                jstring name = env->NewStringUTF("org.cbclassic.FirebaseNet");
                cls = static_cast<jclass>(env->CallObjectMethod(loader, mid_load, name));
            }
        }
        if (env->ExceptionCheck()) {
            env->ExceptionClear();
            cls = nullptr;
        }
        if (!cls) {
            break;
        }
    }
    if (cls) {
        jmethodID mid = env->GetStaticMethodID(cls, "request",
                                               "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;");
        if (env->ExceptionCheck()) {
            env->ExceptionClear();
            mid = nullptr;
        }
        if (mid) {
            jstring jm = env->NewStringUTF(method.c_str());
            jstring ju = env->NewStringUTF(url.c_str());
            jstring jb = env->NewStringUTF(body.c_str());
            jstring jr = static_cast<jstring>(env->CallStaticObjectMethod(cls, mid, jm, ju, jb));
            if (env->ExceptionCheck()) {
                env->ExceptionClear();
                jr = nullptr;
            }
            if (jr) {
                const char *text = env->GetStringUTFChars(jr, nullptr);
                if (text) {
                    *response = text;
                    env->ReleaseStringUTFChars(jr, text);
                    ok = true;
                }
            } else {
                *error = "запрос не удался (Java)";
            }
        } else {
            *error = "нет метода FirebaseNet.request";
        }
    } else {
        *error = "класс FirebaseNet не найден";
    }
    env->PopLocalFrame(nullptr);
    return ok;
}
#else
struct UrlParts {
    bool tls = false;
    std::string host;
    int port = 80;
    std::string path = "/";
};

bool split_url(const std::string &url, UrlParts *out)
{
    size_t pos = url.find("://");
    if (pos == std::string::npos) {
        return false;
    }
    const std::string scheme = url.substr(0, pos);
    out->tls = scheme == "https";
    std::string rest = url.substr(pos + 3);
    size_t slash = rest.find('/');
    std::string hostport = slash == std::string::npos ? rest : rest.substr(0, slash);
    out->path = slash == std::string::npos ? "/" : rest.substr(slash);
    size_t colon = hostport.find(':');
    if (colon == std::string::npos) {
        out->host = hostport;
        out->port = out->tls ? 443 : 80;
    } else {
        out->host = hostport.substr(0, colon);
        out->port = std::atoi(hostport.c_str() + colon + 1);
    }
    return !out->host.empty();
}

// Простой HTTP/1.1 (Connection: close): достаточно для локального эмулятора базы.
bool socket_request(const std::string &method, const std::string &url, const std::string &body,
                    std::string *response, std::string *error)
{
    UrlParts parts;
    if (!split_url(url, &parts)) {
        *error = "плохой адрес: " + url;
        return false;
    }
    if (parts.tls) {
        *error = "https только на телефоне (на десктопе — http)";
        return false;
    }
    addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    addrinfo *info = nullptr;
    const std::string port = std::to_string(parts.port);
    if (getaddrinfo(parts.host.c_str(), port.c_str(), &hints, &info) != 0 || !info) {
        *error = "не разрешается имя " + parts.host;
        return false;
    }
    int fd = ::socket(info->ai_family, info->ai_socktype, info->ai_protocol);
    if (fd < 0) {
        freeaddrinfo(info);
        *error = "сокет не открылся";
        return false;
    }
    timeval tv{};
    tv.tv_sec = kRequestTimeoutMs / 1000;
    tv.tv_usec = (kRequestTimeoutMs % 1000) * 1000;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    if (::connect(fd, info->ai_addr, info->ai_addrlen) != 0) {
        freeaddrinfo(info);
        ::close(fd);
        *error = "нет соединения с " + parts.host;
        return false;
    }
    freeaddrinfo(info);

    std::ostringstream req;
    req << method << " " << parts.path << " HTTP/1.1\r\n"
        << "Host: " << parts.host << "\r\n"
        << "Content-Type: application/json\r\n"
        << "Connection: close\r\n"
        << "Content-Length: " << body.size() << "\r\n\r\n"
        << body;
    const std::string raw = req.str();
    size_t sent = 0;
    while (sent < raw.size()) {
        const ssize_t n = ::send(fd, raw.data() + sent, raw.size() - sent, 0);
        if (n <= 0) {
            ::close(fd);
            *error = "не отправилось";
            return false;
        }
        sent += static_cast<size_t>(n);
    }
    std::string data;
    char buf[4096];
    while (true) {
        const ssize_t n = ::recv(fd, buf, sizeof(buf), 0);
        if (n <= 0) {
            break;
        }
        data.append(buf, static_cast<size_t>(n));
    }
    ::close(fd);

    const size_t head_end = data.find("\r\n\r\n");
    if (head_end == std::string::npos) {
        *error = "пустой ответ";
        return false;
    }
    const std::string head = data.substr(0, head_end);
    const std::string payload = data.substr(head_end + 4);
    const size_t sp = head.find(' ');
    const int code = sp == std::string::npos ? 0 : std::atoi(head.c_str() + sp + 1);
    if (code < 200 || code >= 300) {
        *error = "HTTP " + std::to_string(code) + ": " + payload.substr(0, 120);
        return false;
    }
    *response = payload;
    return true;
}
#endif

}  // namespace

// --- разбор/сборка JSON ---------------------------------------------------

std::string fb_json_of(const std::string &value)
{
    using namespace cb;
    if (looks_like_number(value) || value == "true" || value == "false" || value == "null") {
        return value;
    }
    std::string out = "\"";
    for (char c : value) {
        switch (c) {
        case '"': out += "\\\""; break;
        case '\\': out += "\\\\"; break;
        case '\n': out += "\\n"; break;
        case '\r': out += "\\r"; break;
        case '\t': out += "\\t"; break;
        default: out.push_back(c); break;
        }
    }
    out += "\"";
    return out;
}

std::string fb_text_of(const std::string &json)
{
    const std::string s = trimmed(json);
    if (s.empty() || s == "null") {
        return std::string();
    }
    if (s.front() == '"') {
        return unquote(s);
    }
    return s;
}

void fb_flatten(const std::string &node, const std::string &json, std::vector<FbUpdate> *out)
{
    size_t i = 0;
    flatten_value(node, json, i, out);
}

// --- FbNet -----------------------------------------------------------------

std::string FbNet::default_url()
{
    return "https://circlebattle1-default-rtdb.firebaseio.com/";
}

FbNet::~FbNet()
{
    set_enabled(false);
}

void FbNet::configure(const std::string &base)
{
    std::lock_guard<std::mutex> lock(mu_);
    if (base.empty()) {
        return;
    }
    base_ = base;
    if (base_.back() != '/') {
        base_.push_back('/');
    }
}

std::string FbNet::base_url() const
{
    std::lock_guard<std::mutex> lock(mu_);
    return base_;
}

std::string FbNet::last_error() const
{
    std::lock_guard<std::mutex> lock(mu_);
    return last_error_;
}

void FbNet::set_enabled(bool on)
{
    {
        std::lock_guard<std::mutex> lock(mu_);
        if (on == enabled_) {
            return;
        }
        if (!on) {
            stop_ = true;
            cv_.notify_all();
        } else {
            if (base_.empty()) {
                base_ = default_url();
            }
            stop_ = false;
            enabled_ = true;
        }
    }
    if (!on) {
        if (worker_.joinable()) {
            worker_.join();
        }
        enabled_ = false;
        return;
    }
    worker_ = std::thread(&FbNet::loop, this);
}

void FbNet::put(const std::string &path, const std::string &json)
{
    if (!enabled_) {
        return;
    }
    std::lock_guard<std::mutex> lock(mu_);
    puts_[path] = json;
    cv_.notify_all();
}

void FbNet::watch(const std::string &node)
{
    if (!enabled_) {
        return;
    }
    std::lock_guard<std::mutex> lock(mu_);
    if (watches_.insert(node).second) {
        watch_due_[node] = 0;  // сразу в первом цикле
        cv_.notify_all();
    }
}

bool FbNet::pop(FbUpdate *out)
{
    std::lock_guard<std::mutex> lock(mu_);
    if (updates_.empty()) {
        return false;
    }
    *out = updates_.front();
    updates_.pop_front();
    return true;
}

void FbNet::loop()
{
    while (true) {
        std::map<std::string, std::string> batch;
        std::vector<std::string> due;
        {
            std::unique_lock<std::mutex> lock(mu_);
            cv_.wait_for(lock, std::chrono::milliseconds(kTickMs), [this] { return stop_; });
            if (stop_) {
                return;
            }
            const long long now = now_ms();
            // Записи уходят не чаще kSendMs: в онлайне позиции пишутся каждый кадр,
            // а серверу хватает нескольких пачек в секунду.
            if (!puts_.empty() && now >= next_send_) {
                batch.swap(puts_);
                next_send_ = now + kSendMs;
            }
            for (const auto &node : watches_) {
                long long &when = watch_due_[node];
                if (when <= now) {
                    when = now + kPollMs;
                    due.push_back(node);
                }
            }
        }
        if (!batch.empty()) {
            send_puts(batch);
        }
        for (const auto &node : due) {
            fetch_node(node);
        }
    }
}

void FbNet::send_puts(std::map<std::string, std::string> &batch)
{
    // Собираем записи по родительскому узлу: PATCH {"X1":10,"Y1":20} — один запрос.
    std::map<std::string, std::map<std::string, std::string>> by_parent;
    for (const auto &pair : batch) {
        const size_t slash = pair.first.rfind('/');
        if (slash == std::string::npos || slash == 0) {
            by_parent[pair.first][""] = pair.second;
            continue;
        }
        by_parent[pair.first.substr(0, slash)][pair.first.substr(slash + 1)] = pair.second;
    }
    for (const auto &node : by_parent) {
        const std::string &parent = node.first;
        std::string body;
        std::string method = "PATCH";
        if (node.second.size() == 1 && node.second.count("")) {
            // запись целого узла — обычный PUT
            body = node.second.at("");
            method = "PUT";
        } else {
            body = "{";
            bool first = true;
            for (const auto &leaf : node.second) {
                if (leaf.first.empty()) {
                    continue;
                }
                if (!first) {
                    body += ",";
                }
                first = false;
                body += "\"" + leaf.first + "\":" + leaf.second;
            }
            body += "}";
        }
        std::string response;
        std::string url;
        {
            std::lock_guard<std::mutex> lock(mu_);
            url = base_ + encode_path(parent) + ".json";
            ++requests_;
        }
        if (!request(method, url, body, &response)) {
            std::lock_guard<std::mutex> lock(mu_);
            ++failures_;
            // неудачную запись отложим на следующий круг (не теряем прогресс)
            for (const auto &leaf : node.second) {
                if (leaf.first.empty()) {
                    continue;
                }
                puts_[parent + "/" + leaf.first] = leaf.second;
            }
        }
    }
}

void FbNet::fetch_node(const std::string &node)
{
    std::string url;
    {
        std::lock_guard<std::mutex> lock(mu_);
        url = base_ + encode_path(node) + ".json";
        ++requests_;
    }
    std::string response;
    if (!request("GET", url, "", &response)) {
        std::lock_guard<std::mutex> lock(mu_);
        ++failures_;
        return;
    }
    std::vector<FbUpdate> parsed;
    fb_flatten(node, response, &parsed);
    std::lock_guard<std::mutex> lock(mu_);
    for (auto &entry : parsed) {
        updates_.push_back(std::move(entry));
    }
}

bool FbNet::request(const std::string &method, const std::string &url, const std::string &body,
                    std::string *response)
{
    std::string error;
    bool ok = false;
#if defined(__ANDROID__) && defined(CB_USE_SDL2)
    ok = java_request(method, url, body, response, &error);
#else
    ok = socket_request(method, url, body, response, &error);
#endif
    if (!ok) {
        std::lock_guard<std::mutex> lock(mu_);
        last_error_ = error;
    }
    return ok;
}

}  // namespace cb
