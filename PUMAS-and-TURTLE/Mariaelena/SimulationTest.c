#include <errno.h>
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
/* The PUMAS API */
#include "pumas.h"


#ifndef M_PI
/* Define pi, if unknown */
#define M_PI 3.14159265358979323846
#endif
/* Altitude at which the primary flux is sampled */
#define PRIMARY_ALTITUDE 1E+03

/* A handle for the PUMAS simulation context */
static struct pumas_context * context = NULL;
double perc=0.25;
/* Gracefully exit to the OS */
static int exit_gracefully()
{
  pumas_context_destroy(&context);
  pumas_finalise();
}

/* Error handler for PUMAS with a graceful exit */
static void handle_error(enum pumas_return rc, pumas_function_t * caller, const char * message)
{
  /* Dump the error summary */
  fputs("pumas: library error. See details below\n", stderr);
  fprintf(stderr, "error: %s\n", message);
  /* Exit to the OS */
  exit_gracefully();
  exit(EXIT_SUCCESS);
}



static double locals_rock(struct pumas_medium * medium, struct pumas_state * state, struct pumas_locals * locals)
{
  /* Set the medium density */
  locals->density = 3.35E+03;

  /* Propose a maximum stepping distance. Returning zero or less indicates a uniform medium */
  return 0.;
}

static struct pumas_medium medium = {0, &locals_rock };
static double rock_thickness = 0.;
static double medium2(struct pumas_context * context,
		      struct pumas_state * state, struct pumas_medium ** medium_ptr)
{
  /* Check the muon position and direction */
  const double z = state->position[2]; //position is expressed in meters
  const double x = state->position[0];
  const double y = state->position[1];
  const double uz = state->direction[2]; // - cos(theta)
  double step = 1E+03;
  const double r = sqrt(x*x + y*y + z*z);
  if (z < 0. || r > rock_thickness) { 
    if (medium_ptr != NULL) *medium_ptr = NULL;
    step = -1.;
  }
  else {
    if (medium_ptr != NULL) *medium_ptr = &medium;
    if (uz > FLT_EPSILON)
      /* The muon is backward downgoing. The next boundary is
       * the rock bottom. */
      step = r;
    else if (uz < -FLT_EPSILON)
      /* The muon is backward upgoing. The next boundary is
       * the rock-air interface. */
      step = (rock_thickness - r);
  }
  return step; 
}

/* A basic Pseudo Random Number Generator (PRNG) providing a uniform
 * distribution over [0, 1]
 */
static double uniform01(struct pumas_context * context)
{
  return rand() / (double)RAND_MAX;
}

/* Gaisser's flux model, see e.g. the PDG */
static double flux_gaisser(double cos_theta, double Emu)
{
  const double ec = 1.1 * Emu * cos_theta;
  const double rpi = 1. + ec / 115.;
  const double rK = 1. + ec / 850.;
  return 1.4E+03 * pow(Emu, -2.7) * (1. / rpi + 0.054 / rK);
}

/* Volkova's parameterization of cos(theta*) */
static double cos_theta_star(double cos_theta)
{
  const double p[] = { 0.102573, -0.068287, 0.958633, 0.0407253,
		       0.817285 };
        const double cs2 =
	  (cos_theta * cos_theta + p[0] * p[0] + p[1] * pow(cos_theta, p[2]) +
	   p[3] * pow(cos_theta, p[4])) /
	  (1. + p[0] * p[0] + p[1] + p[3]);
        return cs2 > 0. ? sqrt(cs2) : 0.;
}

/*
 * Guan et al. parameterization of the sea level flux of atmospheric muons
 * Reference: https://arxiv.org/abs/1509.06176
 */
static double flux_gccly(double cos_theta, double kinetic_energy)
{
  const double Emu = kinetic_energy + 0.10566;
  const double cs = cos_theta_star(cos_theta);
  return pow(1. + 3.64 / (Emu * pow(cs, 1.29)), -2.7) *
    flux_gaisser(cs, Emu);
}

int main(int narg, char * argv[]) {


  FILE *fp;
  fp = fopen("Thicknesses_binnin033_completo.txt","rb");

  FILE *f_energy;
  f_energy = fopen("MinEnergy_detectorAndLead_033deg.txt","rb");

  FILE *f_flux;
   f_flux = fopen("fluxPumas_binning033_density_335_bisperc_binning150_210_10_27_Energy1GeV.txt","w"); 
  FILE *ferr; 
  ferr = fopen("fluxPumas_binning033_error_density_335bisperc_binning150_210_10_27_Energy1GeV.txt","w"); 
  //  double dati[90][360];
   const int Nphi = 1080;
   const int Ntheta = 270; 
   double dati[Nphi][Ntheta];
   double energy[Nphi][Ntheta];
  /* Making thickness matrix */

   for(int i=0; i<1080; i++) {
    for(int l=0; l<270; l++) {
     
      //    fscanf(f_energy, "%lf", &energy[l][i]);
      fscanf(fp, "%lf", &dati[i][l]);

    }
    }


  /* loop on (phi, elevation) */
   //     for(int i=361; i<511; i++) {
   // for(int l=31; l<82; l++) { 
   const int i = strtod(argv[2], NULL);
   const int l = strtod(argv[3], NULL);
   rock_thickness = dati[i][l]; 
      
       //       rock_thickness = 0.;

      
      const double elevation = (double) l*90./Ntheta;
      //const double kinetic_min = energy[i][l];
      const double kinetic_min = strtod(argv[1], NULL); 
      //      const double kinetic_max = 1000000; //10^6 GeV
      const double kinetic_max = kinetic_min; //10^6 GeV
      //      printf("Elevation: %f phi %d  ",elevation,i);
      // printf("rock_thickness: %f",rock_thickness);
 //fprintf(f_emin,"%.5lE ",kinetic_min);

 pumas_error_handler_set(&handle_error);

 const char * dump_file = "../pumas-materials/dump";
 FILE * fid = fopen(dump_file, "rb");
 if (fid == NULL) {
   perror(dump_file);
   exit_gracefully();
 
 }
 
        pumas_load(fid);
        fclose(fid);
        pumas_material_index("StandardRock", &medium.material);
        pumas_context_create(&context, 0);
        context->forward = 0;
        context->medium = &medium2;
        context->random = &uniform01;
        context->event |= PUMAS_EVENT_LIMIT_KINETIC;

        const double cos_theta = cos((90. - elevation) / 180. * M_PI);
        const double sin_theta = sqrt(1. - cos_theta * cos_theta);
        const double rk = log(kinetic_max / kinetic_min);
        double w = 0., w2 = 0.;
        const int n = 10000;
        int j;
	for (j = 0; j < n; j++) {


	  double kf, wf;
	  if (rk) {
	    kf = kinetic_min * exp(rk * uniform01(context));
	    wf = kf * rk;
	  } else {

	    kf = kinetic_min;
	    wf = 1;
	  }

	  struct pumas_state state = { .charge = -1., .kinetic = kf, .weight = wf, .direction = { -sin_theta, 0., -cos_theta } };
	  const double kinetic_threshold = kinetic_max * 1E+03;
	  while (state.kinetic < kinetic_threshold - FLT_EPSILON) {
	    if (state.kinetic < 1E+02 - FLT_EPSILON) {
	      context->scheme = PUMAS_SCHEME_DETAILED;
	      context->longitudinal = 0;
	      context->kinetic_limit = 1E+02;
	    } else {
	      context->scheme = PUMAS_SCHEME_HYBRID;
	      context->longitudinal = 1;
	      context->kinetic_limit = kinetic_threshold;
	    }
	    enum pumas_event event;
	    struct pumas_medium * medium[2];
	    pumas_transport(context, &state, &event, medium);
	    if (event == PUMAS_EVENT_MEDIUM) {
	      if (medium[1] == NULL) {

		const double wi = state.weight*flux_gccly(-state.direction[2],state.kinetic); 
		w += wi;
		w2 += wi * wi;
		break;
	      }
	    } else if (event != PUMAS_EVENT_LIMIT_KINETIC) {

	      fprintf(stderr,
		      "error: unexpected PUMAS event `%d`\n",
		      event);
	      exit_gracefully();
	    }
	  }
        }


        w /= n;

	//	double corr =( M_PI/180.)*(M_PI/180.)*(sin((90. - elevation) / 180. * M_PI)*sin((90. - elevation) / 180. * M_PI));
	double corr =1;
	w = w*corr;
        const double sigma =
	  (rock_thickness <= 0.) ? 0. : sqrt(((w2 / n) - w * w) / n);
        const char * unit = rk ? "" : "GeV^{-1} ";
	printf("%.5lE",w);
	//	printf("Flux : %.5lE \\pm %.5lE %sm^{-1} s^{-2} sr^{-1}\n", w, sigma,
	// unit);

//fprintf(f_flux,"%lf ",w);
//	printf("%lf ",rock_thickness);
//	fprintf(ferr,"%.5lE ",sigma);
	exit_gracefully();

//  }
//  fprintf(f_flux,"\n");
//  fprintf(ferr,"\n");
// }

exit(EXIT_SUCCESS);

   /* Exit to the OS */

}
