#include "osrmcurlpp.h"

void OSRMCurlpp::setBaseURL(std::string path_string){
    mBaseURL = path_string;
}

std::string OSRMCurlpp::getBaseURL(){
    return mBaseURL;
}

void OSRMCurlpp::getRoute(float sLon, float sLat, float eLon, float eLat){

}

int OSRMCurlpp::getDataRoute(datapoint_t *datapoints, int ndatapoints, datadt_t **result){

    try{

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
        // Verbose
        request.setOpt(new curlpp::options::Verbose(true));
        // Write resonse to string
        request.setOpt( new curlpp::options::WriteStream( &response ) );

        std::clock_t c_start = std::clock();
        auto t_start = std::chrono::high_resolution_clock::now();

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
                std::cout << "response: " << response.str() << std::endl;
                //response.str().c_str();
            } catch ( curlpp::LogicError & e ) {
                std::cout << e.what() << std::endl;
            } catch ( curlpp::RuntimeError & e ) {
                std::cout << e.what() << std::endl;
            }

            //request.perform();
            //std::cout << "response: " << response.str() << std::endl;
            //response.str().c_str();
            // Lo borro con
            response.str(std::string());
        }

        std::clock_t c_end = std::clock();
        auto t_end = std::chrono::high_resolution_clock::now();

        std::cout << std::fixed << std::setprecision(2) << "CPU time used: "
                  << 1000.0 * (c_end-c_start) / CLOCKS_PER_SEC << " ms\n"
                  << "Wall clock time passed: "
                  << std::chrono::duration<double, std::milli>(t_end-t_start).count()
                  << " ms\n";

    } catch ( curlpp::LogicError & e ) {
      std::cout << e.what() << std::endl;
    } catch ( curlpp::RuntimeError & e ) {
      std::cout << e.what() << std::endl;
    }

    free(result);
    free(datapoints);
}

float OSRMCurlpp::getTotalDistance(){
    return mTotalDistance;
}

float OSRMCurlpp::getTotalTime(){
    return mTotalTime;
}
