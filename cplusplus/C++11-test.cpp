#include <thread>
#include <iostream>

using namespace std;
struct alignas(16) Bar {
    int i;                      // 4
    long long n;                // 8
    char arr[3];                // 3
    short s;                    // 2
};


#if __cplusplus < 201103L
#error "should use C++11 implementation"
#endif

class TestStruct
{
public:
    TestStruct(): name(__func__) {};
    virtual ~TestStruct() {};

    const char *name;
};

void Throw() {throw 1;}
void NoBlockThrow() {Throw();}
void BlockThrow() noexcept {Throw();}

static_assert(sizeof(int) != 8, "Not is 64-bit machine should follw this!");

thread_local int n = 1;
void func() {
    n++;
    cout << "id = " << this_thread::get_id() << ", n = " << n << endl;
}


int main()
{
    cout << "Standard Clib:" << __STDC_HOSTED__ << endl;
    cout << "Standard C:" << __STDC__ << endl;
    cout << "max of long long = " << LLONG_MAX << endl;
    cout << "min of long long = " << LLONG_MIN << endl;
    cout << "max of unsigned long long = " << ULLONG_MAX << endl;
    TestStruct ts;
    cout << ts.name << endl;


    thread t1(func);
    func();
    thread t2(func);
    t1.join();
    t2.join();
    cout << "id = " << this_thread::get_id() << ", n = " << n <<"\n";


    cout << "sizeof(Bar) = " << sizeof(Bar) << "\n";
    Bar bar;
    cout << &bar << endl;
    cout << &bar.i << endl;
    cout << &bar.n << endl;
    cout << &bar.arr << endl;
    cout << &bar.s << endl;


    try {
        Throw();
    } catch (...) {
        cout << "Found throw." << "\n";
    }
    try {
        NoBlockThrow();
    } catch (...) {
        cout << "Throw is not blocked." << "\n";
    }
    try {
        BlockThrow();
    } catch (...) {
        cout << "Found throw 1" << "\n";
    }

    return 0;
}
