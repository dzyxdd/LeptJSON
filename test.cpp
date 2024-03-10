#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "LeptJSON.hpp"

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

using Status = LeptJSON::Status;
using ValueType = LeptJSON::ValueType;

#define EXPECT_EQ_BASE(equality, expect, actual, format) \
    do {\
        test_count++;\
		printf("%d\n: ",test_count);\
        if (equality)\
            test_pass++;\
        else {\
            fprintf(stderr, "%s:%d: expect: " format " actual: " format "\n", __FILE__, __LINE__, expect, actual);\
            main_ret = 1;\
        }\
    } while(0)

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")
#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%.17g")

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

static void test_parse() {
	test_parse_null();
	test_parse_true();
	test_parse_false();
	test_parse_number();
	test_parse_expect_value();
	test_parse_invalid_value();
	test_parse_root_not_singular();
	test_parse_number_too_big();
}

int main() {
	test_parse();
	printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
	return main_ret;
}