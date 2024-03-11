#include <stdio.h>
#include <stdlib.h>

#include "LeptJSON.hpp"
#include <iostream>
#include <string>

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
#define EXPECT_EQ_SIZE_T(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%zu")

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

void test_round_trip(const char* json) {
	LeptJSON v1(json);
	EXPECT_EQ_INT(Status::PARSE_OK, v1.parse());
	std::string json2 = v1.stringify();
	LeptJSON v2(json2.data());
	EXPECT_EQ_INT(Status::PARSE_OK, v2.parse());
	std::string json3 = v2.stringify();
	LeptJSON v3(json3.data());
	EXPECT_EQ_INT(Status::PARSE_OK, v3.parse());
	EXPECT_EQ_STRING(json2, json3);
}

void test_equal(const char* lhs, const char* rhs, bool result) {
	LeptJSON v1(lhs);
	EXPECT_EQ_INT(Status::PARSE_OK, v1.parse());
	LeptJSON v2(rhs);
	EXPECT_EQ_INT(Status::PARSE_OK, v2.parse());
	EXPECT_EQ_INT(result, is_equal(v1, v2));
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
	details::test_string(std::string_view{ "Hello\0World",11 }, "\"Hello\\u0000World\"");
	details::test_string("\x24", "\"\\u0024\"");         /* Dollar sign U+0024 */
	details::test_string("\xC2\xA2", "\"\\u00A2\"");     /* Cents sign U+00A2 */
	details::test_string("\xE2\x82\xAC", "\"\\u20AC\""); /* Euro sign U+20AC */
	details::test_string("\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\"");  /* G clef sign U+1D11E */
	details::test_string("\xF0\x9D\x84\x9E", "\"\\ud834\\udd1e\"");  /* G clef sign U+1D11E */
}

static void test_parse_array() {
	LeptJSON v("[ ]");
	EXPECT_EQ_INT(Status::PARSE_OK, v.parse());
	EXPECT_EQ_INT(ValueType::ARRAY_TYPE, v.get_type());
	EXPECT_EQ_SIZE_T(std::size_t{}, v.get_array().size());

	v.set_json("[ null , false , true , 123 , \"abc\" ]");
	EXPECT_EQ_INT(Status::PARSE_OK, v.parse());
	EXPECT_EQ_INT(ValueType::ARRAY_TYPE, v.get_type());
	EXPECT_EQ_SIZE_T(std::size_t{ 5 }, v.get_array().size());
	EXPECT_EQ_INT(ValueType::NULL_TYPE, get_type(v.get_array()[0]));
	EXPECT_EQ_INT(ValueType::FALSE_TYPE, get_type(v.get_array()[1]));
	EXPECT_EQ_INT(ValueType::TRUE_TYPE, get_type(v.get_array()[2]));
	EXPECT_EQ_INT(ValueType::NUMBER_TYPE, get_type(v.get_array()[3]));
	EXPECT_EQ_INT(ValueType::STRING_TYPE, get_type(v.get_array()[4]));
	EXPECT_EQ_DOUBLE(123.0, get_number((v.get_array()[3])));
	EXPECT_EQ_STRING("abc", get_string(v.get_array()[4]));

	v.set_json("[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]");
	EXPECT_EQ_INT(Status::PARSE_OK, v.parse());
	EXPECT_EQ_INT(ValueType::ARRAY_TYPE, v.get_type());
	for (std::size_t i = 0; i < 4; i++) {
		auto a = v.get_array()[i];
		EXPECT_EQ_INT(ValueType::ARRAY_TYPE, get_type(a));
		EXPECT_EQ_SIZE_T(i, get_array(a).size());
		for (int j = 0; j < i; j++) {
			EXPECT_EQ_INT(ValueType::NUMBER_TYPE, get_type(get_array(a)[j]));
			EXPECT_EQ_DOUBLE((double) j, get_number(get_array(a)[j]));
		}
	}
}

static void test_parse_object() {
	LeptJSON v("{ }");
	EXPECT_EQ_INT(Status::PARSE_OK, v.parse());
	EXPECT_EQ_INT(ValueType::OBJECT_TYPE, v.get_type());
	EXPECT_EQ_SIZE_T(std::size_t{}, v.get_object().size());

	v.set_json(
		" { "
		"\"n\" : null , "
		"\"f\" : false , "
		"\"t\" : true , "
		"\"i\" : 123 , "
		"\"s\" : \"abc\", "
		"\"a\" : [ 1, 2, 3 ],"
		"\"o\" : { \"1\" : 1, \"2\" : 2, \"3\" : 3 }"
		" } "
	);
	EXPECT_EQ_INT(Status::PARSE_OK, v.parse());
	EXPECT_EQ_INT(ValueType::OBJECT_TYPE, v.get_type());
	EXPECT_EQ_SIZE_T(std::size_t{ 7 }, v.get_object().size());
	EXPECT_EQ_INT(ValueType::NULL_TYPE, get_type((v.get_object().at("n"))));
	EXPECT_EQ_INT(ValueType::FALSE_TYPE, get_type((v.get_object().at("f"))));
	EXPECT_EQ_INT(ValueType::TRUE_TYPE, get_type((v.get_object().at("t"))));
	EXPECT_EQ_INT(ValueType::NUMBER_TYPE, get_type((v.get_object().at("i"))));
	EXPECT_EQ_DOUBLE(123.0, get_number((v.get_object().at("i"))));
	EXPECT_EQ_INT(ValueType::STRING_TYPE, get_type((v.get_object().at("s"))));
	EXPECT_EQ_STRING("abc", get_string(v.get_object().at("s")));
	EXPECT_EQ_INT(ValueType::ARRAY_TYPE, get_type(v.get_object().at("a")));
	EXPECT_EQ_SIZE_T(std::size_t{ 3 }, get_array(v.get_object().at("a")).size());
	for (std::size_t i = 0; i < 3; i++) {
		EXPECT_EQ_INT(ValueType::NUMBER_TYPE, get_type(get_array(v.get_object().at("a"))[i]));
		EXPECT_EQ_DOUBLE(i + 1.0, get_number(get_array(v.get_object().at("a"))[i]));
	}
	{
		auto o = v.get_object().at("o");
		EXPECT_EQ_INT(ValueType::OBJECT_TYPE, get_type(o));
		EXPECT_EQ_SIZE_T(std::size_t{ 3 }, get_object(o).size());
		for (std::size_t i = 0; i < 3; i++) {
			auto key = std::to_string(i + 1);
			EXPECT_EQ_INT(ValueType::NUMBER_TYPE, get_type(get_object(o).at(key)));
			EXPECT_EQ_DOUBLE(i + 1.0, get_number(get_object(o).at(key)));
		}

	}
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

	details::test_error(Status::PARSE_INVALID_VALUE, "[1,]");
	details::test_error(Status::PARSE_INVALID_VALUE, "[\"a\", nul]");
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

static void test_parse_invalid_unicode_hex() {
	details::test_error(Status::PARSE_INVALID_UNICODE_HEX, "\"\\u\"");
	details::test_error(Status::PARSE_INVALID_UNICODE_HEX, "\"\\u0\"");
	details::test_error(Status::PARSE_INVALID_UNICODE_HEX, "\"\\u01\"");
	details::test_error(Status::PARSE_INVALID_UNICODE_HEX, "\"\\u012\"");
	details::test_error(Status::PARSE_INVALID_UNICODE_HEX, "\"\\u/000\"");
	details::test_error(Status::PARSE_INVALID_UNICODE_HEX, "\"\\uG000\"");
	details::test_error(Status::PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
	details::test_error(Status::PARSE_INVALID_UNICODE_HEX, "\"\\u0G00\"");
	details::test_error(Status::PARSE_INVALID_UNICODE_HEX, "\"\\u00/0\"");
	details::test_error(Status::PARSE_INVALID_UNICODE_HEX, "\"\\u00G0\"");
	details::test_error(Status::PARSE_INVALID_UNICODE_HEX, "\"\\u000/\"");
	details::test_error(Status::PARSE_INVALID_UNICODE_HEX, "\"\\u000G\"");
	details::test_error(Status::PARSE_INVALID_UNICODE_HEX, "\"\\u 123\"");
}

static void test_parse_invalid_unicode_surrogate() {
	details::test_error(Status::PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\"");
	details::test_error(Status::PARSE_INVALID_UNICODE_SURROGATE, "\"\\uDBFF\"");
	details::test_error(Status::PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\\\\"");
	details::test_error(Status::PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uDBFF\"");
	details::test_error(Status::PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uE000\"");
}

static void test_parse_miss_comma_or_square_bracket() {
	details::test_error(Status::PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1");
	details::test_error(Status::PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1}");
	details::test_error(Status::PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1 2");
	details::test_error(Status::PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[[]");
}

static void test_parse_miss_key() {
	details::test_error(Status::PARSE_MISS_KEY, "{:1,");
	details::test_error(Status::PARSE_MISS_KEY, "{1:1,");
	details::test_error(Status::PARSE_MISS_KEY, "{true:1,");
	details::test_error(Status::PARSE_MISS_KEY, "{false:1,");
	details::test_error(Status::PARSE_MISS_KEY, "{null:1,");
	details::test_error(Status::PARSE_MISS_KEY, "{[]:1,");
	details::test_error(Status::PARSE_MISS_KEY, "{{}:1,");
	details::test_error(Status::PARSE_MISS_KEY, "{\"a\":1,");
}

static void test_parse_miss_colon() {
	details::test_error(Status::PARSE_MISS_COLON, "{\"a\"}");
	details::test_error(Status::PARSE_MISS_COLON, "{\"a\",\"b\"}");
}

static void test_parse_miss_comma_or_curly_bracket() {
	details::test_error(Status::PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1");
	details::test_error(Status::PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1]");
	details::test_error(Status::PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1 \"b\"");
	details::test_error(Status::PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":{}");
}

static void test_parse() {
	test_parse_null();
	test_parse_true();
	test_parse_false();
	test_parse_number();
	test_parse_string();
	test_parse_array();
	test_parse_object();

	test_parse_expect_value();
	test_parse_invalid_value();
	test_parse_root_not_singular();
	test_parse_number_too_big();
	test_parse_missing_quotation_mark();
	test_parse_invalid_string_escape();
	test_parse_invalid_string_char();
	test_parse_invalid_unicode_hex();
	test_parse_invalid_unicode_surrogate();
	test_parse_miss_comma_or_square_bracket();
	test_parse_miss_key();
	test_parse_miss_colon();
	test_parse_miss_comma_or_curly_bracket();
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

static void test_access_array() {
	LeptJSON v, temp;
	for (std::size_t i = 0; i <= 5; i += 5) {
		v.set_array({});
		v.get_array().reserve(i);
		EXPECT_EQ_SIZE_T(std::size_t{ 0 }, v.get_array().size());
		EXPECT_EQ_SIZE_T(i, v.get_array().capacity());
		for (int j = 0; j < 10; j++) {
			temp.set_number(j);
			v.get_array().push_back(std::move(temp.get_value()));
		}
		EXPECT_EQ_SIZE_T(std::size_t{ 10 }, v.get_array().size());
		for (int j = 0; j < 10; j++) {
			EXPECT_EQ_DOUBLE(double(j), get_number(v.get_array()[j]));
		}
	}

	v.get_array().pop_back();
	EXPECT_EQ_SIZE_T(std::size_t{ 9 }, v.get_array().size());
	for (int i = 0; i < 9; i++) {
		EXPECT_EQ_DOUBLE(double(i), get_number(v.get_array()[i]));
	}

	v.get_array().erase(v.get_array().begin() + 4, v.get_array().begin() + 4);
	EXPECT_EQ_SIZE_T(std::size_t{ 9 }, v.get_array().size());
	for (int i = 0; i < 9; i++) {
		EXPECT_EQ_DOUBLE(double(i), get_number(v.get_array()[i]));
	}

	v.get_array().erase(v.get_array().begin() + 8, v.get_array().begin() + 9);
	EXPECT_EQ_SIZE_T(std::size_t{ 8 }, v.get_array().size());
	for (int i = 0; i < 8; i++) {
		EXPECT_EQ_DOUBLE(double(i), get_number(v.get_array()[i]));
	}

	v.get_array().erase(v.get_array().begin(), v.get_array().begin() + 2);
	EXPECT_EQ_SIZE_T(std::size_t{ 6 }, v.get_array().size());
	for (int i = 0; i < 6; i++) {
		EXPECT_EQ_DOUBLE(i + 2.0, get_number(v.get_array()[i]));
	}

	for (int i = 0; i < 2; i++) {
		temp.set_number(i);
		v.get_array().insert(v.get_array().begin() + i, std::move(temp.get_value()));
	}

	EXPECT_EQ_SIZE_T(std::size_t{ 8 }, v.get_array().size());
	for (int i = 0; i < 8; i++) {
		EXPECT_EQ_DOUBLE(double(i), get_number(v.get_array()[i]));
	}

	EXPECT_TRUE(v.get_array().capacity() > 8);
	v.get_array().shrink_to_fit();
	EXPECT_EQ_SIZE_T(std::size_t{ 8 }, v.get_array().size());
	EXPECT_EQ_SIZE_T(std::size_t{ 8 }, v.get_array().capacity());
	for (int i = 0; i < 8; i++) {
		EXPECT_EQ_DOUBLE(double(i), get_number(v.get_array()[i]));
	}

	temp.set_string("Hello");
	v.get_array().push_back(std::move(temp.get_value()));

	std::size_t old_capacity = v.get_array().capacity();
	v.get_array().clear();
	EXPECT_EQ_SIZE_T(0, v.get_array().size());
	EXPECT_EQ_SIZE_T(old_capacity, v.get_array().capacity());
	v.get_array().shrink_to_fit();
	EXPECT_EQ_SIZE_T(0, v.get_array().capacity());
}

static void test_access_object() {
	LeptJSON v, temp;
	v.set_object({});
	EXPECT_EQ_SIZE_T(std::size_t{}, v.get_object().size());
	for (int i = 0; i < 10; i++) {
		char key = 'a' + i;
		temp.set_number(i);
		v.get_object()[{ key }] = std::move(temp.get_value());
		EXPECT_EQ_DOUBLE(i, get_number(v.get_object()[{key}]));
	}
	EXPECT_EQ_SIZE_T(10, v.get_object().size());
	EXPECT_TRUE(v.get_object().contains("a"));
	v.get_object().erase("a");
	EXPECT_FALSE(v.get_object().contains("a"));
	v.get_object().erase(v.get_object().begin(), v.get_object().end());
	EXPECT_EQ_SIZE_T(std::size_t{}, v.get_object().size());
	temp.set_string("23333");
	v.get_object().insert({ "le",temp.get_value() });
	EXPECT_EQ_SIZE_T(std::size_t{ 1 }, v.get_object().size());
	EXPECT_TRUE(v.get_object().contains("le"));
	EXPECT_EQ_STRING("23333", get_string(v.get_object()["le"]));
	v.get_object().clear();
	EXPECT_EQ_SIZE_T(std::size_t{}, v.get_object().size());
}

static void test_access() {
	test_access_null();
	test_access_boolean();
	test_access_number();
	test_access_string();
	test_access_array();
	test_access_object();
}

static void test_stringify_number() {
	details::test_round_trip("0");
	details::test_round_trip("-0");
	details::test_round_trip("1");
	details::test_round_trip("-1");
	details::test_round_trip("1.5");
	details::test_round_trip("-1.5");
	details::test_round_trip("3.25");
	details::test_round_trip("1e+20");
	details::test_round_trip("1.234e+20");
	details::test_round_trip("1.234e-20");

	details::test_round_trip("1.0000000000000002"); /* the smallest number > 1 */
	details::test_round_trip("4.9406564584124654e-324"); /* minimum denormal */
	details::test_round_trip("-4.9406564584124654e-324");
	details::test_round_trip("2.2250738585072009e-308");  /* Max subnormal double */
	details::test_round_trip("-2.2250738585072009e-308");
	details::test_round_trip("2.2250738585072014e-308");  /* Min normal positive double */
	details::test_round_trip("-2.2250738585072014e-308");
	details::test_round_trip("1.7976931348623157e+308");  /* Max double */
	details::test_round_trip("-1.7976931348623157e+308");
}

static void test_stringify_string() {
	details::test_round_trip("\"\"");
	details::test_round_trip("\"Hello\"");
	details::test_round_trip("\"Hello\\nWorld\"");
	details::test_round_trip("\"\\\" \\\\ / \\b \\f \\n \\r \\t\"");
	details::test_round_trip("\"Hello\\u0000World\"");
}

static void test_stringify_array() {
	details::test_round_trip("[]");
	details::test_round_trip("[null,false,true,123,\"abc\",[1,2,3]]");
}

static void test_stringify_object() {
	details::test_round_trip("{}");
	details::test_round_trip("{\"n\":null,\"f\":false,\"t\":true,\"i\":123,\"s\":\"abc\",\"a\":[1,2,3],\"o\":{\"1\":1,\"2\":2,\"3\":3}}");
}

static void test_stringify() {
	details::test_round_trip("null");
	details::test_round_trip("false");
	details::test_round_trip("true");
	test_stringify_number();
	test_stringify_string();
	test_stringify_array();
	test_stringify_object();
}

static void test_equal() {
	details::test_equal("true", "true", true);
	details::test_equal("true", "false", false);
	details::test_equal("false", "false", true);
	details::test_equal("null", "null", true);
	details::test_equal("null", "0", false);
	details::test_equal("123", "123", true);
	details::test_equal("123", "456", false);
	details::test_equal("\"abc\"", "\"abc\"", true);
	details::test_equal("\"abc\"", "\"abcd\"", false);
	details::test_equal("[]", "[]", true);
	details::test_equal("[]", "null", false);
	details::test_equal("[1,2,3]", "[1,2,3]", true);
	details::test_equal("[1,2,3]", "[1,2,3,4]", false);
	details::test_equal("[[]]", "[[]]", true);
	details::test_equal("{}", "{}", true);
	details::test_equal("{}", "null", false);
	details::test_equal("{}", "[]", false);
	details::test_equal("{\"a\":1,\"b\":2}", "{\"a\":1,\"b\":2}", true);
	details::test_equal("{\"a\":1,\"b\":2}", "{\"b\":2,\"a\":1}", true);
	details::test_equal("{\"a\":1,\"b\":2}", "{\"a\":1,\"b\":3}", false);
	details::test_equal("{\"a\":1,\"b\":2}", "{\"a\":1,\"b\":2,\"c\":3}", false);
	details::test_equal("{\"a\":{\"b\":{\"c\":{}}}}", "{\"a\":{\"b\":{\"c\":{}}}}", true);
	details::test_equal("{\"a\":{\"b\":{\"c\":{}}}}", "{\"a\":{\"b\":{\"c\":[]}}}", false);
}

void test_copy() {
	LeptJSON v1("{\"t\":true,\"f\":false,\"n\":null,\"d\":1.5,\"a\":[1,2,3]}");
	v1.parse();
	LeptJSON v2(v1);
	EXPECT_TRUE(is_equal(v2, v1));
	LeptJSON v3;
	copy(v3, v1);
	EXPECT_TRUE(is_equal(v3, v1));
}

void test_move() {
	LeptJSON v1("{\"t\":true,\"f\":false,\"n\":null,\"d\":1.5,\"a\":[1,2,3]}");
	v1.parse();
	LeptJSON v2(v1);
	LeptJSON v3;
	v3 = std::move(v2);
	EXPECT_TRUE(is_equal(v3, v1));
	EXPECT_EQ_INT(ValueType::NULL_TYPE, v2.get_type());
}

void test_swap() {
	LeptJSON v1, v2;
	v1.set_string("Hello");
	v2.set_string("World");
	swap(v1, v2);
	EXPECT_EQ_STRING("Hello", v2.get_string());
	EXPECT_EQ_STRING("World", v1.get_string());
	v1.swap(v2);
	EXPECT_EQ_STRING("Hello", v1.get_string());
	EXPECT_EQ_STRING("World", v2.get_string());
}

int main() {
#ifdef _WINDOWS
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	test_parse();
	test_access();
	test_stringify();
	test_equal();
	test_copy();
	test_move();
	test_swap();
	printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
	return main_ret;
}