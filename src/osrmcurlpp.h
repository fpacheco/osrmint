#ifndef OSRMRCURLPP_H
#define OSRMRCURLPP_H
// App types
#include "types.h"

// Basic
#include <iostream>
#include <string>
#include <vector>
#include <map>

// Curlpp
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Exception.hpp>

typedef std::vector<datapoint_t> vdatapoint;
typedef std::vector<datadt_t> vdatadt;

class OSRMCurlpp
{

public:
    ~OSRMCurlpp();
    void setBaseURL(std::string url) { mBaseURL = url; }
    void setBaseURL(char* url) { mBaseURL = std::string(url); }
    std::string baseURL() { return mBaseURL; }
    bool checkCon();

    void reqGeom(bool req) { mReqGeometry = req; }
    bool reqGeom() const { return mReqGeometry; }

    void reqIns(bool req) { mReqInstructions = req; }
    bool reqIns() const { return mReqInstructions; }

    void reqAlt(bool req) { mReqRouteAlt = req; }
    bool reqAlt() const { return mReqRouteAlt; }

    void reqComp(bool req) { mReqCompression = req; }
    bool reqComp() const { return mReqCompression; }

    void route(float fLon, float fLat, float tLon, float tLat);
    int getRoute(datapoint_t *datapoints, int ndatapoints, datadt_t **result);

    void viaRoute();
    int getViaRoute(dataviaroute_t *datapoints, int ndatapoints, dataroutejson_t **result);

    std::vector<dataroutegeom_t> routeGeometry() { return mRouteGeomerty; }
    std::vector<datarouteinst_t> routeInstructions() { return mRouteInstructions; }

    float totalDistance() const { return mTotalDistance; }
    float totalTime() const { return mTotalTime; }

    void parseOSRM(const char* resp);

private:
    // Private variables
    std::string mBaseURL = "http://localhost:5000/viaroute";
    int mTotalDistance = -1;
    int mTotalTime = -1;
    bool mReqGeometry = true;
    bool mReqInstructions = true;
    bool mReqCompression = false;
    bool mReqRouteAlt = false;
    std::vector<dataviaroute_t> mRoutePoints;
    std::string mRouteJSON="";
    std::vector<dataroutegeom_t> mRouteGeomerty;
    std::vector<datarouteinst_t> mRouteInstructions;

    // Private methods and functions

};

#endif // OSRMRCURLPP_H
