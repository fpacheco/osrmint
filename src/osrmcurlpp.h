#ifndef OSRMRCURLPP_H
#define OSRMRCURLPP_H

// App types
#include "types.h"
// Curlpp
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Exception.hpp>

//Basic
#include <iostream>
#include <string>
#include <vector>

typedef std::vector<datadt_t> vdatadt;
typedef std::vector<datapoint_t> vdatapoint;

class OSRMCurlpp
{

public:
    ~OSRMCurlpp();
    void setBaseURL(std::string path_string);
    std::string getBaseURL();
    void getRoute(float sLon, float sLat, float eLon, float eLat);
    int getDataRoute(datapoint_t *datapoints, int ndatapoints, datadt_t **result);
    float getTotalDistance();
    float getTotalTime();

private:
    std::string mBaseURL = "http://localhost:5000/viaroute";
    float mTotalDistance = -1.0;
    float mTotalTime = -1.0;

};

#endif // OSRMRCURLPP_H
