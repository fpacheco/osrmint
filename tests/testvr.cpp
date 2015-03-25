#include <stdio.h>      /* printf, NULL */
#include <stdlib.h>     /* strtof */
#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <list>
#include <vector>
#include <iomanip>
#include <chrono>

#include <cstdlib>
#include <cerrno>

// Leer archivos
#include <fstream>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Exception.hpp>

#include "osrmcurlpp.h"

std::vector<std::string> splitString(std::string input, std::string delimiter)
{
    std::vector<std::string> output;
    char *pch;
    char *str = strdup(input.c_str());
    pch = strtok(str, delimiter.c_str());
    while (pch != NULL)
    {
        output.push_back(pch);
        pch = strtok (NULL,  delimiter.c_str());
    }
    free(str);
    return output;
}


int main()
{
    std::ifstream txtFile;
    std::string line;

    int ndatapoints;
    dataviaroute_t* datapoints = NULL;

    datapoints = (dataviaroute_t*) malloc (2*sizeof(dataviaroute_t));

    //loc=-34.90685,-56.18605&loc=-34.91285,-56.17314
    (*(datapoints + 0)).seq = 1;
    (*(datapoints + 0)).nodeid = 10;
    (*(datapoints + 0)).x = -56.18605;
    (*(datapoints + 0)).y = -34.90685;

    (*(datapoints + 1)).seq = 2;
    (*(datapoints + 1)).nodeid = 20;
    (*(datapoints + 1)).x = -56.17314;
    (*(datapoints + 1)).y = -34.91285;

    OSRMCurlpp* router = new OSRMCurlpp();
    router->setBaseURL("http://localhost:5000/viaroute");
    router->reqAlt(true);
    //router->reqComp(true);
    char* result;
    router->getViaRoute(datapoints, 2, &result);
    std::cout << "json: " << result << std::endl;

    free(result);
    free(datapoints);
    return 0;
}
