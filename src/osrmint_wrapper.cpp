#include "types.h"
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
    char *baseURL,
    datapoint_t *datapoints,
    int ndatapoints,
    datadt_t **result,
    int *result_count,
    char **err_msg
) {

    try {
        OSRMCurlpp* router = new OSRMCurlpp();
        router->setBaseURL(baseURL);
        router->getDataRoute(datapoints, ndatapoints, result);
        *result_count = ndatapoints;
   } catch ( std::exception &e ) {
        *err_msg = strdup( e.what() );
        return -1;
    } catch ( ... ) {
        *err_msg = strdup( "Caught unknown expection!" );
        return -1;
    }

    *err_msg = (char *)0;
    return EXIT_SUCCESS;
}

#ifdef __cplusplus
}
#endif
