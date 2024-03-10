﻿/*
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
#include <stdexcept>
#include <cmath>
#include <stack>

struct LeptJSON {

	enum class ValueType { NULL_TYPE, FALSE_TYPE, TRUE_TYPE, NUMBER_TYPE, STRING_TYPE, ARRAY_TYPE, OBJECT_TYPE };

	enum class Status {
		PARSE_OK, PARSE_EXPECT_VALUE, PARSE_INVALID_VALUE, PARSE_ROOT_NOT_SINGULAR, PARSE_NUMBER_TOO_BIG, PARSE_MISS_QUOTATION_MARK, PARSE_INVALID_STRING_ESCAPE, PARSE_INVALID_STRING_CHAR, PARSE_INVALID_UNICODE_HEX
	};

private:
	using jsonValueType = std::variant<std::nullptr_t, double, std::string, bool>;
	struct JsonValue {
		jsonValueType value;
		ValueType type;
	}jsonValue;

	struct Visitor {
		jsonValueType operator()(double arg) const {
			return arg;
		}

		jsonValueType operator()(std::string arg) const {
			return arg;
		}

		template<typename T>
		jsonValueType operator()(T) const {
			throw std::runtime_error("Wrong Variant Type");
		}
	};

	std::string_view json;

public:
	explicit LeptJSON(const char* js = "", ValueType vt = ValueType::NULL_TYPE)
		:jsonValue({}, vt), json(js) {}

	ValueType get_type()const {
		return jsonValue.type;
	}

	void set_value(const ValueType vt, const jsonValueType& v) {
		jsonValue.type = vt;
		jsonValue.value = v;
	}

	void set_nullptr() {
		set_value(ValueType::NULL_TYPE, nullptr);
	}

	bool get_boolean() const {
		assert(jsonValue.type == ValueType::TRUE_TYPE || jsonValue.type == ValueType::FALSE_TYPE);
		return std::get<bool>(jsonValue.value);
	}

	void set_boolean(bool b) {
		set_value(b ? ValueType::TRUE_TYPE : ValueType::FALSE_TYPE, b);
	}

	double get_number() const {
		assert(jsonValue.type == ValueType::NUMBER_TYPE);
		return std::get<double>(jsonValue.value);
	}

	void set_number(double number) {
		set_value(ValueType::NUMBER_TYPE, number);
	}

	std::string_view get_string()const {
		assert(jsonValue.type == ValueType::STRING_TYPE);
		return std::get<std::string>(jsonValue.value);
	}

	void set_string(std::string str) {
		set_value(ValueType::STRING_TYPE, str);
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

private:
	/* value = null / false / true / number */
	Status parse_value() {
		if (json.empty())return Status::PARSE_EXPECT_VALUE;
		switch (json[0]) {
			case 't':return parse_literal("true", ValueType::TRUE_TYPE);
			case 'f':return parse_literal("false", ValueType::FALSE_TYPE);
			case 'n':return parse_literal("null", ValueType::NULL_TYPE);
			case '\0':return Status::PARSE_EXPECT_VALUE;
			case '"':return parse_string();
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
		if (json.size() < literal.size()) {
			return Status::PARSE_INVALID_VALUE;
		}
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
		if (judge.starts_with('-')) {
			judge.remove_prefix(1);
		}
		if (judge.starts_with('0')) {
			judge.remove_prefix(1);
		}
		else {
			if (judge.empty() || !isdigit(judge[0])) {
				return Status::PARSE_INVALID_VALUE;
			}
			for (judge.remove_prefix(1); judge.size() && isdigit(judge[0]); judge.remove_prefix(1));
		}
		if (judge.starts_with('.')) {
			judge.remove_prefix(1);
			if (judge.empty() || !isdigit(judge[0])) {
				return Status::PARSE_INVALID_VALUE;
			}
			for (judge.remove_prefix(1); judge.size() && isdigit(judge[0]); judge.remove_prefix(1));
		}
		if (judge.starts_with('e') || judge.starts_with('E')) {
			judge.remove_prefix(1);
			if (judge.starts_with('+') || judge.starts_with('-')) {
				judge.remove_prefix(1);
			}
			if (judge.empty() || !isdigit(judge[0])) {
				return Status::PARSE_INVALID_VALUE;
			}
			for (judge.remove_prefix(1); judge.size() && isdigit(judge[0]); judge.remove_prefix(1));
		}

		errno = 0;
		jsonValue.value = strtod(json.data(), nullptr);
		if (errno == ERANGE && (std::get<double>(jsonValue.value) == HUGE_VAL || std::get<double>(jsonValue.value) == -HUGE_VAL)) {
			return Status::PARSE_NUMBER_TOO_BIG;
		}
		json = judge;
		jsonValue.type = ValueType::NUMBER_TYPE;
		return Status::PARSE_OK;
	}

	Status parse_string() {
		std::string s;
		if (json.starts_with('\"'))
			json.remove_prefix(1);
		for (; json.size(); json.remove_prefix(1)) {
			switch (json.front()) {
				case '\"':
					jsonValue = { s,ValueType::STRING_TYPE };
					json.remove_prefix(1);
					return Status::PARSE_OK;
				case '\\':
					json.remove_prefix(1);
					if (json.empty())return Status::PARSE_INVALID_STRING_ESCAPE;
					switch (json.front()) {
						default:return Status::PARSE_INVALID_STRING_ESCAPE;
						case'\"': s += '\"'; break;
						case'\\':s += '\\'; break;
						case'/':s += '/'; break;
						case'b':s += '\b'; break;
						case'f':s += '\f'; break;
						case'n':s += '\n'; break;
						case'r':s += '\r'; break;
						case't':s += '\t'; break;
					}
					break;
				default:
					if (static_cast<unsigned char>(json.front()) < 0x20) {
						return Status::PARSE_INVALID_STRING_CHAR;
					}
					s += json.front();
			}
		}
		return Status::PARSE_MISS_QUOTATION_MARK;
	}
};

#endif/* _LEPTJSON_H_ */
