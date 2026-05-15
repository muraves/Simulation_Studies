
/// \file Materials.hh
/// \brief Definition of the Materials class

#ifndef Materials_h
#define Materials_h 1

#include "globals.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4Exception.hh"
#include "G4MaterialPropertiesTable.hh"
#include "G4String.hh"
#include "G4OpticalParameters.hh"
#include "G4PhysicalConstants.hh"
#include "G4Scintillation.hh"
#include "G4SystemOfUnits.hh"

#include "PolystyreneProperties.hh"

class G4Material;
namespace Materials
{
    static const G4double TEMP = 283 * kelvin;
    void makeMaterials();

    // scintillation
    static const G4String kSCINT_YIELD = "SCINTILLATIONYIELD";
    static const G4String kSCINT_COMPONENT = "SCINTILLATIONCOMPONENT1";
    static const G4String kDECAY_TIME_CONSTANT = "SCINTILLATIONTIMECONSTANT1";
    static const G4String kRESOLUTION_SCALE = "RESOLUTIONSCALE";
    static const G4String kRISE_TIME = "SCINTILLATIONRISETIME1";

    // optical photon stuff
    static const G4String kREFR_IDX = "RINDEX";
    static const G4String kREFR_IDX_REAL = "REALRINDEX";
    static const G4String kREFR_IDX_IMAG = "IMAGINARYRINDEX";
    static const G4String kABSORPTION_LEN = "ABSLENGTH";
    
    static const G4String kOP_DET_EFF = "EFFICIENCY";
    static const G4String kREFLECTIVITY = "REFLECTIVITY";
    static const G4String kTRANSMITTANCE = "TRANSMITTANCE";
};

#endif