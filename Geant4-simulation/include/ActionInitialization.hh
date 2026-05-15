/// \file ActionInitialization.hh
/// \brief Definition of the ActionInitialization class

#ifndef ActionInitialization_h
#define ActionInitialization_h 1

#include "G4VUserActionInitialization.hh"
#include <string> 

class ActionInitialization : public G4VUserActionInitialization
{
  public:
    ActionInitialization(long seed, const std::string& ecomugFile = "");
    virtual ~ActionInitialization();

    virtual void BuildForMaster() const;
    virtual void Build() const;

  private:
    long fSeed;
    std::string fEcoMugFile;
};

#endif
