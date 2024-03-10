/*
* learn from miloyip's json-tutorial
* url: https://github.com/miloyip/json-tutorial.git
*/

#pragma once
#ifndef _LEPTJSON_H_
#define _LEPTJSON_H_
#include <cassert>
#include <cctype>
#include <variant>
#include <string_view>

#define JSON_VALUE std::visit(visitor, jsonValue.value)

struct LeptJSON {

	enum class ValueType { NULL_TYPE, FALSE_TYPE, TRUE_TYPE, NUMBER_TYPE, STRING_TYPE, ARRAY_TYPE, OBJECT_TYPE };

	enum class Status { PARSE_OK = 0, PARSE_EXPECT_VALUE, PARSE_INVALID_VALUE, PARSE_ROOT_NOT_SINGULAR, PARSE_NUMBER_TOO_BIG };

private:
	struct {
		std::variant<double>value;
		ValueType type;
	}jsonValue;

	inline static auto visitor = []<typename _Ty>(_Ty && arg)->_Ty { return arg; };

	std::string_view json;

public:
	explicit LeptJSON(const char* js, ValueType vt = ValueType::NULL_TYPE)
		:jsonValue({}, vt), json(js) {}

	ValueType get_type()const {
		return jsonValue.type;
	}

	double get_number()const {
		assert(jsonValue.type == ValueType::NUMBER_TYPE);
		return JSON_VALUE;
	}



	Status parse() {
		jsonValue.type = ValueType::NULL_TYPE;
		parse_whitespace();
		auto ret = parse_value();
		if (ret == Status::PARSE_OK) {
			parse_whitespace();
			if (json.size() && !json.starts_with('\0')) {
				jsonValue.type = ValueType::NULL_TYPE;
				ret = Status::PARSE_ROOT_NOT_SINGULAR;
			}
		}
		return ret;
	}

	Status parse(const char* input) {
		json = input;
		return parse();
	}

private:
	/* value = null / false / true / number */
	Status parse_value() {
		if (json.empty())return Status::PARSE_EXPECT_VALUE;
		switch (json[0]) {
		case 't':return parse_literal("true", ValueType::TRUE_TYPE);
		case 'f':return parse_literal("false", ValueType::FALSE_TYPE);
		case 'n':return parse_literal("null", ValueType::NULL_TYPE);
		case '\0':return Status::PARSE_EXPECT_VALUE;
		default:return parse_number();
		}
	}

	/* ws = *(%x20 / %x09 / %x0A / %x0D) */
	void parse_whitespace() {
		while (json.starts_with(' ') || json.starts_with('\t') || json.starts_with('\n') || json.starts_with('\r')) {
			json.remove_prefix(1);
		}
	}

	/* literal = "null" / "false" / "true" */
	Status parse_literal(std::string_view literal, ValueType type) {
		if (json.size() < literal.size())return Status::PARSE_INVALID_VALUE;
		for (size_t i = 0; i < literal.size(); ++i) {
			if (json[i] != literal[i]) {
				return Status::PARSE_INVALID_VALUE;
			}
		}
		json.remove_prefix(literal.size());
		this->jsonValue.type = type;
		return Status::PARSE_OK;
	}

	/*
	 * number = [ "-" ] int [ frac ] [ exp ]
	 * int = "0" / digit1-9 *digit
	 * frac = "." 1*digit
	 * exp = ("e" / "E") ["-" / "+"] 1*digit
	 */
	Status parse_number() {
		std::string_view judge = json;
		if (judge.starts_with('-')) judge.remove_prefix(1);
		if (judge.starts_with('0')) judge.remove_prefix(1);
		else {
			if (judge.empty() || !isdigit(judge[0])) return Status::PARSE_INVALID_VALUE;
			for (judge.remove_prefix(1); judge.size() && isdigit(judge[0]); judge.remove_prefix(1));
		}
		if (judge.starts_with('.')) {
			judge.remove_prefix(1);
			if (judge.empty() || !isdigit(judge[0])) return Status::PARSE_INVALID_VALUE;
			for (judge.remove_prefix(1); judge.size() && isdigit(judge[0]); judge.remove_prefix(1));
		}
		if (judge.starts_with('e') || judge.starts_with('E')) {
			judge.remove_prefix(1);
			if (judge.starts_with('+') || judge.starts_with('-')) judge.remove_prefix(1);
			if (judge.empty() || !isdigit(judge[0])) return Status::PARSE_INVALID_VALUE;
			for (judge.remove_prefix(1); judge.size() && isdigit(judge[0]); judge.remove_prefix(1));
		}

		errno = 0;
		jsonValue.value = strtod(json.data(), nullptr);
		if (errno == ERANGE && (JSON_VALUE == HUGE_VAL || JSON_VALUE == -HUGE_VAL)) {
			return Status::PARSE_NUMBER_TOO_BIG;
		}
		json = judge;
		jsonValue.type = ValueType::NUMBER_TYPE;
		return Status::PARSE_OK;
	}
};



#endif/* _LEPTJSON_H_ */

