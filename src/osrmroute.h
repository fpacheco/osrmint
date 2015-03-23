#ifndef OSRMROUTE_H
#define OSRMROUTE_H

//
#include "types.h"

//Basic
#include <iostream>
#include <string>
#include <vector>

// OSRM
#include <osrm/Coordinate.h>
#include <osrm/RouteParameters.h>
#include <data_structures/query_edge.hpp>

typedef std::vector<datadt_t> vdatadt;
typedef std::vector<datapoint_t> vdatapoint;

// Forward declaration
template <class EdgeDataT> class InternalDataFacade;
template <class DataFacadeT> class ViaRoutePlugin;

class OSRMRoute
{

public:
    ~OSRMRoute();
    void init_data(std::string path_string);
    void init_data(char* path_string);
    void route(float sLon, float sLat, float eLon, float eLat);
    void route(FixedPointCoordinate startPoint, FixedPointCoordinate endPoint);
    int route(datapoint_t *datapoints, int ndatapoints, datadt_t **result);
    vdatadt route(vdatapoint datapoints);
    float getTotalDistance();
    float getTotalTime();

private:
    float mTotalDistance = -1.0;
    float mTotalTime = -1.0;
    int mMult = 1000000;

    RouteParameters mRouteParameters;
    InternalDataFacade<QueryEdge::EdgeData> *mQueryDataFacade;
    ViaRoutePlugin<InternalDataFacade<QueryEdge::EdgeData>> *mRouter;

    void init_route();
    int floatToIntWithMult(float f, int mult);
    void routewr(float sLon, float sLat, float eLon, float eLat, float *result);
    void routewr(FixedPointCoordinate startPoint, FixedPointCoordinate endPoint, float *result);

};

#endif // OSRMROUTE_H
