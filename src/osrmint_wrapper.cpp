#include "types.h"
#include "osrmroute.h"

#ifdef __cplusplus
extern "C" {
#endif

int c_wrapper_route(
    char *osrm_data_path,
    datapoint_t *datapoints,
    int ndatapoints,
    datadt_t **result,
    int *result_count,
    char **err_msg_out
    ) {
        OSRMRoute* router = new OSRMRoute();
        router->init_data(osrm_data_path);

        return 0;
    }

#ifdef __cplusplus
}
#endif
