/// \file Materials.cc
/// \brief Implementation of the Materials class

#include "Materials.hh"

#include "G4SystemOfUnits.hh"

// forward declare here; don't put in header
namespace Materials {
	void configurePolystyreneScintillation();
    void makePolystyrene();
}

namespace {
    static const G4bool useSpline = true; // true: smooth emission spectrum curve / false: linear interpolation
}

namespace Materials
{
    void makeMaterials()
    {
		if (!G4Material::GetMaterial(kPOLYSTYRENE)) {
            makePolystyrene();
            configurePolystyreneScintillation();
        }

		/* 
        auto* nMan = G4NistManager::Instance();
        if (!nMan->FindOrBuildMaterial(kNIST_SIO2)->GetMaterialPropertiesTable())
            configureQuartz();
		*/
    }

    void makePolystyrene()
    {
        auto* nist = G4NistManager::Instance();
        G4Element* elH = nist->FindOrBuildElement("H");
        G4Element* elC = nist->FindOrBuildElement("C");
        auto* polystyrene = new G4Material(
            kPOLYSTYRENE ,
        	POLYSTYRENE_DENSITY,
            2.0,
            kStateSolid, 
            TEMP); 
        polystyrene->AddElement(elH, H_NATOMS);
        polystyrene->AddElement(elC, C_NATOMS);
    }

    void configurePolystyreneScintillation() 
    {
        // to change optical parameters
        // see https://geant4-userdoc.web.cern.ch/UsersGuides/ForApplicationDeveloper/html/TrackingAndPhysics/physicsProcess.html#id1
        auto* opp = G4OpticalParameters::Instance();
        if (!opp) {
            G4Exception(
                "src/Materials.cc configureScintillation()",
                "[no error code]",
                RunMustBeAborted,
                "Optical parameters table not instantiated");
        }

        // can be modified . . .
        //opp->SetScintFiniteRiseTime(false);

        auto* polystyrene = G4Material::GetMaterial(kPOLYSTYRENE);
        if (!polystyrene) {
            G4Exception(
                "src/Materials.cc configureScintillation()",
                "[no error code]",
                RunMustBeAborted,
                "Polystyrene not loaded before configuring scintillation");
        }

        auto* scintPt = new G4MaterialPropertiesTable;

        scintPt->AddConstProperty("SCINTILLATIONYIELD1", 1.0);
        scintPt->AddConstProperty(kRISE_TIME, POLYSTYRENE_RISE_TIME);

        scintPt->AddConstProperty(kSCINT_YIELD, POLYSTYRENE_SCINT_YIELD);
        scintPt->AddProperty(kREFR_IDX, POLYSTYRENE_REFR_IDX_ENERGIES, POLYSTYRENE_REFR_IDXS, useSpline);

        scintPt->AddProperty(
            kSCINT_COMPONENT, POLYSTYRENE_SCINT_OPTICAL_ENERGIES,
            POLYSTYRENE_SCINT_OPTICAL_INTENSITIES, useSpline);
        scintPt->AddConstProperty(kDECAY_TIME_CONSTANT, POLYSTYRENE_DECAY_TIME_CONSTANT);

        // # of photons emitted = RESOLUTION_SCALE * sqrt(mean # of photons)
        scintPt->AddConstProperty(kRESOLUTION_SCALE, POLYSTYRENE_SCINT_RESLN_SCALE);
        scintPt->AddProperty(kABSORPTION_LEN, POLYSTYRENE_ABS_LEN_ENERGIES, POLYSTYRENE_ABS_LEN, useSpline);

        polystyrene->SetMaterialPropertiesTable(scintPt);
    }
}