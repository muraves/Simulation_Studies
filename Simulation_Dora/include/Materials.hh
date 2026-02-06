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
/// \file optical/wls/include/CosMuSimMaterials.hh
/// \brief Definition of the CosMuSimMaterials class
//

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

#pragma once

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