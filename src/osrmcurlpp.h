#ifndef OSRMRCURLPP_H
#define OSRMRCURLPP_H
// App types
#include "types.h"
// Basic
#include <iostream>
#include <string>
#include <vector>
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
    std::string getBaseURL() { return mBaseURL; }
    float getTotalDistance() { return mTotalDistance; }
    float getTotalTime() { return mTotalTime; }
    void getRoute(float fLon, float fLat, float tLon, float tLat);
    int getRoute(datapoint_t *datapoints, int ndatapoints, datadt_t **result);
    void parseOSRM(const char* resp);
    bool checkCon();

private:
    // Private variables
    std::string mBaseURL = "http://localhost:5000/viaroute";
    int mTotalDistance = -1;
    int mTotalTime = -1;
    bool mGeometry = true;
    bool mInstructions = true;
    bool mCompression = false;
    bool mRouteAlt = false;
    // Private methods and functions

};

#endif // OSRMRCURLPP_H
