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
#include <memory>
#include <stack>
#include <vector>
#include <map>

struct LeptJSON {

	enum class ValueType { NULL_TYPE, FALSE_TYPE, TRUE_TYPE, NUMBER_TYPE, STRING_TYPE, ARRAY_TYPE, OBJECT_TYPE };

	enum class Status {
		PARSE_OK, PARSE_EXPECT_VALUE, PARSE_INVALID_VALUE, PARSE_ROOT_NOT_SINGULAR, PARSE_NUMBER_TOO_BIG, PARSE_MISS_QUOTATION_MARK, PARSE_INVALID_STRING_ESCAPE, PARSE_INVALID_STRING_CHAR, PARSE_INVALID_UNICODE_HEX, PARSE_INVALID_UNICODE_SURROGATE, PARSE_MISS_COMMA_OR_SQUARE_BRACKET, PARSE_MISS_KEY, PARSE_MISS_COLON, PARSE_MISS_COMMA_OR_CURLY_BRACKET
	};

private:
	struct JsonValue;
	using json_array_type = std::vector<JsonValue>;
	using json_object_type = std::map<std::string, JsonValue>;
	using jsonValueType = std::variant<std::nullptr_t, double, std::string, bool, json_array_type, json_object_type>;
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

	void set_json(const char* js) {
		json = js;
	}

	void set_nullptr() {
		jsonValue = { nullptr,ValueType::NULL_TYPE };
	}

	bool get_boolean() const {
		assert(jsonValue.type == ValueType::TRUE_TYPE || jsonValue.type == ValueType::FALSE_TYPE);
		return std::get<bool>(jsonValue.value);
	}

	void set_boolean(bool b) {
		jsonValue = { b,b ? ValueType::TRUE_TYPE : ValueType::FALSE_TYPE };
	}

	double get_number() const {
		assert(jsonValue.type == ValueType::NUMBER_TYPE);
		return std::get<double>(jsonValue.value);
	}

	void set_number(double number) {
		jsonValue = { number,ValueType::NUMBER_TYPE };
	}

	std::string_view get_string()const {
		assert(jsonValue.type == ValueType::STRING_TYPE);
		return std::string_view{ std::get<std::string>(jsonValue.value).data(),std::get<std::string>(jsonValue.value).size() };
	}

	void set_string(const char* str) {
		jsonValue = { std::string{str},ValueType::STRING_TYPE };
	}

	auto&& get_array()const {
		assert(jsonValue.type == ValueType::ARRAY_TYPE);
		return std::get<json_array_type>(jsonValue.value);
	}

	auto&& get_object()const {
		assert(jsonValue.type == ValueType::OBJECT_TYPE);
		return std::get<json_object_type>(jsonValue.value);
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
			case '[':return parse_array();
			case '{':return parse_object();
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
			jsonValue.type = ValueType::NULL_TYPE;
			return Status::PARSE_INVALID_VALUE;
		}
		for (size_t i = 0; i < literal.size(); ++i) {
			if (json[i] != literal[i]) {
				jsonValue.type = ValueType::NULL_TYPE;
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
				jsonValue.type = ValueType::NULL_TYPE;
				return Status::PARSE_INVALID_VALUE;
			}
			for (judge.remove_prefix(1); judge.size() && isdigit(judge[0]); judge.remove_prefix(1));
		}
		if (judge.starts_with('.')) {
			judge.remove_prefix(1);
			if (judge.empty() || !isdigit(judge[0])) {
				jsonValue.type = ValueType::NULL_TYPE;
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
				jsonValue.type = ValueType::NULL_TYPE;
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
					jsonValue = { std::string{s.data()},ValueType::STRING_TYPE };//! why
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
						case'u':
							unsigned int u{};
							if (!parse_hex4(u)) {
								return Status::PARSE_INVALID_UNICODE_HEX;
							}
							if (u >= 0xD800 && u <= 0xDBFF) { /* surrogate pair */
								json.remove_prefix(5);
								if (!json.starts_with('\\')) {
									return Status::PARSE_INVALID_UNICODE_SURROGATE;
								}
								json.remove_prefix(1);
								if (!json.starts_with('u')) {
									return Status::PARSE_INVALID_UNICODE_SURROGATE;
								}
								unsigned int u2{};
								if (!parse_hex4(u2)) {
									return Status::PARSE_INVALID_UNICODE_HEX;
								}
								if (u2 < 0xDC00 || u2>0xDFFF) {
									return Status::PARSE_INVALID_UNICODE_SURROGATE;
								}
								u = (((u - 0xD800) << 10) | (u2 - 0xDC00)) + 0x10000;
							}
							json.remove_prefix(4);
							encode_utf8(s, u);
							break;
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

	bool parse_hex4(unsigned int& u) {
		if (json.size() < 5)return false;
		for (int i = 1; i < 5; i++) {
			u <<= 4;
			if (json[i] >= '0' && json[i] <= '9') u |= json[i] - '0';
			else if (json[i] >= 'A' && json[i] <= 'F') u |= json[i] - 'A' + 10;
			else if (json[i] >= 'a' && json[i] <= 'f') u |= json[i] - 'a' + 10;
			else return false;
		}
		return true;
	}

	void encode_utf8(std::string& s, const unsigned int& u) {
		if (u <= 0x7f) {
			s += u & 0xff;
		}
		else if (u <= 0x7ff) {
			s += 0xC0 | ((u >> 6) & 0xFF);
			s += 0x80 | (u & 0x3F);
		}
		else if (u <= 0xffff) {
			s += 0xE0 | ((u >> 12) & 0xFF);
			s += 0x80 | ((u >> 6) & 0x3F);
			s += 0x80 | (u & 0x3F);
		}
		else {
			assert(u <= 0x10ffff);
			s += 0xF0 | ((u >> 18) & 0xFF);
			s += 0x80 | ((u >> 12) & 0x3F);
			s += 0x80 | ((u >> 6) & 0x3F);
			s += 0x80 | (u & 0x3F);
		}
	}

	/* array = %x5B ws [ value *( ws %x2C ws value ) ] ws %x5D */
	Status parse_array() {
		if (json.starts_with('[')) {
			json.remove_prefix(1);
		}
		parse_whitespace();
		if (json.starts_with(']')) {
			json.remove_prefix(1);
			jsonValue = { json_array_type{} ,ValueType::ARRAY_TYPE };
			return Status::PARSE_OK;
		}
		json_array_type v = json_array_type{};
		while (json.size()) {
			auto ret = parse_value();
			if (ret != Status::PARSE_OK) {
				return ret;
			}
			v.push_back(jsonValue);
			parse_whitespace();
			if (json.starts_with(',')) {
				json.remove_prefix(1);
				parse_whitespace();
			}
			else if (json.starts_with(']')) {
				json.remove_prefix(1);
				jsonValue = { std::move(v),ValueType::ARRAY_TYPE };
				return Status::PARSE_OK;
			}
			else {
				jsonValue.type = ValueType::NULL_TYPE;
				return Status::PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
			}
		}
		jsonValue.type = ValueType::NULL_TYPE;
		return Status::PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
	}

	Status parse_object() {
		if (json.starts_with('{')) {
			json.remove_prefix(1);
		}
		parse_whitespace();
		if (json.starts_with('}')) {
			json.remove_prefix(1);
			jsonValue = { json_object_type{},ValueType::OBJECT_TYPE };
			return Status::PARSE_OK;
		}
		json_object_type v = json_object_type{};
		while (true) {
			if (!json.starts_with('\"')) {
				jsonValue.type = ValueType::NULL_TYPE;
				return Status::PARSE_MISS_KEY;
			}
			auto ret = parse_string();
			if (ret != Status::PARSE_OK) {
				return ret;
			}
			std::string key = std::get<std::string>(jsonValue.value);
			parse_whitespace();
			if (!json.starts_with(':')) {
				jsonValue.type = ValueType::NULL_TYPE;
				return Status::PARSE_MISS_COLON;
			}
			json.remove_prefix(1);
			parse_whitespace();
			ret = parse_value();
			if (ret != Status::PARSE_OK) {
				return ret;
			}
			v[key] = jsonValue;
			parse_whitespace();
			if (json.starts_with(',')) {
				json.remove_prefix(1);
				parse_whitespace();
			}
			else if (json.starts_with('}')) {
				json.remove_prefix(1);
				jsonValue = { std::move(v),ValueType::OBJECT_TYPE };
				return Status::PARSE_OK;
			}
			else {
				jsonValue.type = ValueType::NULL_TYPE;
				return Status::PARSE_MISS_COMMA_OR_CURLY_BRACKET;
			}
		}
		jsonValue.type = ValueType::NULL_TYPE;
		return Status::PARSE_MISS_COMMA_OR_CURLY_BRACKET;
	}

	friend ValueType get_type(const JsonValue& jv) { return jv.type; }

	friend bool get_boolean(const JsonValue& jv) {
		assert(jv.type == ValueType::TRUE_TYPE || jv.type == ValueType::FALSE_TYPE);
		return std::get<bool>(jv.value);
	}

	friend double get_number(const JsonValue& jv) {
		assert(jv.type == ValueType::NUMBER_TYPE);
		return std::get<double>(jv.value);
	}

	friend std::string_view get_string(const JsonValue& jv) {
		assert(jv.type == ValueType::STRING_TYPE);
		return std::string_view{ std::get<std::string>(jv.value).data(),std::get<std::string>(jv.value).size() };
	}

	friend auto&& get_array(const JsonValue& jv) {
		assert(jv.type == ValueType::ARRAY_TYPE);
		return std::get<json_array_type>(jv.value);
	}

	friend auto&& get_object(const JsonValue& jv) {
		assert(jv.type == ValueType::OBJECT_TYPE);
		return std::get<json_object_type>(jv.value);
	}
};



#endif/* _LEPTJSON_H_ */
