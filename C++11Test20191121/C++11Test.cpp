// C++11Test20191121.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <map>
#include <bitset>
#include <functional>
#include <memory>
#include <mutex>
#include <chrono>
#include <time.h>
#include <list>
#include <future>
#include <algorithm>
#include <iterator>
#include <queue>
#include <random>
#include <numeric>
#include <atomic>
#include <sstream>
#include <variant>
using namespace std;

class MyClass
{
public:
	MyClass();
	MyClass(int v, string s);
	MyClass(const MyClass& obj)
		:MyClass(obj.val,obj.str)
	{

	}
	~MyClass();
	MyClass& operator =(const MyClass obj)
	{
		this->val = obj.val;
		this->str = obj.str;

		return *this;
	}
public:
	void print()
	{
		cout << val << "," << str << endl;
	}
public:
	int val = 0;
	string str{ "MyClass" };
};

MyClass::MyClass()
{
}

MyClass::~MyClass()
{
}

MyClass::MyClass(int v, string s)
	:val(v),str(s)
{
}

class MyClassExt: public MyClass
{
public:
	using MyClass::MyClass;
private:

};

class ThreadPool
{
public:
	ThreadPool(int nThreadCnt)
		:m_nThreadCnt(nThreadCnt)
	{
		m_bExit = false;
	}

	ThreadPool()
	{

	}

	void Start()
	{
		for (int i = 0; i < m_nThreadCnt; i++)
		{
			m_vtThread.push_back(thread(&ThreadPool::WorkLoop, this, i));
		}
	}

	void Stop()
	{
		m_bExit = true;
		m_is_not_empty.notify_all();
		for (thread& td : m_vtThread)
		{
			td.join();
		}
	}

	void WorkLoop(int index)
	{
		while (!m_bExit)
		{
			std::unique_lock<std::mutex> lock(m_lock);

			m_is_not_empty.wait(lock, [this] {return !this->m_vtTask.empty(); });

			if (!m_vtTask.empty())
			{
				//cout << "thread index = " << index << endl;
				m_vtTask.front()();
				m_vtTask.pop_front();
			}
		
			lock.unlock();
		}

		std::unique_lock<std::mutex> lock(m_lock);
		cout << "thread index = " << index << "exit" << endl;
	}

	template<typename F,typename... Args>
	auto Dispatch(F&& f,Args&& ...args)->std::future<decltype(f(args...))>
	{
		std::unique_lock<std::mutex> lock(m_lock);
		using return_type = decltype(f(args...));

		auto task = std::make_shared<std::packaged_task<return_type()> >(
			std::bind(std::forward<F>(f), std::forward<Args>(args)...)
			); 
		std::future<return_type> future = task->get_future();

		m_vtTask.emplace_back(
			[task]()
			{
				(*task)();
			}
		);
		m_is_not_empty.notify_all();

		return future;
	}

private:
	typedef std::list<std::function<void(void)>> MyQueue;

	int m_nThreadCnt;
	std::mutex m_lock;
	MyQueue m_vtTask;
	vector<thread> m_vtThread;
	std::condition_variable m_is_not_empty;
	bool m_bExit;
};

template<typename T>
auto sum(T a, T b)->decltype(a+b)
{
	//std::this_thread::sleep_for(std::chrono::seconds(3));
	return a + b;
}

void thread_test()
{
	ThreadPool pool(thread::hardware_concurrency());
	pool.Start();
	std::future<int> ret = pool.Dispatch(sum<int>, 1, 2);
	cout << ret.get() << endl;
	pool.Stop();
}

void promise_test()
{	
	std::packaged_task<int(int, int)> task(sum<int>);
	std::future<int> future = task.get_future();

	thread td(std::move(task), 3, 5);

	auto beg = std::chrono::system_clock::now();
	cout << "开始计算" << endl;
	cout << future.get() << endl;
	cout << "计算完毕" << endl;
	auto ts = std::chrono::system_clock::now() - beg;
	cout << "耗费时间：" << std::chrono::duration_cast<std::chrono::seconds>(ts).count() << endl;
	td.join();
	cout << "线程结束" << endl;
}

bool lesser(int lhs, int rhs)
{
	return lhs < rhs;
}

bool greate(int lhs, int rhs)
{
	return lhs > rhs;
}

void function_test()
{
	auto fun = std::bind(std::logical_and<bool>(), std::bind(lesser, 10, std::placeholders::_1), std::bind(greate, 20, std::placeholders::_1));
	//auto fun = std::bind(lesss, 10, std::bind(greate,20,std::placeholders::_1));
	//auto s = std::less<int>()(1,2);
	bool res = fun(5);
}

void time_system_test()
{
	cout << "system clock          : ";
	cout << chrono::system_clock::period::num << "/" << chrono::system_clock::period::den << "s" << endl;
	cout << "steady clock          : ";
	cout << chrono::steady_clock::period::num << "/" << chrono::steady_clock::period::den << "s" << endl;
	cout << "high resolution clock : ";
	cout << chrono::high_resolution_clock::period::num << "/" << chrono::high_resolution_clock::period::den << "s" << endl;
}


long long getDotProduct(std::vector<int>& v, std::vector<int>& w) {

	auto future1 = std::async(std::launch::deferred, [&] {std::cout <<"thread1 id:"<< std::this_thread::get_id() << endl; return std::inner_product(v.begin(), v.begin() + v.size() / 4, w.begin(), 0LL); });
	auto future2 = std::async(std::launch::deferred, [&] {std::cout << "thread2 id:" << std::this_thread::get_id() << endl; return std::inner_product(v.begin()+v.size()/4, v.begin() + v.size() / 2, w.begin()+w.size()/4, 0LL); });
	auto future3 = std::async(std::launch::deferred, [&] {std::cout << "thread3 id:" << std::this_thread::get_id() << endl; return std::inner_product(v.begin() + v.size() / 2, v.begin() + v.size()*3 / 4, w.begin() + w.size() / 2, 0LL); });
	auto future4 = std::async(std::launch::deferred, [&] {std::cout << "thread4 id:" << std::this_thread::get_id() << endl; return std::inner_product(v.begin() + v.size() * 3 / 4, v.end(), w.begin() + w.size()*3 / 4, 0LL); });

	return future1.get() + future2.get() + future3.get() + future4.get();
}

void asyn_function_test()
{
	int NUM = 100000;
	std::cout << std::endl;

	// 从 0 到 100 获取 NUM 个随机数 
	std::random_device seed;

	// 生成随机数
	std::mt19937 engine(seed());

	// 0~100之间均匀分配
	std::uniform_int_distribution<int> dist(0, 100);

	// 放进 vector 容器
	std::vector<int> v, w;
	v.reserve(NUM);
	w.reserve(NUM);
	for (int i = 0; i < NUM; ++i) {
		v.push_back(dist(engine));
		w.push_back(dist(engine));
	}

	// 计算执行时间
	//std::cout << std::this_thread::get_id() << endl;
	std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
	std::cout << "getDotProduct(v,w): " << getDotProduct(v, w) << std::endl;
	std::chrono::duration<double> dur = std::chrono::system_clock::now() - start;
	std::cout << "Parallel Execution: " << dur.count() << std::endl;

	std::cout << std::endl;
}

void future_test()
{
	std::promise<int> promiseObj;
	std::future<int> futureObj = promiseObj.get_future();
	//std::thread th([&](std::promise<int>& promObj) {
	//	std::cout << "Inside Thread" << std::endl;
	//	std::this_thread::sleep_for(std::chrono::seconds(5));
	//	promObj.set_value(35);
	//	std::cout << "setvalue" << endl;
	//}, std::ref(promiseObj));
	//std::cout << futureObj.get() << std::endl;
	//th.join();

	auto ret = std::async(std::launch::async,
		[&](std::promise<int>& promObj) 
		{
			std::cout << "Inside Thread" << std::endl;
			std::this_thread::sleep_for(std::chrono::seconds(5));
			promObj.set_value(35);
			std::cout << "setvalue" << endl;
		}, 
		std::ref(promiseObj));

	std::cout << futureObj.get() << std::endl;
}

void package_task_test()
{
	std::packaged_task<void()> convert([]() {
		throw std::logic_error("will catch in future");
	});
	std::future<void> future = convert.get_future();

	convert(); // 异常不会在此处抛出

	try {
		future.get();
	}
	catch (std::logic_error &e) {
		std::cerr << typeid(e).name() << ": " << e.what() << std::endl;
	}

}

void asyn_call_test()
{
using namespace std::chrono;

	auto print = [](char c) {
		for (int i = 0; i < 10; i++) {
			std::cout << c;
			std::cout.flush();
			std::this_thread::sleep_for(milliseconds(1));
		}
	};
	// 不同launch策略的效果
	std::launch policies[] = { std::launch::async, std::launch::deferred };
	const char *names[] = { "async   ", "deferred" };
	for (int i = 0; i < sizeof(policies) / sizeof(policies[0]); i++) {
		std::cout << names[i] << ": ";
		std::cout.flush();
		auto f1 = std::async(policies[i], print, '+');
		auto f2 = std::async(policies[i], print, '-');
		f1.get();
		f2.get();
		std::cout << std::endl;
	}
}

class A {
public:
	A() {
		std::cout << std::this_thread::get_id()
			<< " " << __FUNCTION__
			<< "(" << (void *)this << ")"
			<< std::endl;
	}

	~A() {
		std::cout << std::this_thread::get_id()
			<< " " << __FUNCTION__
			<< "(" << (void *)this << ")"
			<< std::endl;
	}

	// 线程中，第一次使用前初始化
	void doSth() {
	}
};

//thread_local A a;
void thread_local_test()
{
	//cout << "-----------" << endl;
	//a.doSth();
	//std::thread t([]() {
	//	std::cout << "Thread: "
	//		<< std::this_thread::get_id()
	//		<< " entered" << std::endl;
	//	a.doSth();
	//});

	//t.join();
}

void once_flag_fun()
{
	cout << "once_flag_fun:" << std::this_thread::get_id() << endl;
}

void call_once_flag_test()
{
	std::once_flag flag;
	for (int i = 0; i < 5; i++)
	{
		auto ret = std::async([&] {std::call_once(flag, once_flag_fun); });
	}
}

void atomic_test()
{
	std::atomic_flag flag = ATOMIC_FLAG_INIT;
	thread thrd[5];
	for (int i = 0; i < 5; i++)
	{
		thrd[i] = thread([&]
		{
			for (int n = 0; n < 500000; n++)
			{

			}
			cout << "thread exit:" << std::this_thread::get_id() << endl;
			if (!flag.test_and_set())
			{
				cout << "thread calc" << std::this_thread::get_id() << endl;
			}
		});
	}

	std::for_each(thrd, thrd + 5, [](thread& td) {td.join(); });
}

void atomic_type_test()
{
	std::atomic_int16_t n = 0;
	
	//for (int i = 0; i < 10; i++)
	//{
	//	std::async([&] {std::for_each(0, 100, [&] {n.fetch_add(1); }); });
	//}

	//int ret = n.load();

	std::vector<int> v{ 3, 1, 4 };

	auto it = v.end();

	auto nx = std::prev(it, 2);

	std::cout << *nx << '\n';
}

void fun()
{
	cout << endl;
}

template<typename T,typename... Args>
void fun(T&& t,Args &&... args)
{
	//cout << sizeof...(args) << endl;
	cout << t << endl;
	fun(std::forward<Args>(args)...);
}

template<typename T>
std::string to_str(const T &r) {
	std::stringstream ss;
	ss << "\"" << r << "\"";
	return ss.str();
}

template<typename... Args>
void init_vector(std::vector<std::string> &vec, const Args &...args) {
	// 复杂的包扩展方式
	vec.assign({ to_str(args)...});
}

void template_fun_test()
{
	std::vector<std::string> vec;
	init_vector(vec, 1, "hello", "world");
	std::cout << "vec.size => " << vec.size() << std::endl;
	for (auto r : vec) 
	{
		std::cout << r << std::endl;
	}

	//fun(1, "hello", "world");
}

void lock_fun_test()
{
	std::mutex m1;
	std::unique_lock<std::mutex> lock;
	{
		std::unique_lock<std::mutex> lo(m1);
		bool res = m1.try_lock();
		lock = std::move(lo);
	}

	bool res = m1.try_lock();
	//std::mutex m2;
	//thread td([&]
	//{
	//	std::lock(m1, m2);
	//	{
	//		bool res = m1.try_lock();
	//		std::lock_guard<std::mutex> lock1(m1, std::adopt_lock);
	//		std::lock_guard<std::mutex> lock2(m2, std::adopt_lock);
	//	}

	//	bool res = m1.try_lock();
	//});
	//td.join();
}

template<typename T,typename TContainer = std::queue<T>>
class thread_safe_queue
{
public:
	thread_safe_queue()
	{
	}
	~thread_safe_queue()
	{

	}
public:
	int size()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		return m_queue.size();
	}

	void push(const T& val)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_queue.push(val);
	}

	T pop()
	{
		std::lock_guard <std::mutex> lock(m_mutex);
		T val = m_queue.front();
		m_queue.pop();

		return val;
	}
private:
	TContainer m_queue;
	std::mutex m_mutex;
};

class student
{
public:
	student(string val)
		:name(val)
	{
		cout << "student constuct" << endl;
	}

	~student()
	{
		cout << "student destruct" << endl;
	}

	student(const student& val)
		:name(val.name)
	{

	}

	student& operator ==(const student& val)
	{
		if (this != &val)
		{
			this->name = val.name;
		}

		return *this;
	}

	student(const student&& val)
		:name(std::move(val.name))
	{

	}

	student&& operator == (const student&& val)
	{
		if (this != &val)
		{
			this->name = std::move(val.name);
		}

		return std::move(*this);
	}

public:
	string get_name()
	{
		return name;
	}
private:
	string name;
};

void thread_safe_queue_test()
{
	thread_safe_queue<std::shared_ptr<student>> queue;

	queue.push(std::make_shared<student>("student1"));

	std::shared_ptr<student> st2 = queue.pop();

	cout << st2->get_name() << endl;
}

template<typename T>
class buffer_queue
{
public:
	buffer_queue(int nMaxCnt)
		:m_nMaxCnt(nMaxCnt)
	{

	}

	~buffer_queue()
	{
		m_buffer.clear();
	}

public:
	void push(T* buf, int nLen)
	{

	}
private:
	std::list<std::shared_ptr<std::vector<T>>> m_buffer;
	int m_nMaxCnt;
};


int arr[2] = { 24, 42 };

auto getArrayRef() -> int(&)[2]
{
	return arr;
}

decltype(arr)& getArrayRef2()
{
	return arr;
}

void map_test()
{
	std::map<int, string> mp({ {1,"a"},{2,"b"},{3,"c"},{4,"d"} });

	//结构化绑定
	for (const auto& [key, val] : mp)
	{
		cout << "key=" << key << "," << "val=" << val << endl;
	}


	std::map<std::string, int> coll{ {"new", 42} };

	auto [iter, result] = coll.emplace("new", 42);
	if (!result)
	{
		//if emplace failed.
		std::cout << "emplace(\"new\", 42) failed" << std::endl;
	}
	else
	{
		std::cout << "emplace(\"new\", 42) successful" << std::endl;

	}

	//auto r2 = getArrayRef();
	//auto [x, y] = getArrayRef();
	//std::shared_ptr<student> sp(new student("a"), [](auto* pt)
	//{
	//	cout << "delete pt   " << typeid(pt).name() << endl;
	//	delete pt;
	//});

	//cout << sp.use_count() << endl;

	//std::weak_ptr<student> ap(sp);
	//cout << sp.use_count() << endl;
	//std::shared_ptr<student> s(ap);
	//cout << sp.use_count() << endl;
}


template <typename T>
std::string asString(T x)
{
	if constexpr (std::is_same_v<T, std::string>) {//第一个判断语句
		std::cout << "satement 1 output: ";
		return x; // statement invalid, if no conversion to string
	}
	else if constexpr (std::is_arithmetic_v<T>) { ////第二个判断语句
		std::cout << "satement 2 output: ";
		return std::to_string(x); // statement invalid, if x is not numeric
	}
	else { ////第三个判断语句
		std::cout << "satement 3 output: ";
		return std::string(x); // statement invalid, if no conversion to string
	}
}

//C++11 仅对lam
auto myFun()
{
	return "";
}

void cpp17_test()
{
	//std::cout << asString(42) << '\n';
	//std::cout << asString(std::string("hello")) << '\n';
	//std::cout << asString("hello") << '\n';
	//std::cout << asString("x2");

	//string str = "0x111";
	//size_t size = 0;
	//int val = std::stoi(str,&size,16);

	int val = 0b1111'0000;

	string myString = "My hello world string";
	if (auto it = myString.find("hello"); it != string::npos)
		cout << it << " - Hello\n";
	if (auto it = myString.find("world"); it != string::npos)
		cout << it << " - World\n";

}

//用户定义字面量 

struct Color
{
	int r, g, b;
};

//定义的字面量转换函数 
//在gcc编译器下注意双引号与后面的_c之间必须有一个空格 vs2014没有该要求
//_c可以任意取名字 一般最好用下划线(_)开头 避免与系统定义的冲突（比方说长整形后缀 L等））
Color operator ""_c(const char* str, size_t n)
{
	//字面常量格式为 r数字g数字b数字
	unsigned char r = 0, g = 0, b = 0;
	for (size_t i = 0; i < n; i++)
	{
		if (str[i] == 'r') r = (unsigned char)atoi(&str[i + 1]);
		else if (str[i] == 'g') g = (unsigned char)atoi(&str[i + 1]);
		else if (str[i] == 'b') b = (unsigned char)atoi(&str[i + 1]);
	}

	return { r, g, b };
}

long double operator""_km(long double val)
{
	return val * 1000;
}

void user_define_test()
{
	//这样使用 将把“r11g12b13”转换成Color结构体 字符串必须用引号括起来 后面必须跟一个_c（前面自定义）的后缀 且与前面的字符串不予许有其他包括空格等字符
	Color c = "r11g12b13"_c;

	long double val = 20.0_km;
}

void string_view_test()
{
	string str = "111222";
	std::string_view sv(str);
	bool res = str.starts_with("2");

}

template<auto Sep = ' ', typename First, typename... Args>
void print(First first, const Args& ... args)
{
	std::cout << first;
	auto outWithSpace = [](auto const& arg)
	{
		std::cout << Sep << arg;
	};

	(outWithSpace(args),...);
	//(..., outWithSpace(args));
	std::cout << '\n';

}

//C++17之占位符类型作为模板参数
void print_test()
{
	std::string s{ "world" };
	print<'-'>(7.5, "hello", s, "222"); // prints: 7.5-hello-worl
}

template <size_t n, typename... Args>
std::variant<Args...> _tuple_index(size_t i, const std::tuple<Args...>& tpl) {
	if (i == n)
		return std::get<n>(tpl);
	else if (n == sizeof...(Args) - 1)
		throw std::out_of_range("越界.");
	else
		return _tuple_index<(n < sizeof...(Args) - 1 ? n + 1 : 0)>(i, tpl);
}
template <typename... Args>
std::variant<Args...> tuple_index(size_t i, const std::tuple<Args...>& tpl) {
	return _tuple_index<0>(i, tpl);
}

void variant_test()
{
	std::tuple tp{ 1,"2",3.3f };
	std::variant<int,const char*,float> val = tuple_index(1, tp);
	int index = val.index();
	val = tuple_index(0, tp);

	auto v = std::get_if<int>(&val);
	bool res = v == nullptr;
	if (std::holds_alternative<const char*>(val)) {
		std::cout << "var1 is " << typeid(int).name() << std::endl;
	}
	else
	{
		cout << "no init" << endl;
	}
}


int main()
{
	//thread_test();
	//promise_test();
	//function_test();
	//time_system_test();
	//asyn_function_test();
	//future_test();
	//package_task_test();
	//asyn_call_test();
	//thread_local_test();
	//call_once_flag_test();
	//atomic_test();
	//atomic_type_test();
	//template_fun_test();
	//lock_fun_test();
	//thread_safe_queue_test();
	//map_test();
	//cpp17_test();
	string_view_test();
	user_define_test();
	print_test();
	variant_test();
	system("pause");
    return 0;
}

