#include <stdio.h>      /* printf, NULL */
#include <stdlib.h>     /* strtof */
#include <iostream>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <chrono>

#include <cstdlib>
#include <cerrno>

//Rapidjson
#include <rapidjson/document.h>

#include "osrmcurlpp.h"

bool OSRMCurlpp::checkCon() {
    try {
        // Curlpp declarations
        curlpp::Cleanup cleaner;
        curlpp::Easy request;
        std::stringstream response;
        // Write resonse to string
        request.setOpt( new curlpp::options::WriteStream( &response ) );
        std::ostringstream url;
        url << mBaseURL;
        request.setOpt( new curlpp::options::Url(url.str()) );
        request.perform();
        // No exception. URL (host and port) is valid.
        return true;
    } catch ( curlpp::LogicError & e ) {
        std::cout << "curlpp LogicError: " << e.what() << std::endl;
    } catch ( curlpp::RuntimeError & e ) {
        std::cout << "curlpp RuntimeError: " << e.what() << std::endl;
    }
    // Exception. URL (host and port) is invalid.
    return false;
}

void OSRMCurlpp::getRoute(float fLon, float fLat, float tLon, float tLat){
    try {
        // Curlpp declarations
        curlpp::Cleanup cleaner;
        curlpp::Easy request;
        std::stringstream response;
        std::list<std::string> header;
        //???
        //curlpp::options::HttpGet();
        // Headers
        header.push_back("Content-Type: application/octet-stream");
        request.setOpt(new curlpp::options::HttpHeader(header));
        // UserAgent
        request.setOpt(new curlpp::options::UserAgent("Ruteo de residuos solidos (IDM)"));
        // Verbose for debug
        // request.setOpt(new curlpp::options::Verbose(true));
        // Write resonse to string
        request.setOpt( new curlpp::options::WriteStream( &response ) );
        std::ostringstream url;
        url << mBaseURL << "?"
            << "loc=" << fLat << "," << fLon
            << "&loc=" << tLat << "," << tLon
            << "&alt=false" << "&geometry=false" << "&instructions=false";
        request.setOpt( new curlpp::options::Url(url.str()) );
        request.perform();
        //std::cout << "response: " << response.str() << std::endl;
        // Parse response in json
        parseOSRM( response.str().c_str() );
    } catch ( curlpp::LogicError & e ) {
        std::cout << "curlpp LogicError: " << e.what() << std::endl;
        mTotalDistance = -1;
        mTotalTime = -1;
    } catch ( curlpp::RuntimeError & e ) {
        std::cout << "curlpp RuntimeError: " << e.what() << std::endl;
        mTotalDistance = -1;
        mTotalTime = -1;
    }
}

int OSRMCurlpp::getRoute(datapoint_t *datapoints, int ndatapoints, datadt_t **result) {

    try {
        // Allocate memory for result
        *result = ( datadt_t * ) malloc( ndatapoints * sizeof( datadt_t ) );
        // Curlpp declarations
        curlpp::Cleanup cleaner;
        curlpp::Easy request;
        std::stringstream response;
        std::list<std::string> header;
        //???
        //curlpp::options::HttpGet();
        // Headers
        header.push_back("Content-Type: application/octet-stream");
        request.setOpt(new curlpp::options::HttpHeader(header));
        // UserAgent
        request.setOpt(new curlpp::options::UserAgent("Ruteo de residuos solidos (IDM)"));
        // Verbose for debug
        // request.setOpt(new curlpp::options::Verbose(true));
        // Write resonse to string
        request.setOpt( new curlpp::options::WriteStream( &response ) );

        // Iterate thru datapoints
        for (int i = 0; i < ndatapoints; ++i) {
            // Set URL
            std::ostringstream url;
            url << mBaseURL << "?"
                << "loc=" << datapoints[i].yf << "," << datapoints[i].xf
                << "&loc=" << datapoints[i].yt << "," << datapoints[i].xt
                << "&alt=false" << "&geometry=false" << "&instructions=false";
            request.setOpt( new curlpp::options::Url(url.str()) );
            //
            try {
                request.perform();
                //std::cout << "response: " << response.str() << std::endl;
                // Parse response in json
                parseOSRM( response.str().c_str() );
                // Set result data
                (*result+i)->id          = datapoints[i].id;
                (*result+i)->tdist       = mTotalDistance;
                (*result+i)->ttime       = mTotalTime;
            } catch ( curlpp::LogicError & e ) {
                std::cout << "curlpp LogicError: " << e.what() << std::endl;
            } catch ( curlpp::RuntimeError & e ) {
                std::cout << "curlpp RuntimeError: " << e.what() << std::endl;
            }
            // Vacio el reponse
            response.str(std::string());
        }
        return 0;
    } catch ( curlpp::LogicError & e ) {
      std::cout << e.what() << std::endl;
      return -1;
    } catch ( curlpp::RuntimeError & e ) {
      std::cout << e.what() << std::endl;
      return -2;
    }
}

void OSRMCurlpp::parseOSRM(const char* resp){

    mTotalDistance = -1;
    mTotalTime = -1;

    rapidjson::Document jsonDoc;
    //std::string str( replyContent.begin(),replyContent.end() );

    if ( jsonDoc.Parse( resp ).HasParseError() )
    {
        std::cout << "*** Error parsing ***" << std::endl;
    } else {
        if (jsonDoc["status"].GetInt()==0) {
            //std::cout << "status==0" << std::endl;
            if ( jsonDoc.HasMember("route_summary") ) {
                mTotalDistance = jsonDoc["route_summary"]["total_distance"].GetInt();
                mTotalTime = jsonDoc["route_summary"]["total_time"].GetInt();
            }
            if ( jsonDoc.HasMember("route_geometry") ) {
                // array [[-34.906479,-56.186089],[-34.906433,-56.185513] ...]
            }
            if ( jsonDoc.HasMember("route_instructions") ) {
                // array ["10","San José",142,0,22,"141m","E",85,1],["7","Doctor Javier Barrios Amorín",77,3,5,"76m","N",354,1] ...]
            }
        } else {
            std::cout << "Error: json status!=0 on OSRM" << std::endl;
        }
    }
}