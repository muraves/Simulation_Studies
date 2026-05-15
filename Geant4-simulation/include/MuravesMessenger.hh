/// \file MuravesMessenger.hh
/// \brief Definition of the MuravesMessenger class

#ifndef MuravesMessenger_h
#define MuravesMessenger_h 1

#include "globals.hh"

//class G4GenericMessenger;

class MuravesMessenger {

public:
  MuravesMessenger();
  ~MuravesMessenger();

  static MuravesMessenger* Instance() { return fgInstance; };

  void SetPrimaryGenerator(G4String val) { primarygenerator = val; };
  const G4String& GetPrimaryGenerator() { return primarygenerator; };

private:
  static MuravesMessenger* fgInstance;
  G4String primarygenerator;
};

#endif