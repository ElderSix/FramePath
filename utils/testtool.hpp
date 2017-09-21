#include <iostream>
#include <vector>
#include <string>
#include <functional>

/*
Sample of unit test
TEST(ModuleName, FuncName) {
    ASSERT_OK(func_to_test());
    ASSERT_LT(func_to_test());
    ASSERT_LE(func_to_test());
    ASSERT_GT ditto...
    ASSERT_GE ditto...
    ASSERT_TRUE ditto...
    ASSERT_EQ ditto...
}

TEST(ModuleName, FuncName):
1. declare a foo class with ModuleName, FuncName and RunFunc
2. call test register func:
    1. make a instance of foo class
    2. set member of foo class instance
    3. add test to global test suit vector

void run_all_tests():
1. get global test suit vector
2. run RunFunc of each foo class instance
*/

namespace frame_path {
namespace test {

#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

struct test_case {
    std::string module_name;
    std::string test_name;
    std::function<void()> run;
};
std::vector<test_case> all_tests;

class test_assert_op {
public:
    test_assert_op(std::string file, int line_number)
    :file_name(file),line_number(line_number) {}
#define ASSERT_OP(func_name, op)                \
    template <typename X, typename Y>           \
    void func_name(const X& a, const Y& b) {    \
        if(!(a op b)) {                         \
            std::cout<<RED<<"[Test failed at line "<<line_number<<" of "<<file_name<<"]: "    \
            <<a<<" "#op" "<<b<<std::endl;       \
        }                                       \
    }                                           \
    
    ASSERT_OP(is_eq, ==)
    ASSERT_OP(is_lt, <)
    ASSERT_OP(is_le, <=)
    ASSERT_OP(is_gt, >)
    ASSERT_OP(is_ge, >=)
#undef ASSERT_OP
private:
    std::string file_name;
    int line_number;
};

#define ASSERT_EQ(a, b) test::test_assert_op(__FILE__, __LINE__).is_eq((a), (b));
#define ASSERT_LT(a, b) test::test_assert_op(__FILE__, __LINE__).is_lt((a), (b));
#define ASSERT_LE(a, b) test::test_assert_op(__FILE__, __LINE__).is_le((a), (b));
#define ASSERT_GT(a, b) test::test_assert_op(__FILE__, __LINE__).is_gt((a), (b));
#define ASSERT_GE(a, b) test::test_assert_op(__FILE__, __LINE__).is_ge((a), (b));

bool register_test_case(std::string module_name, std::string test_name, std::function<void()> func);

#define TCONCAT(a, b) TCONCAT1(a, b)
#define TCONCAT1(a, b) a##b

#define TEST(module_name, test_name)         \
class TCONCAT(_TEST_, test_name) {           \
public:                                      \
    void run();                              \
    static void run_test() {                 \
        TCONCAT(_TEST_, test_name) t;        \
        t.run();                             \
    }                                        \
};                                           \
bool TCONCAT(_TEMP_REG_, test_name) =        \
    test::register_test_case(#module_name, #test_name, &TCONCAT(_TEST_, test_name)::run_test);  \
void TCONCAT(_TEST_, test_name)::run()

bool register_test_case(std::string module_name, std::string test_name, std::function<void()> func) {
    test_case t;
    t.module_name = module_name;
    t.test_name = test_name;
    t.run = func;
    all_tests.push_back(t);
    return true;
}

void run_all_tests() {
    std::cout<<YELLOW<<"=== Frame Path Tests Start ==="<<RESET<<std::endl;    
    int n = 0;
    for(auto test_case : all_tests) {
        std::cout<<YELLOW<<"=== Test "<<test_case.test_name<<RESET<<std::endl;
        test_case.run();
        n++;
    }
    std::cout<<YELLOW<<"=== Total "<<n<<" test(s) executed ==="<<RESET<<std::endl;    
    std::cout<<YELLOW<<"=== Frame Path Tests End ==="<<RESET<<std::endl;    
}

}
}
