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
/// \file ActionInitialization.cc
/// \brief Implementation of the ActionInitialization class

#include "ActionInitialization.hh"
#include "PrimaryGeneratorAction_CRY.hh"
#include "PrimaryGeneratorAction_PartGun.hh"
#include "PrimaryGeneratorAction_GenPartSrc.hh"
#include "RunAction.hh"
#include "EventAction.hh"
#include "TrackingAction.hh"
#include "MuravesMessenger.hh"

ActionInitialization::ActionInitialization()
 : G4VUserActionInitialization()
{}


ActionInitialization::~ActionInitialization()
{}


void ActionInitialization::BuildForMaster() const
{
  EventAction* theEventAction = new EventAction;
  SetUserAction(new RunAction(theEventAction));
}


void ActionInitialization::Build() const
{
  auto generator = MuravesMessenger::Instance()->GetPrimaryGenerator();
  if ( generator == "CRY" )
    SetUserAction(new PrimaryGeneratorAction_CRY(""));
  else if ( generator == "PartGun" )
    SetUserAction(new PrimaryGeneratorAction_PartGun());
  else if ( generator == "GPS" )
    SetUserAction(new PrimaryGeneratorAction_GenPartSrc());

  auto theEventAction = new EventAction;
  SetUserAction(theEventAction);

  SetUserAction(new RunAction(theEventAction));

  SetUserAction(new TrackingAction());
}  

