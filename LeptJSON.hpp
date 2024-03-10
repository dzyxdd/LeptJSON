/*
* learn from miloyip's json-tutorial
* url: https://github.com/miloyip/json-tutorial.git
*/

#pragma once
#ifndef _LEPTJSON_H_
#define _LEPTJSON_H_
#include <cassert>
#include <cctype>
#include <string_view>

struct LeptJSON {

	enum class json_type { NULL_TYPE, FALSE_TYPE, TRUE_TYPE, NUMBER_TYPE, STRING_TYPE, ARRAY_TYPE, OBJECT_TYPE };

	enum class status { PARSE_OK = 0, PARSE_EXPECT_VALUE, PARSE_INVALID_VALUE, PARSE_ROOT_NOT_SINGULAR, PARSE_NUMBER_TOO_BIG };

	using number_type = double;

	json_type type;
	const char* json;
	number_type number;

	json_type get_type()const {
		return type;
	}

	number_type get_number()const {
		assert(type == json_type::NUMBER_TYPE);
		return number;
	}



	status parse() {
		type = json_type::NULL_TYPE;
		parse_whitespace();
		auto ret = parse_value();
		if (ret == status::PARSE_OK) {
			parse_whitespace();
			if (*json != '\0') {
				type = json_type::NULL_TYPE;
				ret = status::PARSE_ROOT_NOT_SINGULAR;
			}
		}
		return ret;
	}

	status parse(const char* input) {
		json = input;
		return parse();
	}

private:
	/* value = null / false / true / number */
	status parse_value() {
		switch (*json) {
		case 't':return parse_literal("true", json_type::TRUE_TYPE);
		case 'f':return parse_literal("false", json_type::FALSE_TYPE);
		case 'n':return parse_literal("null", json_type::NULL_TYPE);
		case '\0':return status::PARSE_EXPECT_VALUE;
		default:return parse_number();
		}
	}

	/* ws = *(%x20 / %x09 / %x0A / %x0D) */
	void parse_whitespace() {
		while (*json == ' ' || *json == '\t' || *json == '\n' || *json == '\r') {
			++json;
		}
	}

	/* literal = "null" / "false" / "true" */
	status parse_literal(std::string_view literal, json_type type) {
		assert(*json == literal[0]);
		for (size_t i = 0; i < literal.size(); ++i) {
			if (json[i] != literal[i]) {
				return status::PARSE_INVALID_VALUE;
			}
		}
		json += literal.size();
		this->type = type;
		return status::PARSE_OK;
	}

	/*
	 * number = [ "-" ] int [ frac ] [ exp ]
	 * int = "0" / digit1-9 *digit
	 * frac = "." 1*digit
	 * exp = ("e" / "E") ["-" / "+"] 1*digit
	 */
	status parse_number() {
		const char* judge = json;
		if (*judge == '-') ++judge;
		if (*judge == '0') ++judge;
		else {
			if (!isdigit(*judge)) return status::PARSE_INVALID_VALUE;
			for (++judge; isdigit(*judge); ++judge);
		}
		if (*judge == '.') {
			++judge;
			if (!isdigit(*judge)) return status::PARSE_INVALID_VALUE;
			for (++judge; isdigit(*judge); ++judge);
		}
		if (*judge == 'e' || *judge == 'E') {
			++judge;
			if (*judge == '+' || *judge == '-') ++judge;
			if (!isdigit(*judge)) return status::PARSE_INVALID_VALUE;
			for (++judge; isdigit(*judge); ++judge);
		}

		errno = 0;
		number = strtod(json, nullptr);
		if (errno == ERANGE && (number == HUGE_VAL || number == -HUGE_VAL)) {
			return status::PARSE_NUMBER_TOO_BIG;
		}
		json = judge;
		type = json_type::NUMBER_TYPE;
		return status::PARSE_OK;
	}
};



#endif/* _LEPTJSON_H_ */

