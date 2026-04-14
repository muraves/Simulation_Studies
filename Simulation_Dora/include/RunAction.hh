/// \file RunAction.hh
/// \brief Definition of the RunAction class

#ifndef RunAction_h
#define RunAction_h 1

#include "G4UserRunAction.hh"
#include "PrimaryGeneratorInfo.hh"
#include <chrono>
#include "G4UImessenger.hh"
#include "G4UIcmdWithAString.hh"

class G4Run;

class EventAction;

class RunAction : public G4UserRunAction, public G4UImessenger
{
  public:
    RunAction(EventAction* eventAction, PrimaryGeneratorInfo* generatorInfo, long seed);
    ~RunAction() override = default;

    void BeginOfRunAction(const G4Run*) override;
    void EndOfRunAction(const G4Run*) override;
    void SetNewValue(G4UIcommand* cmd, G4String val) override;

  private:
    EventAction* fEventAction = nullptr;
    PrimaryGeneratorInfo* fGeneratorInfo; 
    std::string GetTimestamp();  
    std::chrono::high_resolution_clock::time_point startTime;
    std::chrono::high_resolution_clock::time_point endTime; 
    long fSeed;
    std::string fTimestamp;
    bool fIsHTCondor;
    std::string fClusterId;
    std::string fProcessId;
    std::string fNameFlag;

    //G4UImessenger*      fMessenger; // can be commented out since RunAction inhertics already from G4UIMessenger
    G4UIcmdWithAString* fDataPathCmd;
    G4String            fDataPath;
};


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
