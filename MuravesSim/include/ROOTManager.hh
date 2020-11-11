#ifndef ROOTManager_h
#define ROOTManager_h 1

#include "globals.hh"
#include "EventAction.hh"
#include "RunAction.hh"
//#include "SteppingAction.hh"

#include <TFile.h>
#include <TTree.h>

class ROOTManager
{

  static const int MaxNGenPart = 500;
  static const int MaxNScintHit = 5000;
  
  struct ROOTTreeStruct_t {
    Int_t Event;
    
    Int_t NGenPart;
    Int_t GenPartID[MaxNGenPart];
    Int_t GenPartPDG[MaxNGenPart];
    Float_t GenPartE[MaxNGenPart];
    Float_t GenPartTheta[MaxNGenPart];
    Float_t GenPartPhi[MaxNGenPart];

    Int_t NScintHit;
    Int_t ScintHitPrimaryID[MaxNScintHit];
    Float_t ScintHitE[MaxNScintHit];
    Float_t ScintHitPosX[MaxNScintHit];
    Float_t ScintHitPosY[MaxNScintHit];
    Float_t ScintHitPosZ[MaxNScintHit];
    Int_t ScintHitStation[MaxNScintHit];
    Int_t ScintHitModule[MaxNScintHit];
    Int_t ScintHitBar[MaxNScintHit];

  };

public:
  
  ROOTManager();
  ~ROOTManager();
  
  static ROOTManager* Instance();   
    
  void Init();		
  void Save();
  void Fill();

  struct ROOTTreeStruct_t ROOTTreeStruct;
  
private:

  static ROOTManager* fgInstance;    
  TFile* ROOTFile;        
  TTree* ROOTTree;
};

#endif

