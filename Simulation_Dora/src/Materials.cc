//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
//
/// \file optical/wls/src/CosMuSimMaterials.cc
/// \brief Implementation of the CosMuSimMaterials class
//
//
#include "Materials.hh"

#include "G4SystemOfUnits.hh"

// forward declare here; don't put in header
namespace Materials {
	void configurePolystyreneScintillation();
    void makePolystyrene();
}

namespace {
    static const G4bool useSpline = true; // true: smooth emission spectrum curve
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