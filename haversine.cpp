/*
 * Reads the haversine distances from JSON and computes the result
 * and those a comparison against the reference binary if needed
 */

#include "ce_json.h"

#include <string.h>

int main(int argc, char** args) {


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

    ceJSONParse(json, strlen(json));

	return 0;
}