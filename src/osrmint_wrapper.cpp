#include "osrmcurlpp.h"

#include <stdlib.h>
#include <stdio.h>
#include <exception>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief C wrapper for C++
  *
  * @todo: document this function
  */
int c_wrapper_route(
    datapoint_t *datapoints,
    char *baseURL,
    int ndatapoints,
    datadt_t **result,
    int *result_count,
    char **err_msg
) {

    try {
        OSRMCurlpp* router = new OSRMCurlpp();
        router->setBaseURL(baseURL);
        if (router->checkCon()) {
            if (router->getRoute(datapoints, ndatapoints, result)>=0) {
                *result_count = ndatapoints;
            } else {
                *result_count = 0;
                *err_msg = strdup( "Something was wrong!" );
                return -10;
            }

        } else {
            *result_count = 0;
            *err_msg = strdup( "Wrong port or IP un URL!" );
            return -20;
        }
    } catch ( std::exception &e ) {
        *err_msg = strdup( e.what() );
        return -30;
    } catch ( ... ) {
        *err_msg = strdup( "Caught unknown expection!" );
        return -40;
    }

    *err_msg = (char *)0;
    return EXIT_SUCCESS;
}

int c_wrapper_viaroute(
    dataviaroute_t *datapoints,
    char *baseURL,
    int ndatapoints,
    dataroutejson_t **result,
    int *result_count,
    char **err_msg
) {

    try {
        OSRMCurlpp* router = new OSRMCurlpp();
        router->setBaseURL(baseURL);
        if (router->checkCon()) {
            if (router->getViaRoute(datapoints, ndatapoints, result)>=0) {
                *result_count = 1;
            } else {
                *result_count = 0;
                *err_msg = strdup( "Something was wrong!" );
                return -10;
            }
        } else {
            *result_count = 0;
            *err_msg = strdup( "Wrong port or IP un URL!" );
            return -20;
        }
    } catch ( std::exception &e ) {
        *err_msg = strdup( e.what() );
        return -30;
    } catch ( ... ) {
        *err_msg = strdup( "Caught unknown expection!" );
        return -40;
    }

    *err_msg = (char *)0;
    return EXIT_SUCCESS;
}


#ifdef __cplusplus
}
#endif
