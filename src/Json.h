#pragma once
// =============================================================================
// Json.h — Minimal single-header JSON parser (zero dependencies)
//
// Supports: objects, arrays, strings, numbers (int/float), booleans, null.
//
// Usage:
//   auto root = Json::parse(src);
//   std::string name = root["device"].getString();
//   for (auto& page : root["pages"].toArray()) { ... }
// =============================================================================
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <cstdlib>


struct Json {
    enum Type { Null, Bool, Number, String, Array, Object };
    Type type = Null;

    bool                                    b   = false;
    double                                  n   = 0.0;
    std::string                             s;
    std::vector<Json>                       arr;
    std::unordered_map<std::string, Json>   obj;

    bool isNull()   const { return type == Null;   }
    bool isBool()   const { return type == Bool;   }
    bool isNumber() const { return type == Number; }
    bool isString() const { return type == String; }
    bool isArray()  const { return type == Array;  }
    bool isObject() const { return type == Object; }

    bool               toBool()   const { return b; }
    int                toInt()    const { return static_cast<int>(n); }
    double             toDouble() const { return n; }
    const std::string& toString() const { return s; }

    const std::vector<Json>&                     toArray()  const { return arr; }
    const std::unordered_map<std::string, Json>& toObject() const { return obj; }

    bool has(const std::string& key) const {
        return type == Object && obj.count(key) > 0;
    }

    // Returns a null Json if the key is absent — safe for chaining.
    const Json& operator[](const std::string& key) const {
        if (type == Object) {
            auto it = obj.find(key);
            if (it != obj.end()) return it->second;
        }
        static Json null;
        return null;
    }

    const Json& operator[](size_t idx) const {
        if (type == Array && idx < arr.size()) return arr[idx];
        static Json null;
        return null;
    }

    std::string getString(std::string def = "") const {
        return isString() ? s : def;
    }
    int getInt(int def = 0) const {
        return isNumber() ? toInt() : def;
    }
    bool getBool(bool def = false) const {
        return isBool() ? b : def;
    }

    std::string getString(const std::string& key, std::string def) const {
        return has(key) ? obj.at(key).getString(std::move(def)) : def;
    }
    int getInt(const std::string& key, int def) const {
        return has(key) ? obj.at(key).getInt(def) : def;
    }
    bool getBool(const std::string& key, bool def) const {
        return has(key) ? obj.at(key).getBool(def) : def;
    }

    static Json parse(const std::string& src);
    static Json parseFile(const std::string& path);

private:
    struct Parser {
        const char* p;
        const char* end;

        explicit Parser(const std::string& src)
            : p(src.data()), end(src.data() + src.size()) {}

        void skip() {
            while (p < end && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n'))
                ++p;
        }
        char peek()    { skip(); return p < end ? *p : '\0'; }
        char consume() { return p < end ? *p++ : '\0'; }

        void expect(char c) {
            skip();
            if (p >= end || *p != c)
                throw std::runtime_error(
                    std::string("JSON: expected '") + c +
                    "' got '" + (p < end ? *p : '?') + "'");
            ++p;
        }

        Json parseValue() {
            char c = peek();
            if (c == '{') return parseObject();
            if (c == '[') return parseArray();
            if (c == '"') { Json j; j.type = String; j.s = parseString(); return j; }
            if (c == 't') { p += 4; Json j; j.type = Bool; j.b = true;  return j; }
            if (c == 'f') { p += 5; Json j; j.type = Bool; j.b = false; return j; }
            if (c == 'n') { p += 4; return Json(); }
            if (c == '-' || (c >= '0' && c <= '9')) return parseNumber();
            throw std::runtime_error(std::string("JSON: unexpected char '") + c + "'");
        }

        Json parseObject() {
            Json j; j.type = Object;
            expect('{'); skip();
            if (peek() == '}') { consume(); return j; }
            do {
                skip();
                std::string key = parseString();
                expect(':');
                j.obj[key] = parseValue();
                skip();
            } while (peek() == ',' && consume());
            expect('}');
            return j;
        }

        Json parseArray() {
            Json j; j.type = Array;
            expect('['); skip();
            if (peek() == ']') { consume(); return j; }
            do {
                j.arr.push_back(parseValue());
                skip();
            } while (peek() == ',' && consume());
            expect(']');
            return j;
        }

        std::string parseString() {
            skip(); expect('"');
            std::string r;
            while (p < end && *p != '"') {
                if (*p == '\\') {
                    ++p;
                    switch (*p) {
                        case '"':  r += '"';  break;
                        case '\\': r += '\\'; break;
                        case '/':  r += '/';  break;
                        case 'n':  r += '\n'; break;
                        case 'r':  r += '\r'; break;
                        case 't':  r += '\t'; break;
                        default:   r += *p;   break;
                    }
                } else {
                    r += *p;
                }
                ++p;
            }
            expect('"');
            return r;
        }

        Json parseNumber() {
            skip();
            const char* start = p;
            if (*p == '-') ++p;
            while (p < end && *p >= '0' && *p <= '9') ++p;
            if (p < end && *p == '.') {
                ++p;
                while (p < end && *p >= '0' && *p <= '9') ++p;
            }
            if (p < end && (*p == 'e' || *p == 'E')) {
                ++p;
                if (p < end && (*p == '+' || *p == '-')) ++p;
                while (p < end && *p >= '0' && *p <= '9') ++p;
            }
            Json j; j.type = Number; j.n = std::strtod(start, nullptr);
            return j;
        }
    };

    friend Json Json::parse(const std::string&);
};

#include <fstream>
#include <sstream>

inline Json Json::parse(const std::string& src) {
    Parser p(src);
    return p.parseValue();
}

inline Json Json::parseFile(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open())
        throw std::runtime_error("JSON: cannot open file: " + path);
    std::ostringstream ss;
    ss << f.rdbuf();
    return parse(ss.str());
}
