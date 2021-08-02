#include <vector>
#include <string>
#include <limits>
#include <cmath>
#include <cassert>

#include "json.hpp"

namespace json{

const unsigned maxdepth = 200;

using std::string;
using std::vector;
using std::map;
using std::make_shared;

// 传入类型null, NUL类型原型, null类型比较原理，所有的null类型均相等
struct NullStruct{
    bool operator==(NullStruct) const { return true; }
    bool operator<(NullStruct) const { return false; }
};

// dump序列化函数

static void dump(NullStruct, string &out){
    out += "null";
}

static void dump(int value, string &out){
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", value);
    out += buf;
}

static void dump(double value, string &out){
    if(std::isfinite(value)){
        char buf[32];
        snprintf(buf, sizeof(buf), "%.17g", value);
        out += buf;
    }
    else{
        out += "null";
    }
}

static void dump(bool value, string &out){
    out += value? "true" : "false";
}

static void dump(const string &value, string &out){
    out += '"';
    for(size_t i = 0; i < value.length(); i++){
        const char ch =  value[i];
        if(ch == '\\'){
            out += "\\\\";
        }
        else if(ch == '"'){
            out += "\\\"";
        }
        else if(ch == '\b'){
            out += "\\b";
        }
        else if(ch == '\f'){
            out += "\\f";
        }
        else if(ch == '\n'){
            out += "\\n";
        }
        else if(ch == '\r'){
            out += "\\r";
        }
        else if(ch == '\t'){
            out += "\\t";
        }
        else if(static_cast<uint8_t>(ch) <= 0x1f){
            char buf[8];
            snprintf(buf, sizeof(buf), "\\u%04x", ch);
            out += buf;
        }
        else if(static_cast<uint8_t>(ch) == 0xe2 && static_cast<uint8_t>(value[i+1]) == 0x80
                && static_cast<uint8_t>(value[i+2]) == 0xa8){
            out += "\\u2028"; // 行分割符
            i += 2;
        }
        else if (static_cast<uint8_t>(ch) == 0xe2 && static_cast<uint8_t>(value[i+1]) == 0x80
                && static_cast<uint8_t>(value[i+2]) == 0xa9){
            out += "\\u2029"; // 段落分隔符
            i += 2;
        }
        else{
            out += ch;
        }
    }
    out += '"';
}

static void dump(Json::array values, string &out){
    bool first = true;
    out += "[";
    for(const auto &value : values){
        if(!first)
            out += ", ";
        value.dump(out);
        first = false;
    }
    out += "]";
}

static void dump(Json::object values, string &out){
    bool first = true;
    out += "{";
    for(const auto &value : values){
        if(!first)
            out += ", ";
        dump(value.first, out); // value.first 是一个string, 调用上文中的dump函数
        out += ": ";
        value.second.dump(out);
        first = false;
    }
    out += "}";
}


// Json成员函数 dump() 序列化
void Json::dump(string &out) const {
    m_ptr->dump(out);
}

template <Json::Type tag, typename T>
class Value : public JsonValue{
protected:
    explicit Value(const T &value) : m_value(value) {}
    explicit Value(T &&value)      : m_value(std::move(value)) {}

    Json::Type type() const override {
        return tag;
    }

    bool equals(const JsonValue *other) const override{
        return m_value == static_cast<const Value<tag, T>*>(other)->m_value;
    }

    bool less(const JsonValue *other) const override{
        return m_value < static_cast<const Value<tag, T>*>(other)->m_value;
    }

    void dump(string &out) const override { json::dump(m_value, out); }

    const T m_value;

};



class JsonDouble final : public Value<Json::NUMBER, double>{
    double number_value() const override { return m_value; }
    int int_value() const override { return static_cast<int>(m_value); }
    // double 类型可以判等吗？ 这里根本不合理, 建议增加判等方式，确定一个边界
    bool equals(const JsonValue *other) const override { return m_value == other->number_value(); }
    bool less(const JsonValue *other)   const override { return m_value <  other->number_value(); }
public:
    explicit JsonDouble(double value) : Value(value) {}
};

class JsonInt final : public Value<Json::NUMBER, int>{
    double number_value() const override { return static_cast<double>(m_value); }
    int int_value() const override { return m_value; }
    bool equals(const JsonValue *other) const override { return m_value == other->number_value(); }
    bool less(const JsonValue *other)   const override { return m_value <  other->number_value(); }
public:
    explicit JsonInt(int value) : Value(value) {}
};

class JsonBoolean final : public Value<Json::BOOL, bool>{
    bool bool_value() const override { return m_value; }
public:
    explicit JsonBoolean(bool value) : Value(value) {}
};

class JsonString final : public Value<Json::STRING, string>{
    const std::string & string_value() const override { return m_value; }
    // const Json &operator[](size_t i) const override; 字符串不提供下标运算符
public:
    explicit JsonString(const std::string &value) : Value(value) {}
    explicit JsonString(std::string &&value) : Value(std::move(value)) {}
};

class JsonArray final : public Value<Json::ARRAY, Json::array>{
    const Json::array & array_items() const override { return m_value; }
    const Json &operator[](size_t i) const override;
public:
    explicit JsonArray(const Json::array &value) : Value(value) {}
    explicit JsonArray(Json::array &&value) : Value(std::move(value)) {}
};

class JsonObject final : public Value<Json::OBJECT, Json::object>{
    const Json::object & object_items() const override { return m_value; }
    const Json &operator[](const std::string &key) const override;
public:
    explicit JsonObject(const Json::object &value) : Value(value) {}
    explicit JsonObject(Json::object &&value) : Value(std::move(value)) {}
};

class JsonNull final : public Value<Json::NUL, NullStruct>{
public:
    JsonNull() : Value(NullStruct{}) {}
};

// 静态安全初始化
struct Statics{
    const std::shared_ptr<JsonValue> null = make_shared<JsonNull>();
    const std::shared_ptr<JsonValue> t = make_shared<JsonBoolean>(true);
    const std::shared_ptr<JsonValue> f = make_shared<JsonBoolean>(false);

    const string empty_string;
    const vector<Json> empty_vector;
    const map<string, Json> empty_map;

    Statics() {}
};


static const Statics &statics() {
    static const Statics s;
    return s;
}

static const Json &static_null() {
    static const Json json_null;
    return json_null;
}

// Json接口的初始化
// 构造函数
Json::Json() noexcept                    : m_ptr(statics().null) {} // 特殊
Json::Json(std::nullptr_t) noexcept      : m_ptr(statics().null) {} // 特殊
Json::Json(int value)                    : m_ptr(make_shared<JsonInt>(value)) {}
Json::Json(double value)                 : m_ptr(make_shared<JsonDouble>(value)) {}
Json::Json(bool value)                   : m_ptr(value? statics().t : statics().f) {}  // 特殊
Json::Json(const std::string &value)     : m_ptr(make_shared<JsonString>(value)) {}
Json::Json(std::string &&value)          : m_ptr(make_shared<JsonString>(std::move(value))) {}
Json::Json(const char *value)            : m_ptr(make_shared<JsonString>(value)) {}
Json::Json(const array &values)          : m_ptr(make_shared<JsonArray>(values)) {}
Json::Json(array &&values)               : m_ptr(make_shared<JsonArray>(std::move(values))) {}
Json::Json(const object &values)         : m_ptr(make_shared<JsonObject>(values)) {}
Json::Json(object &&values)              : m_ptr(make_shared<JsonObject>(std::move(values))) {}

// 存取器
Json::Type Json::type()                              const { return m_ptr->type(); }

double Json::number_value()                          const { return m_ptr->number_value(); }
int Json::int_value()                                const { return m_ptr->int_value(); }
bool Json::bool_value()                              const { return m_ptr->bool_value(); }
const std::string & Json::string_value()             const { return m_ptr->string_value(); }
const Json::array & Json::array_items()              const { return m_ptr->array_items(); }
const Json::object & Json::object_items()           const { return m_ptr->object_items(); }

// 下标运算符重载
const Json& Json::operator[](size_t i)               const { return (*m_ptr)[i]; }
const Json& Json::operator[](const std::string &key) const { return (*m_ptr)[key]; }

// 比较器 思考过程 自上而下 一级一级的考虑
// 1. 智能指针相等 直接返回
// 2. 类型不等 ==：返回false <: 返回其类型enum类型定义顺序比较
// 3. 类型相等 使用其内部指针比较其内部数据(C++内置比较规则)
bool Json::operator==(const Json &rhs) const {
    if(m_ptr == rhs.m_ptr)
        return true;
    if(m_ptr->type() != rhs.m_ptr->type()){
        return false;
    }
    return m_ptr->equals(rhs.m_ptr.get());
}

bool Json::operator< (const Json &rhs) const {
    if(m_ptr == rhs.m_ptr)
        return false;
    if(m_ptr->type() != rhs.m_ptr->type())
        return m_ptr->type() < m_ptr->type();
    return m_ptr->less(rhs.m_ptr.get());
}

// JsonValue 类的成员函数定义 安全初始化 非纯虚函数
double                  JsonValue::number_value()                     const { return 0; }
int                     JsonValue::int_value()                        const { return 0; }
bool                    JsonValue::bool_value()                       const { return false; }
const std::string &     JsonValue::string_value()                     const { return statics().empty_string; }
const Json::array &     JsonValue::array_items()                      const { return statics().empty_vector; }
const Json::object &    JsonValue::object_items()                     const { return statics().empty_map; }

const Json &            JsonValue::operator[](size_t i)               const { return static_null(); }
const Json &            JsonValue::operator[](const std::string &key) const { return static_null(); }

// 子类下标运算符重载
// 下标运算符重载 需要检查 越界问题
const Json & JsonArray::operator[](size_t i) const {
    if(i >= m_value.size()) return static_null();
    return m_value[i];
};

const Json & JsonObject::operator[](const std::string &key) const {
    auto iter = m_value.find(key);
    return (iter == m_value.end())? static_null() : iter->second; // m_value[key];?
}

/*esc(c), 格式化字符c，用于错误处理，更加友好的错误分析*/
static inline string esc(char c){
    char buf[12];
    if(static_cast<uint8_t>(c) >= 0x20 && static_cast<uint8_t>(c) <= 0x7f){
        snprintf(buf, sizeof(buf), "'%c' (%d)", c, c);
    }
    else{
        snprintf(buf, sizeof(buf), "(%d)", c);
    }
    return string(buf);
}

/*判断x是否在[lower, upper]范围内*/
static inline bool in_range(long x, long lower, long upper){
    return (x >= lower && x <= upper);
}

// 解析器实现  将解析器实现细节放入一个匿名命名空间
namespace {

struct JsonParser final{
    const string &str;
    size_t i;
    string &err;
    bool failed;
    const JsonParse strategy;

    /*错误处理函数*/
    // T err_ret:           返回第二个实参
    // std::string err:     错误信息存储变量，只有第一次错误的信息
    Json fail(string &&msg){
        return fail(move(msg), Json());
    }

    template <typename T>
    T fail(string &&msg, const T err_ret){
        if(!failed) // 如果是第一次错误
            err = std::move(msg);
        failed = true;
        return err_ret;
    }
    
    /*消耗空白字符*/
    // 包括空白字符，回车符，换行符，横向制表符
    void consume_whitespace(){
        while(str[i] == ' ' || str[i] == '\r' || str[i] == '\n' || str[i] == '\t')
            i++;
    }

    /*消耗注释字符*/
    // 返回值：发现注释，返回true，未发现或出现错误语法 则返回 false 并设置err
    bool consume_comment(){
        bool consume_found = false;
        if(str[i] == '/'){
            i++;
            if(i == str.size())
                return fail("unexpected end of input after start of comment", false);
            if(str[i] == '/'){
                i++;
                while(i < str.size() && str[i] != '\n'){
                    i++;
                }
                consume_found = true;
            }
            else if(str[i] == '*'){
                i++;
                if(i > str.size() - 2) 
                    return fail("unexpected end of input inside multi-line comment", false);
                while(!(str[i] == '*' && str[i+1] == '/')){
                    i++;
                    if(i > str.size()-2) 
                        return fail("unexpected end of input inside multi-line comment", false);
                }
                i += 2;
                consume_found = true;
            }
            else
                return fail("malformed comment", false);
        }
        return consume_found;
    }


    /*消耗无用字符*/
    void consume_garbage(){
        consume_whitespace();
        if(strategy == JsonParse::COMMENTS){
            bool comment_found = false;
            do{
                comment_found = consume_comment();
                if(failed) return ;
                consume_whitespace();
            } while(comment_found);
        }
    }

    /*获取下一个可解析字符*/
    // 消耗空白等垃圾字符
    // 消耗注释字符
    // 主要为了ARRAY与OBJECT设置，在这两种类型中可以使用注释或者空白
    // 返回值： 返回一个有效字符，i指向该有效字符下一个字符
    char get_next_token(){
        consume_garbage();
        if(failed) return static_cast<char>(0); // 为什么这里不 return failed; 没必要
        if(i == str.size())
            return fail("unexpected end of input", static_cast<char>(0));
        
        return str[i++];
    }

    /*将字符pt编码为UTF-8*/
    void encode_utf8(long pt, string &out){
        if(pt < 0)
            return;
        else if(pt < 0x80)
            out += static_cast<char>(pt);
        else if(pt < 0x800){
            out += static_cast<char>((pt >> 6) | 0xC0);
            out += static_cast<char>((pt & 0x3F) | 0x80);
        }
        else if(pt < 0x10000){
            out += static_cast<char>((pt >> 12) | 0xE0);
            out += static_cast<char>((pt >> 6) & 0x3F | 0x80);
            out += static_cast<char>((pt & 0x3F) | 0x80);
        }
        else{
            out += static_cast<char>((pt >> 18) | 0xF0);
            out += static_cast<char>((pt >> 12) & 0x3F | 0x80);
            out += static_cast<char>((pt >> 6) & 0x3F | 0x80);
            out += static_cast<char>((pt & 0x3F) | 0x80);
        }
    }


    /*解析字符串*/
    string parse_string(){
        string out;
        long last_escaped_codepoint = -1;
        while(true){
            if(i == str.size())
                return fail("unexpected end of input in string", "");
            
            char ch = str[i++];
            
            if(ch == '"'){
                encode_utf8(last_escaped_codepoint, out);
                return out;
            }

            if(in_range(ch, 0, 0x1f))
                return fail("unexpected char(0 ~ 0x1f): " + esc(ch) + " in string", "");
            
            if(ch != '\\'){
                encode_utf8(last_escaped_codepoint, out);
                last_escaped_codepoint = -1;
                out += ch;
                continue;
            }
            // ch == '\\'
            if(i == str.size())
                return fail("unexpected end of input in string", "");
            
            ch = str[i++];

            if(ch == 'u'){
                // unicode编码
                string esc = str.substr(i, 4);
                if(esc.size() < 4){
                    return fail("bad \\u escape: " + esc, "");
                }
                for(size_t j = 0; j < 4; j++){
                    if(!in_range(esc[j], 'a', 'f') && !in_range(esc[j], 'A', 'F') 
                        && !in_range(esc[j], '0', '9')){
                        return fail("bad \\u escape: " + esc, "");
                    }
                }
                long codepoint = strtol(esc.data(), nullptr, 16);

                if(in_range(last_escaped_codepoint, 0xD800, 0xDBFF) 
                    && in_range(codepoint, 0xDC00, 0xDFFF)){
                    encode_utf8((((last_escaped_codepoint - 0xD800) << 10) | (codepoint - 0xDC00)) + 0x10000, out);
                    last_escaped_codepoint = -1;
                }
                else{
                    encode_utf8(last_escaped_codepoint, out);
                    last_escaped_codepoint = codepoint;
                }
                i += 4;
                continue;
            }

            encode_utf8(last_escaped_codepoint, out); // 上一次转义的码点还未处理
            last_escaped_codepoint = -1;

            if(ch == 'b'){
                out += '\b';
            }
            else if(ch == 'f'){
                out += '\f';
            }
            else if(ch == 'n'){
                out += '\n';
            }
            else if(ch == 'r'){
                out += '\r';
            }
            else if(ch == 't'){
                out += '\t';
            }
            else if(ch == '"' || ch == '\\' || ch == '/'){
                out += ch;
            }
            else{
                return fail("invalid escape character " + esc(ch), "");
            }
        }
    }

    /*解析数字*/
    Json parse_number(){
        size_t start_pos = i;

        if(str[i] == '-')
            i++;
        if(str[i] == '0'){
            i++;
            if(in_range(str[i], '0', '9')){
                return fail("leading 0s not permitted in numbers");
            }
        }
        else if(in_range(str[i], '0', '9')){
            i++;
            while(in_range(str[i], '0', '9')){
                i++;
            }
        }
        else{
            return fail("invaild char " + esc(str[i]) + " in number");
        }

        // 整数部分
        if(str[i] != '.' && str[i] != 'E' && str[i] != 'e'
            && (i - start_pos) <= static_cast<size_t>(std::numeric_limits<int>::digits10)){
            return std::atoi(str.c_str() + start_pos);
        }

        // 浮点数部分
        if(str[i] == '.'){
            i++;
            if(!in_range(str[i], '0', '9')){
                return fail("at least one digit required in fractional part");
            }
            while(in_range(str[i], '0', '9')){
                i++;
            }
        }
        
        // 使用科学计数法的数字解析
        if(str[i] == 'E' || str[i] == 'e'){
            i++;
            if(str[i] == '+' || str[i] == '-'){
                i++;
            }
            if(!in_range(str[i], '0', '9')){
                return fail("at least one digit required in exponent");
            }
            while(in_range(str[i], '0', '9')){
                i++;
            }
        }

        return std::strtod(str.c_str() + start_pos, nullptr);
    }

    /*解析NUL, BOOL类型*/
    Json expect(const string &expected, Json res){
        assert(i != 0);
        i--;
        if(str.compare(i, expected.length(), expected) == 0){
            i += expected.length();
            return res;
        }
        else{
            return fail("parse error: expected " + expected + ", got " + 
                str.substr(i, expected.length()));
        }
    }

    /*解析一个Json*/
    Json parse_json(unsigned depth){
        if(depth > maxdepth){
            return fail("exceeded maximum nesting depth");
        }
        char ch = get_next_token();
        if(failed){
            return Json();
        }
        if(ch == '-' || (ch >= '0' && ch <= '9')){
            i--;
            return parse_number();
        }
        if(ch == 't')
            return expect("true", true);
        if(ch == 'f')
            return expect("false", false);
        if(ch == 'n')
            return expect("null", Json());
        if(ch == '"')
            return parse_string();
        
        if(ch == '{'){
            map<string, Json> data;
            ch = get_next_token();
            if(ch == '}')
                return data;
            while(1){
                if(ch != '"')
                    return fail("expected '\"' in object, but got " + esc(ch));
                string key = parse_string();
                if(failed)
                    return Json();
                ch = get_next_token();
                if(ch != ':')
                    return fail("expected ':' in object, but got " + esc(ch));
                data[std::move(key)] = parse_json(depth + 1);
                if(failed) // 上述过程可能失败
                    return Json();
                ch = get_next_token();
                if(ch == '}')
                    break;
                if(ch != ',')
                    return fail("expected ',' in object, but got " + esc(ch));
                ch = get_next_token();
            }
            return data;
        }

        if(ch == '['){
            vector<Json> data;
            ch = get_next_token();
            if(failed)
                return Json();
            if(ch == ']')
                return data;
            while(1){

                i--; //****
                data.push_back(parse_json(depth + 1));
                if(failed)
                    return Json();

                ch = get_next_token();
                if(ch == ']')
                    break;
                if(ch != ',')
                    return fail("expected ',' in array, but got " + esc(ch));

                ch = get_next_token();
                (void)ch; // ?
            }
            return data;
        }
        return fail("expected value, but got " + esc(ch) + ". parse failed!!!");
    }
}; // struct JsonParse

}; //namespace


// 解析器接口
Json Json::parse(const string &in, string &err, JsonParse strategy){
    JsonParser parser { in, 0, err, false, strategy };
    Json result = parser.parse_json(0);

    parser.consume_garbage();
    if(parser.failed)
        return Json();
    if(parser.i != in.size())
        return parser.fail("unexpected trailing " + esc(in[parser.i]));

    return result;
}


} //namespace json;