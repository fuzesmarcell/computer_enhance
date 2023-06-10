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

bool ceJSONParse(const char* buffer, size_t len);