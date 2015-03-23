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
    void setBaseURL(std::string url);
    void setBaseURL(char* url);
    std::string getBaseURL();
    void getRoute(float sLon, float sLat, float eLon, float eLat);
    int getDataRoute(datapoint_t *datapoints, int ndatapoints, datadt_t **result);
    float getTotalDistance();
    float getTotalTime();

private:
    // Private variables
    std::string mBaseURL = "http://localhost:5000/viaroute";
    float mTotalDistance = -1.0;
    float mTotalTime = -1.0;
    // Private methods and functions
    void parseDataRoute(const char* resp);

};

#endif // OSRMRCURLPP_H
