/// \file RunAction.hh
/// \brief Definition of the RunAction class

#ifndef RunAction_h
#define RunAction_h 1

#include "G4UserRunAction.hh"
#include "PrimaryGeneratorInfo.hh"
#include <chrono>

class G4Run;

class EventAction;

class RunAction : public G4UserRunAction
{
  public:
    RunAction(EventAction* eventAction, PrimaryGeneratorInfo* generatorInfo, long seed);
    ~RunAction() override = default;

    void BeginOfRunAction(const G4Run*) override;
    void EndOfRunAction(const G4Run*) override;

  private:
    EventAction* fEventAction = nullptr;
    PrimaryGeneratorInfo* fGeneratorInfo; 
    std::string GetTimestamp();  
    std::chrono::high_resolution_clock::time_point startTime;
    std::chrono::high_resolution_clock::time_point endTime; 
    long fSeed;
    std::string fTimestamp;
};


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
