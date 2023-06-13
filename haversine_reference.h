/*
Copyright (c) 2023, Fuzes Marcel
All rights reserved.

This source code is licensed under the BSD-style license found in the
LICENSE file in the root directory of this source tree.
*/

#pragma once

#include <math.h>

static double square(double a) { return a*a; }
static double radiansFromDegrees(double degrees) { return 0.01745329251994329577*degrees; }

static double referenceHaversine(double x0, double y0, double x1, double y1, double earth_radius = 6372.8) {
	double lat1 = y0;
	double lat2 = y1;
	double lon1 = x0;
	double lon2 = x1;

	double d_lat = radiansFromDegrees(lat2 - lat1);
	double d_lon = radiansFromDegrees(lon2 - lon1);
	lat1 = radiansFromDegrees(lat1);
	lat2 = radiansFromDegrees(lat2);

	double a = square(sin(d_lat/2.)) + cos(lat1)*cos(lat2)*square(sin(d_lon/2.));
	double c = 2.*asin(sqrt(a));

	return earth_radius*c;
}