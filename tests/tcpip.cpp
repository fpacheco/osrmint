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

#include "types.h"

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

    try{
        curlpp::Cleanup cleaner;
        curlpp::Easy request;

        curlpp::options::HttpGet();

        //request.setOpt(new curlpp::options::Url(url));
        //request.setOpt(new curlpp::options::Port(5000));
        request.setOpt(new curlpp::options::UserAgent("Ruteo de residuos solidos (IDM)"));

        //request.setOpt(new curlpp::options::Verbose(true));

        std::list<std::string> header;
        header.push_back("Content-Type: application/octet-stream");
        request.setOpt(new curlpp::options::HttpHeader(header));

        //request.setOpt(new curlpp::options::PostFields("abcd"));
        //request.setOpt(new curlpp::options::PostFieldSize(5));

        std::clock_t c_start = std::clock();
        auto t_start = std::chrono::high_resolution_clock::now();

        std::stringstream response;
        for (int i = 0; i < ndatapoints; ++i) {
            std::ostringstream url;
            url << "http://localhost:9000/viaroute" << "?"
                << "loc=" << datapoints[i].yf << "," << datapoints[i].xf
                << "&loc=" << datapoints[i].yt << "," << datapoints[i].xt
                << "&alt=false" << "&geometry=false" << "&instructions=false";

            request.setOpt( new curlpp::options::Url(url.str()) );
            request.setOpt( new curlpp::options::WriteStream( &response ) );
            request.perform();
            std::cout << "response: " << response.str() << std::endl;
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
    return 0;
}
