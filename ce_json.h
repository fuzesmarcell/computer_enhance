/*
 * JSON parser
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