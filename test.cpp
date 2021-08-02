#include <cassert>
#include <string>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <sstream>
#include "json.hpp"
#include <list>
#include <set>
#include <unordered_map>
#include <algorithm>
#include <type_traits>
#include <map>
#include <stack>
#include <array>
#include <queue>
#include <forward_list>
#include <unordered_set>

#include <typeinfo>

using namespace json;  //*****
using std::string;
using std::map;
using std::cout;
using std::cin;
using namespace std;

Json test_01(const string& str) {
	return Json(str);
}

template<typename T>
class fangvector {
public:

	T* m_begin_value = nullptr;
	T* m_end_value = nullptr;
	T* begin() const {
		return m_begin_value;
	}
	T* end() const {
		return m_end_value;
	}

};


int main() {

	fangvector<int> fangv1;

	Json fangv1json(fangv1);
	cout << fangv1json.dump() << endl;


	std::list<int> l1{ 1, 2, 3 };
	std::vector<int> l2{ 1, 2, 3 };
	std::set<int> l3{ 1, 2, 3 };
	std::unordered_set<int> luset{ 1, 2, 3 };
	std::deque<int> l4{ 1, 2, 3 };
	std::deque<string> ls4{ "x", "xx", "xxx" };
	std::stack<int> stk4;
	std::array<int, 4> arl5{ 1, 2, 3 };
	std::forward_list<int> fll6{ 1, 2, 3 };

	vector<int> lcopy(l3.begin(), l3.end());
	for (auto e : lcopy) {
		cout << e << endl;
		cout << "fdsfsf" << endl;
	}

	stk4.push(1);
	stk4.push(2);
	stk4.push(3);
	int a[5] = { 1, 2, 3, 4, 5 };
	string strxy("test_011");

	std::map<string, string> m1{ { "k1", "v1" }, { "k2", "v2" } };
	std::unordered_map<string, string> m2{ { "k1", "v1" }, { "k2", "v2" } };

	//std::multimap<string, int> m3{ {"k1", "v1"},{"k1", "v1"} };

	Json jsonnull;
	std::cout << jsonnull.int_value() << std::endl;
	std::cout << jsonnull.string_value() << std::endl;
	Json doublex(5);
	Json doublec(5.2);
	//*****************************************************************************
	cout << "================================================================" << endl;
	Json mpjson(m1);
	cout << mpjson.dump() << endl;

	Json hsjson(m2);
	cout << hsjson.dump() << endl;

	Json lsjson(l1);
	cout << lsjson.dump() << endl;

	Json stjson(l3); //set
	cout << stjson.dump() << endl;

	Json dejson(l4); //deque
	cout << dejson.dump() << endl;

	Json desjson(ls4);
	cout << desjson.dump() << endl;

	Json ustjson(luset);
	cout << ustjson.dump() << endl;

	//Json stkjson(stk4);  这个不行
	//cout << stkjson.dump() << endl;

	Json armjson(a[5]);
	cout << armjson.dump() << endl;

	Json funcjson(test_01(strxy));
	cout << funcjson.dump() << endl;

	Json arrjson(arl5);
	cout << arrjson.dump() << endl;

	Json fltjson(fll6);
	cout << fltjson.dump() << endl; 

	Json jscpy(fltjson);
	cout << jscpy.dump() << endl;

	Json jsas = jscpy;
	cout << jsas.dump() << endl;

	cout << "==================================================================" << endl;
	//******************************************************************************
	
	if (doublex < doublec) {
		std::cout << "ok" << std::endl;
	}
	int x = doublex.int_value();
	std::cout << x << "x" << std::endl;
	const Json obj({
		{ "k1", "v1" },
		{ "k2", 42.0 },
		{ "k3", Json::array{ "a", 123.0, true, false, nullptr } },
		});
	string sxx1 = "xxxx";
	Json strin1 = Json(std::move(sxx1));
	Json strin2 = Json(sxx1); // 左值是不进行移动构造的，右值才行

	const Json obj1 = Json({
		{ "k1", "v1" },
		{ "k2", 42.0 },
		{ "k3", Json::array{ "a", 123.0, true, false, nullptr} },
	});
	Json arrayx = Json::array{ 1, 2, 3, 4, 5 };
	Json array1({ "1", "c", "c" });

	if (obj1 > array1) {
		std::cout << "obj1 > array" << std::endl;
	}
	Json jsnull1(nullptr);
	Json jsnull2(0);
	std::cout << "jsnull1(nullptr): " << jsnull1.dump() << std::endl;
	std::cout << "jsnull2(NULL): " << jsnull2.dump() << std::endl;
	std::cout << "jsnull1.type(): " << jsnull1.type() << std::endl;
	std::cout << "obj1.type(): " << obj1.type() << std::endl; // 可不可以返回字符串啊
	// 本json库禁止传入NULL参数，会重载Json(int value);
	// 怎么解决呢？
	
	// test 1
	// Json my_json = Json::object({
	//     { "key1", "value1" },
	//     { "key2", false },
	//     { "key3", Json::array { 1, 2, 3 } },
	//     { "key4", m1}
	// });

	// Json json_me = 


	// std::string json_obj_str = my_json.dump();
	// std::cout << "json_obj_str: " << json_obj_str << "\n";

	string err;
	// test 2
	
	const string simple_test = R"({"k1": "中国", "k2":42, "k3":["a",123,true,false,null], "k4": "\ud842\udfff", "k5": "\u5403\u996d"})";
	

	Json json = Json::parse(simple_test, err);
    cout << "json::parse err: " << err << endl;
	//cout << "json.object(): " << json.object_items() << endl;
	cout << "json.dump(): " << json.dump() << endl;
	// json11::JsonDouble
	int m_q = 10;

	Json json_string_2("中国");
	std::cout << "json_string_2; " << json_string_2.dump() << endl;

	const auto string_2 = R"({"鬞东": "中国" })";

	const auto json_2 = Json::parse(string_2, err);
	std::cout << "string_2: " << json_2.dump() << endl;

	std::cout << "jsonxxx" << json.dump() << std::endl;
	std::cout << "k1: " << json["k1"].string_value() << "\n";
	// 将Json类型转换其他类型; // json.dump() 用于序列化
	std::cout << "k3: " << json["k3"].dump() << "\n";

	string s1 = json["k3"].dump();
	for (auto e : s1) {
		std::cout << e;
	}

	std::cout << std::endl;
	string strxxxx = "\\x";
	// array_items()  返回值为  vector<Json>& 
	for (auto &k : json["k3"].array_items()) {
		std::cout << "    - " << k.dump() << "\n";
		// ---string s2 = k.dump();  //string类重载了 cout 对象
		// ---std::cout << s2 << std::endl;
	}
	// ---std::cout << json["k3"].array_items()[1].dump() << std::endl;
	// ---json["k3"].array_items()[1] 是一个json类型
	// ---使用dump方法后 成为一个string类型
	std::cout << "=====================================" << std::endl;
	const string sx = R"({" k1 ":   " v1", "k3":[]]})";
	// ---格式存在错误，所有成员都是null
	// ---严格对应，空格也要展现出来，":" 前后的空格 不会受到限制
	// ---如果不加上R 括号内的字符是白色的，不是字符串
	const auto json_me = Json::parse(sx, err);
	// ---object_items() 返回类型 std::map<std::string, Json>&;
	// for(auto k : json_me.object_items()){
	//     std::cout << "e.dump(): " << k[" k1 "].dump() << std::endl;
	// }
	std::cout << "err: " << err << std::endl;
	std::cout << json_me[" k1 "].dump() << std::endl;
	std::cout << json_me["k3"].dump() << std::endl;
	std::cout << "=====================================" << std::endl;
	

	string stringnumber = R"(-1234, true, false, {"k1": "123", "k2": false, "k3": null})";
	const auto json_string_88 = Json::parse(stringnumber, err);
	cout << "json_string_88: " << json_string_88.dump() << endl;
	cout << "err: " << err << endl;

	return 0;
}
