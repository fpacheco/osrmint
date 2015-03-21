#include "types.h"
#include "osrmroute.h"

#include <stdlib.h>
#include <stdio.h>
#include <exception>
#include <string.h>

#include <omp.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief C wrapper for C++
  *
  * @todo: document this function
  */
int c_wrapper_route(
    char *osrm_data_path,
    datapoint_t *datapoints,
    int ndatapoints,
    datadt_t **result,
    int *result_count,
    char **err_msg
) {

    try {
        OSRMRoute* router = new OSRMRoute();
        router->init_data(osrm_data_path);
        //User* u1 = (User*)malloc(sizeof(User));
        //results = ( vehicle_path_t * ) malloc( count * sizeof( vehicle_path_t ) );
        *result = ( datadt_t * ) malloc( ndatapoints * sizeof( datadt_t ) );

        #pragma omp parallel
        {
            #pragma omp for
            for ( int i = 0; i < ndatapoints; ++i ) {
                router->route(datapoints[i].xf,datapoints[i].yf,datapoints[i].xt,datapoints[i].yt);
                (*result+i)->id          = datapoints[i].id;
                (*result+i)->tdist       = router->getTotalDistance();
                (*result+i)->ttime       = router->getTotalTime();
            }
        }
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
