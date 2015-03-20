#include "osrmroute.h"
#include <string>
/* boost */
#include <boost/filesystem/path.hpp>
//Rapidjson
#include <rapidjson/document.h>
/* osrm-backend */
#include <osrm/ServerPaths.h>
#include <Util/ProgramOptions.h>
#include <Server/DataStructures/InternalDataFacade.h>
#include <plugins/viaroute.hpp>

ViaRoutePlugin<InternalDataFacade<QueryEdge::EdgeData>> *router;

void OSRMRoute::init_data(std::string path_string)
{
    //fetch data from files
    ServerPaths paths;
    paths["base"] = path_string;
    boost::filesystem::path path = paths["base"];
    populate_base_path(paths);

    //construct router object
    InternalDataFacade<QueryEdge::EdgeData> *query_data_facade = new InternalDataFacade<QueryEdge::EdgeData>(paths);
    router = new ViaRoutePlugin<InternalDataFacade<QueryEdge::EdgeData>>(query_data_facade);

    init_route();
}

void OSRMRoute::init_data(char* path_string)
{
    std::string str(path_string);
    init_data(str);
}

/** @brief init_route
  *
  * @todo: document this function
  */
void OSRMRoute::init_route()
{
    mRouteParameters.zoom_level = 18;            // no generalization
    mRouteParameters.print_instructions = false; // turn by turn instructions
    mRouteParameters.alternate_route = false;    // get an alternate route, too
    mRouteParameters.geometry = false;           // retrieve geometry of route
    mRouteParameters.compression = false;        // polyline encoding
    mRouteParameters.check_sum = UINT_MAX;       // see wiki
    mRouteParameters.service = "viaroute";       // that's routing
    mRouteParameters.output_format = "json";
    mRouteParameters.jsonp_parameter = "";       // set for jsonp wrapping
    mRouteParameters.language = "";              // unused atm
}

//makes call to OSRM and returns route weights
void OSRMRoute::route(FixedPointCoordinate startPoint, FixedPointCoordinate endPoint)
{
    mTotalDistance = -1;
    mTotalTime = -1;

    // Clear pevious data
    mRouteParameters.coordinates.clear();
    mRouteParameters.coordinates.push_back(startPoint);
    mRouteParameters.coordinates.push_back(endPoint);

    http::Reply reply;
    router->HandleRequest(mRouteParameters, reply);

    if (reply.status==http::Reply::ok)
    {
        //std::cout << "ok" << std::endl;

        std::vector<char> replyContent = reply.content;

        /*
        std::cout << "replyContent contains:";
        for (std::vector<char>::iterator it = replyContent.begin() ; it != replyContent.end(); ++it)
          std::cout << *it;
        std::cout << '\n';
        */

        // Read the reply
        rapidjson::Document jsonDoc;
        std::string str( replyContent.begin(),replyContent.end() );

        if ( jsonDoc.Parse( str.c_str() ).HasParseError() )
        {
            std::cout << "*** Error parsing ***" << std::endl;
        }
        else
        {
            if (jsonDoc["status"].GetInt()==0)
            {
                //std::cout << "status==0" << std::endl;
                mTotalDistance = jsonDoc["route_summary"]["total_distance"].GetDouble();
                mTotalTime = jsonDoc["route_summary"]["total_time"].GetDouble();
            }
            else
            {
                //std::cout << "status!=0" << std::endl;
                mTotalDistance = -1;
                mTotalTime = -1;
            }
        }
        //jsonDoc.Parse( reinterpret_cast<char*>(reply.content.data()) );
    }
    else
    {
        std::cout << "NOT OK" << std::endl;
        mTotalDistance = -1;
        mTotalTime = -1;
    }
}

/** @brief float2Int
  *
  * @todo: document this function
  */
int OSRMRoute::floatToIntWithMult(float f, int mult)
{
    if (f>0)
    {
        return (int)(f*mult + 0.5);
    }
    else
    {
        return (int)(f*mult - 0.5);
    }
}


/** @brief route
  *
  * @todo: document this function
  */
void OSRMRoute::route(float sLon, float sLat, float eLon, float eLat)
{
    route(
        FixedPointCoordinate( floatToIntWithMult(sLat, mMult), floatToIntWithMult(sLon, mMult) ),
        FixedPointCoordinate( floatToIntWithMult(eLat, mMult), floatToIntWithMult(eLon, mMult) )
    );
}

/** @brief getTotalDistance
  *
  * @todo: document this function
  */
float OSRMRoute::getTotalDistance()
{
    return mTotalDistance;
}

/** @brief getTotalTime
  *
  * @todo: document this function
  */
float OSRMRoute::getTotalTime()
{
    return mTotalTime;
}
