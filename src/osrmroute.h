#ifndef OSRMROUTE_H
#define OSRMROUTE_H

//Basic
#include <iostream>
#include <iostream>
// OSRM
#include <osrm/Coordinate.h>
#include <osrm/RouteParameters.h>

class OSRMRoute
{

public:
    void init_data(std::string path_string);
    void route(float sLon, float sLat, float eLon, float eLat);
    void route(FixedPointCoordinate startPoint, FixedPointCoordinate endPoint);
    float getTotalDistance();
    float getTotalTime();

private:
    float mTotalDistance = -1.0;
    float mTotalTime = -1.0;
    int mMult = 1000000;
    RouteParameters mRouteParameters;

    void init_route();
    int floatToIntWithMult(float f, int mult);

};

#endif // OSRMROUTE_H
