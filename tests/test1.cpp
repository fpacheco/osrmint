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
    datapoint_t* datapoints = NULL;
    datadt_t* result = NULL;

    /* Leer archivo */
    txtFile.open ("../data/data.txt");
    std::cout << std::fixed << std::setw( 11 ) << std::setprecision( 6 );
    if (txtFile.is_open())
    {
        // Leer los datos
        std::cout << "Leyendo archivo y cargando datos" << std::endl;
        ndatapoints = 0;
        while ( getline(txtFile,line) )
        {
            std::vector<std::string> elem = splitString(line, "|");
            // Reallocate one more
            datapoints = (datapoint_t*) realloc (datapoints, (ndatapoints+1)*sizeof(datapoint_t));
            if (datapoints) {
                (*(datapoints + ndatapoints)).id = atoi(elem[0].c_str());
                (*(datapoints + ndatapoints)).xf = atof(elem[3].c_str());
                (*(datapoints + ndatapoints)).yf = atof(elem[4].c_str());
                (*(datapoints + ndatapoints)).xt = atof(elem[5].c_str());
                (*(datapoints + ndatapoints)).yt = atof(elem[6].c_str());
                ndatapoints = ndatapoints + 1;
            } else {
                printf("realloc failed\n");
                free(datapoints);
                exit(1);
            }

        }
        txtFile.close();
        std::cout << "Se leyeron " << ndatapoints << " lineas" << std::endl;
        std::cout << datapoints[1].id << ": xt=" << datapoints[1].xt << ", yt=" << datapoints[1].yt << std::endl;
        std::cout << "Cerrando archivo" << std::endl;

    } else {
        std::cout << "No se puede abrir el archivo" << std::endl;
    }
    txtFile.close();
    /* */

    /* Llamar a servicio TCPIP */
    std::cout << "Llamando a localhost" << std::endl;
    OSRMCurlpp* router = new OSRMCurlpp();

    router->setBaseURL("http://localhost:5000/viaroute");
    //router->getRoute(datapoints,ndatapoints,result);
    router->getRoute(-56.18605, -34.90685, -56.096480, -34.849390);
    std::cout << "Total distance: " << router->getTotalDistance() << std::endl;
    std::cout << "Total time: " << router->getTotalTime() << std::endl;

    router->setBaseURL("http://localhost:9000/viaroute");
    //router->getRoute(datapoints,ndatapoints,result);
    router->getRoute(-56.18605, -34.90685, -56.096480, -34.849390);
    std::cout << "Total distance: " << router->getTotalDistance() << std::endl;
    std::cout << "Total time: " << router->getTotalTime() << std::endl;

    router->setBaseURL("http://localhost:5000/viaroute");

    std::clock_t c_start = std::clock();
    auto t_start = std::chrono::high_resolution_clock::now();

    router->getRoute(datapoints, ndatapoints, &result);

    std::clock_t c_end = std::clock();
    auto t_end = std::chrono::high_resolution_clock::now();
    std::cout << std::fixed << std::setprecision(2) << "CPU time used: "
              << 1000.0 * (c_end-c_start) / CLOCKS_PER_SEC << " ms\n"
              << "Wall clock time passed: "
              << std::chrono::duration<double, std::milli>(t_end-t_start).count()
              << " ms\n";

    for (int i = 0; i < ndatapoints; ++i) {
        std::cout << result[i].id << ": ("
                  << result[i].tdist << " m, "
                  << result[i].ttime << " s, "
                  << (result[i].tdist/1000)/(result[i].ttime/60/60) << "km/h"
                  << ")" << std::endl;
    }

    free(result);
    free(datapoints);
    return 0;
}
