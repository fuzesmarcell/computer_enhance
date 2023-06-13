/*
Copyright (c) 2023, Fuzes Marcel
All rights reserved.

This source code is licensed under the BSD-style license found in the
LICENSE file in the root directory of this source tree.
*/

#include "ce_json.h"

#include <utility>
#include <assert.h>
#include <stdlib.h>

struct Parser {
	const char* at;
	const char* end;
};

static bool parseNode(Parser* p, ceJSON* json);

static bool isWhiteSpace(char c) {
	return (c == ' '  ||
			c == '\n' ||
			c == '\t' ||
			c == '\r');
}

static std::pair<bool, char> nextChar(Parser* p) {

	char c;
	for (;;) {
		c = p->at++[0];
		if (!isWhiteSpace(c)) break;
	}

	if (p->at >= p->end)
		return { false, 0 };

	return { true, c };
}

static std::tuple<bool, const char*, const char*> nextRange(Parser* p, int len) {

	const char* start = p->at;
	const char* end = p->at + len;

	if (end >= p->end) {
		return { false, nullptr, nullptr };
	}

	p->at = end;

	return { true, start, end };

}

static std::pair<bool, std::string_view> parseString(Parser* p, ceJSON* json) {

	const char* start = p->at;
	for (;;) {
		auto [s, c] = nextChar(p);
		if (!s) return { false, {} };
		if (c == '"') break;
	}

	std::string_view string(start, p->at - 1);

	if (json != nullptr) {
		json->kind = ceJSONKind::string;
		json->string = string;
	}

	return { true, string };
}

static bool parseObject(Parser* p, ceJSON* json, bool hasKey) {

	json->kind = hasKey ? ceJSONKind::object : ceJSONKind::array;

	char end_char = hasKey ? '}' : '[';

	ceJSON* prev = nullptr;
	for (;;) {
		auto [s, c] = nextChar(p);
		if (!s) return false;

		auto next_json = (ceJSON*)malloc(sizeof(*json));
		memset(next_json, 0, sizeof(*next_json));

		if (json->first_child == nullptr)
			json->first_child = next_json;

		if (prev != nullptr)
			prev->next = next_json;

		if (hasKey) {
			if (c != '"') return false;
			auto [succ, str] = parseString(p, nullptr);
			if (!succ) return false;

			next_json->key = str;

			std::tie(s, c) = nextChar(p);
			if (!s) return false;

			if (c != ':') return false;
		}
		else {
			// HACK: Move pointer back one.
			// TODO: Define a more precise meaning where the `at` ptr should
			// be point to. Is it the next char or is it the current one ?
			// Should a number parser not expect to be on the first number it receives ?
			p->at--;
		}
		
		parseNode(p, next_json);

		std::tie(s, c) = nextChar(p);
		if (!s) return false;

		if (c != ',') {
			if (c != end_char) {
				return false;
			}
			else {
				break;
			}
		}

		prev = next_json;
	}

	return true;
}

static bool parseLiteral(Parser* p, ceJSON* json, char begin_char) {

	const char* literal = nullptr;
	int length = 0;

	if (begin_char == 't') {
		literal = "true";
		length = 4;
	}
	else if (begin_char == 'f') {
		literal = "false";
		length = 5;
	}
	else if (begin_char == 'n') {
		literal = "null";
		length = 4;
	}

	if (literal == nullptr)
		return false;

	auto [s, start, end] = nextRange(p, length-1);
	if (!s) return false;

	int i = 1;
	for (const char* c = start; c < end; c++, i++) {
		if (literal[i] != c[0]) {
			return false;
		}
	}

	if (json != nullptr) {
		if (begin_char == 'n') {
			json->kind = ceJSONKind::null;
		}
		else {
			json->kind = ceJSONKind::boolean;
			json->boolean = begin_char == 'f' ? false : true;
		}
	}

	return true;
}

static bool parseNumber(Parser* p, ceJSON* json) {

	p->at--;
	double value = strtod(p->at, (char**)&p->at);

	if (json != nullptr) {
		json->kind = ceJSONKind::number;
		json->number = value;
	}

	return true;
}

bool parseNode(Parser* p, ceJSON* json) {

	auto [s, c] = nextChar(p);
	if (!s) return false;

	switch (c) {

	case '{':
		parseObject(p, json, true);
		break;

	case '[':
		parseObject(p, json, false);
		break;

	case '"':
		parseString(p, json);
		break;

	case '+':
	case '-':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		parseNumber(p, json);
		break;

	case 'f':
	case 'n':
	case 't':
		parseLiteral(p, json, c);
		break;

	default:
		return false;
		break;
	}

	return true;
}

ceJSON* ceJSONParse(const char* buffer, size_t len) {

	Parser p = {
		.at = buffer,
		.end = buffer + len,
	};


	ceJSON* root = (ceJSON*)malloc(sizeof(ceJSON));
	if (root == nullptr) {
		return nullptr;
	}

	memset(root, 0, sizeof(*root));

	parseNode(&p, root);

	return root;
}

ceJSON* ceJSONGetByKey(ceJSON* json, const char* buffer) {

	if (json->kind != ceJSONKind::object) {
		return nullptr; /* must be a object otherwise we can not iterate over the keys inside it */
	}

	for (ceJSON* node = json->first_child; node; node = node->next) {
		if (node->key.empty()) // TODO: Is this even possible ?
			continue;

		if (node->key == buffer) {
			return node;
		}
	}

	return nullptr;
}

ceJSONIterator ceJSONIterBegin(ceJSON* obj) {

	ceJSONIterator result;
	result.node = nullptr;
	result.json = nullptr;

	if (obj->kind == ceJSONKind::object || obj->kind == ceJSONKind::array) {
		result.json = obj;
		result.node = obj->first_child;
	}

	return result;
}

bool ceJSONIterValid(ceJSONIterator* iter) {
	return iter->node != nullptr;
}

void ceJSONIterNext(ceJSONIterator* iter) {
	iter->node = iter->node->next;
}

size_t ceJSONLen(ceJSON* json) {
	size_t result = 0;

	if (json->kind != ceJSONKind::object && json->kind != ceJSONKind::array) {
		return result;
	}

	for (ceJSON* node = json->first_child; node; node = node->next) {
		result++;
	}

	return result;
}