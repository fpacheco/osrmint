#include "osrmroute.h"
#include <string>
/* boost */
#include <boost/filesystem/path.hpp>
//Rapidjson
#include <rapidjson/document.h>
//OpenMP
//#include <omp.h>
/* osrm-backend */
#include <osrm/ServerPaths.h>
#include <Util/ProgramOptions.h>
#include <Server/DataStructures/InternalDataFacade.h>
#include <plugins/viaroute.hpp>


/** @brief init_route
  *
  * @todo: document this function
  */
void OSRMRoute::init_data(std::string path_string)
{
    //fetch data from files
    ServerPaths paths;
    paths["base"] = path_string;
    boost::filesystem::path path = paths["base"];

    populate_base_path(paths);

    //construct router object. shared memory = SharedDataFacade, paths InternalDataFacade
    mQueryDataFacade = new InternalDataFacade<QueryEdge::EdgeData>(paths);

    std::cout << "Nodes: " << mQueryDataFacade->GetNumberOfNodes() << std::endl
              << "Edges: " << mQueryDataFacade->GetNumberOfEdges() << std::endl;

    mRouter = new ViaRoutePlugin<InternalDataFacade<QueryEdge::EdgeData>>(mQueryDataFacade);

    init_route();
}


OSRMRoute::~OSRMRoute()
{
    delete mQueryDataFacade;
    delete mRouter;
}


/** @brief init_route
  *
  * @todo: document this function
  */
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

/** @brief init_route
  *
  * @todo: document this function
  */
void OSRMRoute::route(FixedPointCoordinate startPoint, FixedPointCoordinate endPoint)
{
    mTotalDistance = -1;
    mTotalTime = -1;

    // Clear pevious data
    mRouteParameters.coordinates.clear();
    mRouteParameters.coordinates.push_back(startPoint);
    mRouteParameters.coordinates.push_back(endPoint);

    http::Reply reply;
    ViaRoutePlugin<InternalDataFacade<QueryEdge::EdgeData>> *router = new ViaRoutePlugin<InternalDataFacade<QueryEdge::EdgeData>>(mQueryDataFacade);
    router->HandleRequest(mRouteParameters, reply);
    //mRouter->HandleRequest(mRouteParameters, reply);

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
    delete router;
}

/** @brief init_route
  *
  * @todo: document this function
  */
void OSRMRoute::routewr(FixedPointCoordinate startPoint, FixedPointCoordinate endPoint, float *result)
{
    int tdist = -1;
    int ttime = -1;

    // Clear pevious data
    mRouteParameters.coordinates.clear();
    mRouteParameters.coordinates.push_back(startPoint);
    mRouteParameters.coordinates.push_back(endPoint);

    http::Reply reply;
    ViaRoutePlugin<InternalDataFacade<QueryEdge::EdgeData>> *router = new ViaRoutePlugin<InternalDataFacade<QueryEdge::EdgeData>>(mQueryDataFacade);
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

        if ( jsonDoc.Parse( str.c_str() ).HasParseError() ) {
            std::cout << "*** Error parsing ***" << std::endl;
        } else {
            if (jsonDoc["status"].GetInt()==0)
            {
                //std::cout << "status==0" << std::endl;
                tdist = jsonDoc["route_summary"]["total_distance"].GetDouble();
                ttime = jsonDoc["route_summary"]["total_time"].GetDouble();
            }
        }
    } else {
        std::cout << "NOT OK" << std::endl;
    }
    result[0] = tdist;
    result[1] = ttime;
    delete router;
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

/** @brief route
  *
  * @todo: document this function
  */
void OSRMRoute::routewr(float sLon, float sLat, float eLon, float eLat, float *result)
{
    routewr(
        FixedPointCoordinate( floatToIntWithMult(sLat, mMult), floatToIntWithMult(sLon, mMult) ),
        FixedPointCoordinate( floatToIntWithMult(eLat, mMult), floatToIntWithMult(eLon, mMult) ),
        result
    );
}


int OSRMRoute::route(datapoint_t *datapoints, int ndatapoints, datadt_t **result)
{
    try {
        *result = ( datadt_t * ) malloc( ndatapoints * sizeof( datadt_t ) );

        //#pragma omp parallel
        {            
            //#pragma omp for
            for ( int i = 0; i < ndatapoints; ++i ) {                
                float *rData = (float*) malloc(2*sizeof(double));
                rData[0] = -1.0;
                rData[1] = -1.0;
                //#pragma omp critical
                {
                    routewr(
                        FixedPointCoordinate(
                            floatToIntWithMult(datapoints[i].yf,mMult),
                            floatToIntWithMult(datapoints[i].xf,mMult)
                        ),
                        FixedPointCoordinate(
                            floatToIntWithMult(datapoints[i].yt,mMult),
                            floatToIntWithMult(datapoints[i].xt,mMult)
                        ),
                        rData
                    );
                }
                (*result+i)->id          = datapoints[i].id;
                (*result+i)->tdist       = rData[0]; //getTotalDistance();
                (*result+i)->ttime       = rData[1]; //getTotalTime();
                //(*result+i)->tdist       = 2.0;
                //(*result+i)->ttime       = 3.0;
            }
        }

    } catch ( std::exception &e ) {
        //*err_msg = strdup( e.what() );
        return -1;
    } catch ( ... ) {
        //*err_msg = strdup( "Caught unknown expection!" );
        return -1;
    }
    //*err_msg = (char *)0;
    return EXIT_SUCCESS;
}


vdatadt OSRMRoute::route(vdatapoint datapoints)
{
    vdatadt result;

    return result;
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
