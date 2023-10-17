/*
 * This example shows how to project data from a Global Digital Elevation Model
 * (GDEM) onto a local map dumped to disk. It also provides a simple example
 * of error handler with clean memory management.
 *
 * Note that for this example to work you'll need the tile at (45N, 2E) from
 * a global model, e.g. N45E002.hgt for SRTMGL1. This tile is assumed to be
 * located in a folder named `share/topography`.
 */

/* C89 standard library */
#include <stdio.h>
#include <stdlib.h>
/* The TURTLE library */
#include "turtle.h"

static struct turtle_map * geoid = NULL;
/* A stack of elevation data, from a global model, e.g. SRTMGL1, ASTER-GDEM2, ...*/
static struct turtle_stack * stack = NULL;
/* A map that will contain the projected data */
static struct turtle_map * map = NULL;

/* Clean all allocated data and exit to the OS */
void exit_gracefully(enum turtle_return rc)
{
        turtle_map_destroy(&map);
        turtle_map_destroy(&geoid);
        turtle_stack_destroy(&stack);
        exit(rc);
}

/* Handler for TURTLE library errors */
void handle_error(enum turtle_return code, turtle_function_t * function,
    const char * message)
{
        fprintf(stderr, "A TURTLE library error occurred:\n%s\n", message);
        exit_gracefully(EXIT_FAILURE);
}

int main()
{
        /* Set a custom error handler */
        turtle_error_handler_set(&handle_error);

        /*Create a RGF93 local projection map, centered on the Auberge des Gros Manaux at Col de Ceyssat, Auvergne, France */
        const int NX = 1624;
        const int NY = 1508;
        struct turtle_map_info info = { .nx = 1624,
                .ny = 1508,
		.x = { 447584.687500, 455704.687500},
		.y = { 4515386.500000, 4522926.500000},
		.z = { 71., 1274.} };
        turtle_map_create(&map, &info, "UTM 33N");

        FILE *file_elevations;
        file_elevations= fopen("../share/topography/Vesuvio_modificatoXYZ_5m.txt","rb");
        double **elevations_dem;
        double xx, yy;
        elevations_dem=calloc(sizeof(double**), NY);
        for(int k=0; k< NY;k++) {
                elevations_dem[k] = calloc(sizeof(double*), NX);
                }
        for(int i = 0; i < NY; ++i)
	  {
	    for(int j = 0; j < NX; ++j) {
	      fscanf(file_elevations, "%lf %lf %lf", &xx, &yy, &elevations_dem[i][j]);
              }
	  }
        fclose(file_elevations);

        /* Fill the elevation */
        for(int i = 0; i < NY; ++i) 
        {
            for(int j = 0; j < NX; ++j) {
                double elevation;
                elevation = elevations_dem[i][j];
                turtle_map_fill(map, j, i, elevation);
                //printf("X: %f, Y:%f, Elevation: %f \n", xx, yy, elevation);
                }
        }
  
          /* Dump the projection map to disk */
        turtle_map_dump(map, "../share/data/map_Vesuvius.png");

        /* Load the EGM96 geoid map */
        turtle_map_load(&geoid, "../share/data/ww15mgh.grd");
        turtle_stack_create(&stack, "../share/SRTMGL1", 0, NULL, NULL);

        const struct turtle_projection *projob = turtle_map_projection(map);
        double latitude, longitude;
        /*============================================================
        ==================find the MURAVES elevation=================*/
        //location of MURAVES at the Vesuvio_modificatoXYZ_5m.txt file
        double xDem  =  450382;
        double yDem = 4517860;
        turtle_projection_unproject(projob, xDem, yDem, &latitude, &longitude);
        printf("Muraves: lat %f, long %f\n", latitude, longitude);
        /* Get the origine's elevation from the stack */
        double elevation_gdem;
        turtle_map_elevation(geoid, latitude, longitude, &elevation_gdem, NULL);
        printf("MURAVES' EGM %f\n",elevation_gdem);
        /* Get the same from the map */
        double elevation_map;
        turtle_map_elevation(map, xDem, yDem, &elevation_map, NULL);
        printf("MURAVES' INGV %f\n",elevation_map);
        /* Get the same from the SRTMGL1 */
        double elevation_SRTMGL1;
        turtle_stack_elevation(stack, latitude, longitude, &elevation_SRTMGL1, NULL);
        printf("MURAVES' SRTMGL1: %f\n",elevation_SRTMGL1);

        /* Clean and exit */
        exit_gracefully(TURTLE_RETURN_SUCCESS);
}
