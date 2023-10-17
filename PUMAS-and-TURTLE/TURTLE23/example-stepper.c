/*
 * This example illustrates how to step through elevation data with a
 * `turtle_stepper`, using Cartesian ECEF coordinates.
 *
 * Note that for this example to work you'll need the 4 tiles at (45N, 2E),
 * (45N, 3E), (46N, 2E), and (46N, 3E) from a global model, e.g. N45E002.hgt,
 * etc ... for SRTMGL1. These tiles are assumed to be located in a folder named
 * `share/topography`. In addition you'll need the local map created by
 * `example-projection` as well as a grid of the EGM96 geoid undulations
 * (`ww15mgh.grd`).
 */

/* C89 standard library */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
/* The TURTLE library */
#include "turtle.h"

/*
 * The turtle objects are declared globally. This allows us to define a simple
 * error handler with a gracefull exit to the OS. It also provides a simple
 * example of error handler with clean memory management.
 */
static struct turtle_map * geoid = NULL;
static struct turtle_map * map = NULL;
static struct turtle_stack * stack = NULL;
static struct turtle_stepper * stepper = NULL;

/* Clean all allocated data and exit to the OS. */
void exit_gracefully(enum turtle_return rc)
{
        turtle_stepper_destroy(&stepper);
        turtle_map_destroy(&geoid);
        turtle_map_destroy(&map);
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

int main(int argc, char * argv[])
{
        double approximation_range = 10., resolution_factor = 1E-02, slope_factor = 1.;
        
        /* Set a custom error handler */ 
        turtle_error_handler_set(&handle_error);

        /* Get the RGF93 local projection map*/
        turtle_map_load(&map, "../share/data/map_Vesuvius.png");

        /* Load the EGM96 geoid map */
        turtle_map_load(&geoid, "../share/data/ww15mgh.grd");

        /* Create the ECEF stepper and configure it */
        turtle_stepper_create(&stepper);
        turtle_stepper_geoid_set(stepper, geoid);
        turtle_stepper_slope_set(stepper, slope_factor);
        turtle_stepper_resolution_set(stepper, resolution_factor);
        turtle_stepper_range_set(stepper, approximation_range);
        turtle_stepper_add_flat(stepper, 0.);
        //turtle_stepper_add_stack(stepper, stack, 0.);
        turtle_stepper_add_map(stepper, map, 0.);

        /* Get the MURAVES position and direction in ECEF */
        const double latitude = 40.810251, longitude = 14.411708, height = 0;
        const double altitude_max = 1274.; //DEM maximum z is 1274.
        double azimuth=0., elevation = 0.;

        FILE *fileOutput;
        fileOutput = fopen("../share/data/Thickness_TURTLE_30elevation.txt","w");

        while (azimuth <= 360.){
            elevation = 0.;
            while (elevation <= 30.){
                printf("Azimuth: %f, Elevation: %f\n", azimuth, elevation);
                double position[3], direction[3];
                turtle_stepper_position(stepper, latitude, longitude, height, 0, position, NULL);
                turtle_ecef_from_horizontal(latitude, longitude, azimuth, elevation, direction);

                double altitude;
                int index[2];
                turtle_stepper_step(stepper, position, NULL, NULL, NULL, &altitude, NULL, NULL, index);
                //printf("First elevation: \n");
                //for (int i=0; i<2; i++) {printf("%f \n", elev[i]);}

                double rock_length = 0.;
                while (altitude < altitude_max) {
                    const int initial_layer = index[0];
                    
                    /* Do the next step */
                    double step;
                    turtle_stepper_step(stepper, position, direction, NULL,
                        NULL, &altitude, NULL, &step, index);
                    
                    /* Update the rock depth if the step started below the topography*/
                    if (initial_layer == 0) rock_length += step;
                }
            fprintf(fileOutput,"%f ",rock_length);
            elevation += 0.5;
            }
        printf("\n azimuth %f",elevation);
        fprintf(fileOutput,"\n");
        azimuth += 0.5;
        }
        
        exit_gracefully(TURTLE_RETURN_SUCCESS);
}
