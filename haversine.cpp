/*
Copyright (c) 2023, Fuzes Marcel
All rights reserved.

This source code is licensed under the BSD-style license found in the
LICENSE file in the root directory of this source tree.
*/

#include "ce_json.h"

#include "haversine_reference.h"

#include <string.h>
#include <assert.h>
#include <stdio.h>

#include <sys/stat.h>

struct HaversinePair {
	double x0, y0;
	double x1, y1;
};

static void test() {
	{
		const char* json = R"(
      {
        "Image": {
            "Width":  800,
            "Height": 600,
            "Title":  "View from 15th Floor",
            "Thumbnail": {
                "Url":    "http://www.example.com/image/481989943",
                "Height": 125,
                "Width":  100
            },
            "Animated" : false,
            "IDs": [116, 943, 234, 38793]
          }
      }
)";

		ceJSON* root = ceJSONParse(json, strlen(json));
		assert(root);

		ceJSON* image = ceJSONGetByKey(root, "Image");
		assert(image);
		if (image) {
			ceJSON* Animated = ceJSONGetByKey(image, "Animated");
			assert(Animated);
		}
	}

	{
		const char* json = R"(
      [
        {
           "precision": "zip",
           "Latitude":  37.7668,
           "Longitude": -122.3959,
           "Address":   "",
           "City":      "SAN FRANCISCO",
           "State":     "CA",
           "Zip":       "94107",
           "Country":   "US"
        },
        {
           "precision": "zip",
           "Latitude":  37.371991,
           "Longitude": -122.026020,
           "Address":   "",
           "City":      "SUNNYVALE",
           "State":     "CA",
           "Zip":       "94085",
           "Country":   "US"
        }
      ]

)";

		ceJSON* root = ceJSONParse(json, strlen(json));
		assert(root);

		double lat = 0.;
		for (ceJSON* node = root->first_child; node; node = node->next) {

			ceJSON* lat_json = ceJSONGetByKey(node, "Latitude");
			assert(lat_json);
			assert(lat_json && lat_json->kind == ceJSONKind::number);
			if (lat_json && lat_json->kind == ceJSONKind::number) {
				lat += lat_json->number;
			}
		}
	}
}

static bool load_entire_file(const char* file_name, void** buffer, size_t* buffer_size) {

	FILE* f = fopen(file_name, "rb");

#if _WIN32
	struct __stat64 stat;
	_stat64(file_name, &stat);
#else
	struct stat stat;
	stat(FileName, &stat);
#endif

	*buffer_size = stat.st_size;
	*buffer = malloc(stat.st_size);
	if (*buffer == nullptr) {
		return false;
	}

	if (fread(*buffer, stat.st_size, 1, f) != 1) {
		return false;
	}

	fclose(f);

	return true;
}

static bool parseAndAllocHaversineDistances(char* json, size_t json_len, HaversinePair** pairs, size_t* count) {
	ceJSON* root = ceJSONParse(json, json_len);
	ceJSON* j_pairs = ceJSONGetByKey(root, "pairs");

	if (!j_pairs) {
		return false;
	}

	*count = ceJSONLen(j_pairs);

	*pairs = (HaversinePair*)malloc((*count) * sizeof(HaversinePair));
	if (*pairs == nullptr) {
		return false;
	}

	HaversinePair* dst = *pairs;
	for (ceJSONIterator it = ceJSONIterBegin(j_pairs); ceJSONIterValid(&it); ceJSONIterNext(&it)) {

		ceJSON* node = it.node;

		ceJSON* j_x0 = ceJSONGetByKey(node, "x0");
		ceJSON* j_y0 = ceJSONGetByKey(node, "y0");
		ceJSON* j_x1 = ceJSONGetByKey(node, "x1");
		ceJSON* j_y1 = ceJSONGetByKey(node, "y1");

		// TODO: We kind of assume these are valid and we can pull a number from
		// them I guess...

		dst->x0 = j_x0->number;
		dst->y0 = j_y0->number;
		dst->x1 = j_x1->number;
		dst->y1 = j_y1->number;
		dst++;
	}

	return true;
}

static double sumHaversineDistances(HaversinePair* pairs, size_t pair_count) {

	double result = 0.;
	double sum_coef = 1 / (double)pair_count;

	for (size_t i = 0; i < pair_count; i++) {
		HaversinePair pair = pairs[i];
		double dist = referenceHaversine(pair.x0, pair.y0, pair.x1, pair.y1);
		result += sum_coef*dist;
	}

	return result;
}

static void validation(FILE* f, HaversinePair* pairs, size_t pair_count, double result) {

	int num_pairs;
	fread(&num_pairs, sizeof(int), 1, f);

	if (num_pairs != pair_count) {
		fprintf(stderr, "Number of pairs do not match: %d!=%llu\n", num_pairs, pair_count);
		return;
	}

	// For now this is not used we just read out the values in case we would like to compare the results
	for (size_t i = 0; i < num_pairs; i++) {
		double stub;
		fread(&stub, sizeof(double), 1, f);
	}

	double expected;
	fread(&expected, sizeof(double), 1, f);

	fprintf(stdout, "Reference sum: %.16f\n", expected);
	fprintf(stdout, "Difference: %.16f\n", result - expected);
}

int main(int argc, char** args) {

	// test();

	if (argc != 2 && argc != 3) {
		fprintf(stderr, "Usage: haversine [input.json]\n");
		fprintf(stderr, "Usage: haversine [input.json] [answers.double]\n");
		return EXIT_FAILURE;
	}

	char* json;
	size_t json_len;
	if (!load_entire_file(args[1], (void**)&json, &json_len)) {
		fprintf(stderr, "Unable to load json file\n");
		return EXIT_FAILURE;
	}

	HaversinePair* pairs;
	size_t pair_count;
	if (!parseAndAllocHaversineDistances(json, json_len, &pairs, &pair_count)) {
		fprintf(stderr, "Unable to parse or allocate haversine pairs\n");
		return EXIT_FAILURE;
	}

	double result = sumHaversineDistances(pairs, pair_count);

	fprintf(stdout, "Input size: %llu\n", json_len);
	fprintf(stdout, "Pair count: %llu\n", pair_count);
	fprintf(stdout, "Haversine sum: %.16f\n", result);

	if (argc == 3) {
		FILE* f = fopen(args[2], "rb");
		validation(f, pairs, pair_count, result);
		fclose(f);
	}

	return EXIT_SUCCESS;
}