#pragma once

#include <vector>
#include <stdlib.h>
#include <map>
#include <string>
#include <memory>

namespace json{
// 解析模式
enum JsonParse{
    STANDARD, COMMENTS
};

class JsonValue;

class Json final{
public:

    // 枚举Json类型
    enum Type{
        NUL, NUMBER, BOOL, STRING, ARRAY, OBJECT
    };

    // typedef
    typedef std::vector<Json> array;
    typedef std::map<std::string, Json> object;

    // 构造函数
    Json() noexcept;
    Json(std::nullptr_t) noexcept;
    Json(int value);
    Json(double value);
    Json(bool value);
    Json(const std::string &value);
    Json(std::string &&value);
    Json(const char *value);
    Json(const array &value);
    Json(array &&value);
    Json(const object &value);
    Json(object &&value);

    template <typename T, class = decltype(&T::to_json())>
    Json(const T &t) : Json(t.to_json()) {}

    template <class V, typename std::enable_if<
        std::is_constructible<Json, decltype(*std::declval<V>().begin())>::value,
            int>::type = 0>
    Json(const V &v) : Json(array(v.begin(), v.end())) {}

    template <class M, typename std::enable_if<
        std::is_constructible<std::string, decltype(std::declval<M>().begin()->first)>::value
        && std::is_constructible<Json, decltype(std::declval<M>().begin()->second)>::value,
            int>::type = 0>
    Json(const M &m) : Json(object(m.begin(), m.end())) {}

    Json(void*) = delete;

    // 判别器
    Type type() const;

    bool is_null()          const { return type() == NUL; }
    bool is_numter()        const { return type() == NUMBER;}
    bool is_bool()          const { return type() == BOOL; }
    bool is_string()        const { return type() == STRING; }
    bool is_array()         const { return type() == ARRAY; }
    bool is_object()        const { return type() == OBJECT; }

    // 存取器
    double number_value() const;
    int int_value() const;
    bool bool_value() const;
    const std::string & string_value() const;
    const array & array_items() const;
    const object & object_items() const;

    // 下标运算符
    const Json& operator[](size_t i) const;
    const Json& operator[](const std::string &key) const;

    // 序列化
    void dump(std::string &out) const;
    std::string dump() const {
        std::string out;
        dump(out);
        return out;
    }

    // 解析静态函数
    static Json parse(const std::string &in,
                      std::string &err,
                      JsonParse strategy = JsonParse::STANDARD);
    static Json parse(const char *in,
                      std::string &err,
                      JsonParse strategy = JsonParse::STANDARD){
        if(in){
            return parse(std::string(in), err, strategy);
        }
        else{
            err = "null input";
            return nullptr;
        }
    }

    // 比较器
    bool operator==(const Json &rhs) const;
    bool operator< (const Json &rhs) const;
    bool operator!=(const Json &rhs) const { return !(*this == rhs); }
    bool operator> (const Json &rhs) const { return (rhs < *this);   }
    bool operator<=(const Json &rhs) const { return !(rhs < *this);  }
    bool operator>=(const Json &rhs) const { return !(*this < rhs);  }

private:
    std::shared_ptr<JsonValue> m_ptr;
};

class JsonValue{
protected:
    friend class Json;
    friend class JsonInt;
    friend class JsonDouble;

    virtual Json::Type type() const = 0;
    virtual bool equals(const JsonValue *other) const = 0;
    virtual bool less(const JsonValue *other) const = 0;

    virtual void dump(std::string &out) const = 0;

    virtual double number_value() const;
    virtual int int_value() const;
    virtual bool bool_value() const;
    virtual const std::string & string_value() const;
    virtual const Json::array & array_items() const;
    virtual const Json::object & object_items() const;

    virtual const Json &operator[](size_t i) const;
    virtual const Json &operator[](const std::string &key) const;

    virtual ~JsonValue() {} 
};
} // namespace json