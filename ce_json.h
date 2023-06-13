/*
Copyright (c) 2023, Fuzes Marcel
All rights reserved.

This source code is licensed under the BSD-style license found in the
LICENSE file in the root directory of this source tree.
*/

#include <string_view>

enum class ceJSONKind {
	null,
	boolean,
	object,
	array,
	number,
	string,
};

struct ceJSON {
	ceJSONKind kind;
	
	std::string_view key;

	std::string_view string;
	double number;
	bool boolean;

	ceJSON* first_child;
	ceJSON* next;
};

ceJSON* ceJSONParse(const char* buffer, size_t len);
ceJSON* ceJSONGetByKey(ceJSON* json, const char* buffer);

struct ceJSONIterator {
	ceJSON* json;
	ceJSON* node;
};

ceJSONIterator ceJSONIterBegin(ceJSON* obj);
bool ceJSONIterValid(ceJSONIterator* iter);
void ceJSONIterNext(ceJSONIterator* iter);

size_t ceJSONLen(ceJSON* json);