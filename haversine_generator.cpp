/*
 * Listing 65
 * JSON Haversine Generator
 * Two modes single or cluster
 */

#include "haversine_reference.h"

#include <stdint.h>
#include <limits>
#include <stdio.h>
#include <algorithm>
#include <assert.h>
#include <format>

struct Xorshift32State {
	uint32_t a;
};

/* The state must be initialized to non-zero */
static uint32_t xorshift32(Xorshift32State* state) {
	/* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
	uint32_t x = state->a;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	return state->a = x;
}

static double rand_uniform(Xorshift32State* state) {
	return (double)xorshift32(state) / std::numeric_limits<uint32_t>::max();
}

static double rand_range(Xorshift32State* state, double min, double max) {
	double t = rand_uniform(state);
	return t*min + (1 - t)*max;
}

enum class GeneratorKind {
	uniform,
	cluster,
};

static const char* generatorKindStrTable[] = {
	"uniform",
	"cluster",
};

int main(int argc, char** args) {

	GeneratorKind gen = argc > 1 ? (strcmp("uniform", args[1]) == 0 ? GeneratorKind::uniform : GeneratorKind::cluster) : GeneratorKind::uniform;
	uint32_t seed = argc > 2 ? atoi(args[2]) : 1234;
	int num_coordinates = argc > 3 ? atoi(args[3]) : 100;

	Xorshift32State _state = { seed };
	Xorshift32State* state = &_state;

	fprintf(stdout, "Method: %s\n", generatorKindStrTable[(int)gen]);
	fprintf(stdout, "Random seed: %d\n", seed);
	fprintf(stdout, "Pair count: %d\n", num_coordinates);

	std::string json_file_name = std::format("data_{}_flex.json", num_coordinates);
	FILE* json_file = fopen(json_file_name.c_str(), "w");

	std::string bin_file_name = std::format("data_{}_haveranswer.double", num_coordinates);
	FILE* bin_file = fopen(bin_file_name.c_str(), "w");

	fwrite(&num_coordinates, sizeof(num_coordinates), 1, bin_file);

	const char* json_start = R"({"pairs": [ )";
	fprintf(json_file, json_start);

	double result = 0.;
	double result_coef = 1. / num_coordinates;

	if (gen == GeneratorKind::uniform) {

		int c = 0;
		for (int i = 0; i < num_coordinates; i++) {
			double x0 = rand_range(state, -180., 180.);
			double y0 = rand_range(state, -180., 180.);
			double x1 = rand_range(state, -180., 180.);
			double y1 = rand_range(state, -180., 180.);
			double h = referenceHaversine(x0, y0, x1, y1);
			result += h*result_coef;

			fprintf(json_file, R"({"x0": %f, "y0": %f, "x1": %f, "y1": %f})", x0, y0, x1, y1);
			fwrite(&h, sizeof(h), 1, bin_file);

			c++;

			if (i < (num_coordinates-1)) {
				fprintf(json_file, ",\n");
			}
		}
		fclose(bin_file);
		int stophere = 5;
	}
	else {

		int num_clusters = 32;
		int coordinates_per_cluster = num_coordinates / num_clusters;

		int coordinates_left = num_coordinates;
		for (int i = 0; i < num_clusters; i++) {

			double x = rand_range(state, -180., 180.);
			double y = rand_range(state, -180., 180.);
			double r = 30.;

			int n = coordinates_per_cluster;
			if (i == (num_clusters - 1)) {
				n = coordinates_left;
			}

			coordinates_left -= n;
			for (int j = 0; j < n; j++) {

				double x0 = x + rand_range(state, -r, r);
				double y0 = y + rand_range(state, -r, r);
				double x1 = x + rand_range(state, -r, r);
				double y1 = y + rand_range(state, -r, r);
				double h = referenceHaversine(x0, y0, x1, y1);
				result += h*result_coef;

				fprintf(json_file, R"({"x0": %f, "y0": %f, "x1": %f, "y1": %f})", x0, y0, x1, y1);
				fwrite(&h, sizeof(h), 1, bin_file);

				bool last_cluster = i == (num_clusters-1);
				bool last_loop = j == (n-1);
				if (!last_cluster || !last_loop) {
					fprintf(json_file, ",\n");
				}
			}
		}

		assert(coordinates_left == 0);
	}

	fprintf(json_file, "]\n}");
	fclose(json_file);

	double expected_result = result;

	fwrite(&expected_result, sizeof(double), 1, bin_file);
	fclose(bin_file);

	fprintf(stdout, "Expected sum: %f\n", expected_result);

	return 0;
}