#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "LeptJSON.hpp"
#include <iostream>

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

using Status = LeptJSON::Status;
using ValueType = LeptJSON::ValueType;

#define EXPECT_EQ_BASE(equality, expect, actual, format) \
    do {\
        test_count++;\
        if (equality)\
            test_pass++;\
        else {\
            fprintf(stderr, "%s:%d: expect: " format " actual: " format "\n", __FILE__, __LINE__, expect, actual);\
            main_ret = 1;\
        }\
    } while(0)

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")
#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%.17g")
#define EXPECT_EQ_STRING(expect, actual) EXPECT_EQ_BASE(expect == actual, expect, actual.data(), "%s")
#define EXPECT_TRUE(actual) EXPECT_EQ_BASE((actual) != 0, "true", "false", "%s")
#define EXPECT_FALSE(actual) EXPECT_EQ_BASE((actual) == 0, "false", "true", "%s")

namespace details {
void test_number(double expect_number, const char* json) {
	LeptJSON v(json);
	EXPECT_EQ_INT(Status::PARSE_OK, v.parse());
	EXPECT_EQ_INT(ValueType::NUMBER_TYPE, v.get_type());
	EXPECT_EQ_DOUBLE(expect_number, v.get_number());
}

void test_error(Status error, const char* json) {
	LeptJSON v(json, ValueType::FALSE_TYPE);
	EXPECT_EQ_INT(error, v.parse());
	EXPECT_EQ_INT(ValueType::NULL_TYPE, v.get_type());
}

void test_string(std::string_view expect_string, const char* json) {
	LeptJSON v(json);
	EXPECT_EQ_INT(Status::PARSE_OK, v.parse());
	EXPECT_EQ_INT(ValueType::STRING_TYPE, v.get_type());
	EXPECT_EQ_STRING(expect_string, v.get_string());
}
}

static void test_parse_null() {
	LeptJSON v("null", ValueType::TRUE_TYPE);
	EXPECT_EQ_INT(Status::PARSE_OK, v.parse());
	EXPECT_EQ_INT(ValueType::NULL_TYPE, v.get_type());
}

static void test_parse_true() {
	LeptJSON v("true", ValueType::FALSE_TYPE);
	EXPECT_EQ_INT(Status::PARSE_OK, v.parse());
	EXPECT_EQ_INT(ValueType::TRUE_TYPE, v.get_type());
}

static void test_parse_false() {
	LeptJSON v("false", ValueType::TRUE_TYPE);
	EXPECT_EQ_INT(Status::PARSE_OK, v.parse());
	EXPECT_EQ_INT(ValueType::FALSE_TYPE, v.get_type());
}

static void test_parse_number() {
	details::test_number(0.0, "0");
	details::test_number(0.0, "-0");
	details::test_number(0.0, "-0.0");
	details::test_number(1.0, "1");
	details::test_number(-1.0, "-1");
	details::test_number(1.5, "1.5");
	details::test_number(-1.5, "-1.5");
	details::test_number(3.1416, "3.1416");
	details::test_number(1E10, "1E10");
	details::test_number(1e10, "1e10");
	details::test_number(1E+10, "1E+10");
	details::test_number(1E-10, "1E-10");
	details::test_number(-1E10, "-1E10");
	details::test_number(-1e10, "-1e10");
	details::test_number(-1E+10, "-1E+10");
	details::test_number(-1E-10, "-1E-10");
	details::test_number(1.234E+10, "1.234E+10");
	details::test_number(1.234E-10, "1.234E-10");
	details::test_number(0.0, "1e-10000"); /* must underflow */

	details::test_number(1.0000000000000002, "1.0000000000000002"); /* the smallest number > 1 */
	details::test_number(4.9406564584124654e-324, "4.9406564584124654e-324"); /* minimum denormal */
	details::test_number(-4.9406564584124654e-324, "-4.9406564584124654e-324");
	details::test_number(2.2250738585072009e-308, "2.2250738585072009e-308");  /* Max subnormal double */
	details::test_number(-2.2250738585072009e-308, "-2.2250738585072009e-308");
	details::test_number(2.2250738585072014e-308, "2.2250738585072014e-308");  /* Min normal positive double */
	details::test_number(-2.2250738585072014e-308, "-2.2250738585072014e-308");
	details::test_number(1.7976931348623157e+308, "1.7976931348623157e+308");  /* Max double */
	details::test_number(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}

void test_parse_string() {
	details::test_string("", "\"\"");
	details::test_string("Hello", "\"Hello\"");
	details::test_string("Hello\nWorld", "\"Hello\\nWorld\"");
	details::test_string("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
}

static void test_parse_expect_value() {
	details::test_error(Status::PARSE_EXPECT_VALUE, "");
	details::test_error(Status::PARSE_EXPECT_VALUE, " ");
}

static void test_parse_invalid_value() {
	details::test_error(Status::PARSE_INVALID_VALUE, "nul");
	details::test_error(Status::PARSE_INVALID_VALUE, "?");

	/* invalid number */
	details::test_error(Status::PARSE_INVALID_VALUE, "+0");
	details::test_error(Status::PARSE_INVALID_VALUE, "+1");
	details::test_error(Status::PARSE_INVALID_VALUE, ".123");
	details::test_error(Status::PARSE_INVALID_VALUE, "1.");
	details::test_error(Status::PARSE_INVALID_VALUE, "INF");
	details::test_error(Status::PARSE_INVALID_VALUE, "inf");
	details::test_error(Status::PARSE_INVALID_VALUE, "NAN");
	details::test_error(Status::PARSE_INVALID_VALUE, "nan");
}

static void test_parse_root_not_singular() {
	details::test_error(Status::PARSE_ROOT_NOT_SINGULAR, "null x");

	/* invalid number */
	details::test_error(Status::PARSE_ROOT_NOT_SINGULAR, "0123");
	details::test_error(Status::PARSE_ROOT_NOT_SINGULAR, "0x0");
	details::test_error(Status::PARSE_ROOT_NOT_SINGULAR, "0x123");
}

static void test_parse_number_too_big() {
	details::test_error(Status::PARSE_NUMBER_TOO_BIG, "1e309");
	details::test_error(Status::PARSE_NUMBER_TOO_BIG, "-1e309");
}

static void test_parse_missing_quotation_mark() {
	details::test_error(Status::PARSE_MISS_QUOTATION_MARK, "\"");
	details::test_error(Status::PARSE_MISS_QUOTATION_MARK, "\"abc");
}

static void test_parse_invalid_string_escape() {
	details::test_error(Status::PARSE_INVALID_STRING_ESCAPE, "\"\\v\"");
	details::test_error(Status::PARSE_INVALID_STRING_ESCAPE, "\"\\'\"");
	details::test_error(Status::PARSE_INVALID_STRING_ESCAPE, "\"\\0\"");
	details::test_error(Status::PARSE_INVALID_STRING_ESCAPE, "\"\\x12\"");
}

static void test_parse_invalid_string_char() {
	details::test_error(Status::PARSE_INVALID_STRING_CHAR, "\"\x01\"");
	details::test_error(Status::PARSE_INVALID_STRING_CHAR, "\"\x1F\"");
}

static void test_access_null() {
	LeptJSON v;
	v.set_string("a");
	v.set_nullptr();
	EXPECT_EQ_INT(ValueType::NULL_TYPE, v.get_type());
}

static void test_access_boolean() {
	LeptJSON v;
	v.set_string("a");
	v.set_boolean(true);
	EXPECT_TRUE(v.get_boolean());
	v.set_boolean(false);
	EXPECT_FALSE(v.get_boolean());
}

static void test_access_number() {
	LeptJSON v;
	v.set_string("a");
	v.set_number(1234.5);
	EXPECT_EQ_DOUBLE(1234.5, v.get_number());
}

static void test_access_string() {
	LeptJSON v;
	v.set_string("");
	EXPECT_EQ_STRING("", v.get_string());
	v.set_string("Hello");
	EXPECT_EQ_STRING("Hello", v.get_string());
}

static void test_parse() {
	test_parse_null();
	test_parse_true();
	test_parse_false();
	test_parse_number();
	test_parse_string();
	test_parse_expect_value();
	test_parse_invalid_value();
	test_parse_root_not_singular();
	test_parse_number_too_big();
	test_parse_missing_quotation_mark();
	test_parse_invalid_string_escape();
	test_parse_invalid_string_char();

	test_access_null();
	test_access_boolean();
	test_access_number();
	test_access_string();
}

int main() {
#ifdef _WINDOWS
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	test_parse();
	printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
	return main_ret;
}