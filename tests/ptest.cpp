#include <stdio.h>      /* printf, NULL */
#include <stdlib.h>     /* strtof */
#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <iomanip>
#include <chrono>

// Leer archivos
#include <fstream>

// Ruteo
#include "osrmroute.h"

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
    FixedPointCoordinate sP = FixedPointCoordinate(-34906850,-56186050);
    FixedPointCoordinate eP = FixedPointCoordinate(-34849390,-56096480);
    //std::string osrmPath = "/tmp/montevideo/montevideo.osrm";
    char* osrmPath = "/home/fpacheco/workspace/idm/montevideo/montevideo.osrm";

    OSRMRoute* router = new OSRMRoute();
    router->init_data(osrmPath);
    // Route from FixedPointCoordinate
    router->route(sP,eP);
    std::cout << "Total distance: " << router->getTotalDistance() << std::endl;
    std::cout << "Total time: " << router->getTotalTime() << std::endl;

    // Route from lon, lat
    router->route(-56.18605, -34.90685, -56.096480, -34.849390);
    std::cout << "Total distance: " << router->getTotalDistance() << std::endl;
    std::cout << "Total time: " << router->getTotalTime() << std::endl;

    std::ifstream txtFile;
    std::string line;

    int ndatapoints;
    datapoint_t* datapoints = NULL;
    datadt_t* result = NULL;

    txtFile.open ("../data/data.txt");
    //txtFile.open ("../../data/data_10000.txt");
    std::cout << std::fixed << std::setw( 11 ) << std::setprecision( 6 );

    if (txtFile.is_open())
    {
        // Leer los datos
        std::cout << "Leyendo archivo y cargando datos" << std::endl;
        ndatapoints = 0;
        while ( getline(txtFile,line) )
        {
            std::vector<std::string> elem = splitString(line, "|");
            /*
            std::cout << "-----------------------------------------------\n";
            std::cout << "(" << atof(elem[3].c_str()) << "," << atof(elem[4].c_str()) << ")" << '\n';
            std::cout << "(" << atof(elem[5].c_str()) << "," << atof(elem[6].c_str()) << ")" << '\n';
            */

            //router->route(atof(elem[3].c_str()), atof(elem[4].c_str()), atof(elem[5].c_str()), atof(elem[6].c_str()));
            //std::cout << "Total distance: " << router->getTotalDistance() << std::endl;
            //std::cout << "Total time: " << router->getTotalTime() << std::endl;
            //datapoints = (datapoint_t *)malloc(1*sizeof(*datapoints));
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
    std::cout << "Llamando a route" << std::endl;

    std::clock_t c_start = std::clock();
    auto t_start = std::chrono::high_resolution_clock::now();

    router->route(datapoints, ndatapoints, &result);

    std::clock_t c_end = std::clock();
    auto t_end = std::chrono::high_resolution_clock::now();

    std::cout << std::fixed << std::setprecision(2) << "CPU time used: "
              << 1000.0 * (c_end-c_start) / CLOCKS_PER_SEC << " ms\n"
              << "Wall clock time passed: "
              << std::chrono::duration<double, std::milli>(t_end-t_start).count()
              << " ms\n";

    /*
    for (int i = 0; i < ndatapoints; ++i) {
        std::cout << result[i].id << ": Distancia(m)=" << result[i].tdist << ", Tiempo(s):" << result[i].ttime << std::endl;
    }
    */
    free(result);
    free(datapoints);
    return 0;
}
