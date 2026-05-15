#ifndef MuSimPhysicsList_h
#define MuSimPhysicsList_h 1

#include "G4VModularPhysicsList.hh"
#include "globals.hh"

class PhysicsList : public G4VModularPhysicsList
{
  public:
    PhysicsList();
   ~PhysicsList();

    void ConstructProcess() override;
    void SetCuts()          override;
};

#endif