
/******************************************************************************
 *                            Test utility tool				                  *
 ******************************************************************************/
/******************************************************************************
 * how to use this test?
 *
 * Test() is a function that accepts two parameters and checks them for equality.
 * the function also contain internal variables for statistic usage.
 *
 * BoolTest() works similarly, but accepts a single statement which returns true
 * or false, similarly to assert() function. BoolTest should be used for
 * test which examine elements which are not primitive types.
 *
 * TestSummary() should be called after all tests in test file, and will output
 * some usefull statistics about the tests executed.
 *
 * NOTE: any object of any class can use these functions as long as the these
 * operators are defined for the class: equality operators(!= , ==),
 * logical negation operator(!) and output stream operator (<<).
 * using the util fucntions with objects of class type without these operators
 * pre-defined will result in an undefined behaviour.
 ******************************************************************************/


#ifndef CA_TEST_UTIL_LIBRARY_H
#define CA_TEST_UTIL_LIBRARY_H

#include <string>
#include <iostream>
#include <cstring>

#define Test(x,y) (imp_template_check((x),(y),(__LINE__), (__FILE__), (__FUNCTION__)))
#define BoolTest(x) (imp_boolean_check((x),(__LINE__), (__FILE__), (__FUNCTION__)))

namespace ca_test_util
{

static std::size_t CHECK_FAILED_COUNT = 0;
static std::size_t CHECK_SUCCESS_COUNT = 0;

const std::string RED = "\033[1;31m";
const std::string GREEN = "\033[1;32m";
const std::string YELLOW = "\033[1;33m";
const std::string BLUE = "\033[1;34m";
const std::string MAGENTA = "\033[1;35m";
const std::string CYAN = "\033[1;36m";
const std::string DEFUALT_COLOR = "\033[0m";

const std::string UNDERLINE = "\033[4m";

void
PrintFatiledDetails(int line, const std::string &file, const std::string &func);

template<typename T>
inline void imp_template_check(const T &x,const T &y, int line,
                               const std::string &file, const std::string &func)
{
	if(x != y)
	{
		std::cout << std::endl << RED
		          <<  "test failed" << DEFUALT_COLOR << std::endl;
		PrintFatiledDetails(line, file, func);
		std::cout << MAGENTA << "expected: " << x << std::endl <<
		          "actual: " << y << std::endl << std::endl << DEFUALT_COLOR;
		++CHECK_FAILED_COUNT;
		std::cout << RED << "FAILURE" << DEFUALT_COLOR << std::endl;
	}
	else
	{
		++CHECK_SUCCESS_COUNT;
		std::cout << GREEN << "SUCCESS" << DEFUALT_COLOR << std::endl;
	}
}

void
PrintFatiledDetails(int line, const std::string &file, const std::string &func)
{
	std::cout << DEFUALT_COLOR
	          << "\tin line: " << line << std::endl
	          << "\tin function: " << func << std::endl
	          << "\tin file: " << file.substr(file.rfind('/') + 1) << DEFUALT_COLOR << std::endl;

}

void imp_boolean_check(bool b, int line, const std::string &file,
                       const std::string &func)
{
	if(!b)
	{
		std::cout << std::endl << RED
		          <<  "bool test failed" << DEFUALT_COLOR << std::endl;
		PrintFatiledDetails(line, file, func);
		std::cout << MAGENTA << "expression is false" <<
		DEFUALT_COLOR << std::endl << std::endl;
		++CHECK_FAILED_COUNT;
		std::cout << RED << "FAILURE" << DEFUALT_COLOR << std::endl;
	}
	else
	{
		++CHECK_SUCCESS_COUNT;
		std::cout << GREEN << "SUCCESS" << DEFUALT_COLOR << std::endl;
	}
}

void TestSummary()
{
	double __RES__ = (CHECK_FAILED_COUNT + CHECK_SUCCESS_COUNT);


	std::cout << std::endl << UNDERLINE << YELLOW << "TEST SUMMARY" << DEFUALT_COLOR << std::endl;
	std::cout << "OVERALL TESTS: " << (std::size_t)__RES__ << std::endl;
	std::cout << "SUCCESSFUL TESTS: " << CHECK_SUCCESS_COUNT << std::endl;
	if (__RES__)
	{
		__RES__ = (double)((CHECK_SUCCESS_COUNT)/(__RES__));
		__RES__ *= 100;
		(__RES__ != 100) ? std::cout << YELLOW << "SUCCESS RATE %: " <<  __RES__
		                 : std::cout << GREEN << "SUCCESS RATE %: " <<  __RES__;
		std::cout << DEFUALT_COLOR << std::endl;
	}
}


} // namespace ca_test_util


#endif //CA_TEST_UTIL_LIBRARY_H
