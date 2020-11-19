#include <crsRead/MCorsikaReader.h>

#include <crs/TSubBlock.h>
#include <crs/MRunHeader.h>
#include <crs/MEventHeader.h>
#include <crs/MEventEnd.h>
#include <crs/MParticleBlock.h>
#include <crs/MLongitudinalBlock.h>
#include <crs/MParticle.h>

#include <TFile.h>
#include <TH1D.h>
#include <TNtupleD.h>
#include <TGraph.h>

#include <iostream>
#include <sstream>
#include <map>
using namespace std;


// to hold data of one observation level
struct ObsLevel {
  TNtupleD* His_mouns1;
  double x;
  double y;
  double x2;
  double y2;
  double w;
  double Px;
  double Py;

  
};
          



int
main (int argc, char **argv) 
{  
  if (argc<2) {
    cout << "  please specify Corsika file " << endl;
    return 1;
  }
    
  string fname(argv[1]);
  crsRead::MCorsikaReader cr(fname, 3);
  
  string inputFile;
  if (fname.rfind('/')==string::npos) {
    inputFile = fname;
  } else {
    inputFile = fname.substr(fname.rfind('/')+1, 
                             fname.size()-fname.rfind('/'));
  }
  
  
  int ShowerCounter = 0;
  
  crs::MRunHeader Run;
  while (cr.GetRun (Run)) {
    
    crs::MEventHeader Shower;
    while (cr.GetShower(Shower)) {
      
      ++ShowerCounter;
      
      ostringstream oFileName;
      oFileName << inputFile.c_str() << "_"
                << Shower.GetEventNumber () << ".root";

      cout << " Writing summary to output file: " << oFileName.str() << endl;

      TFile oFile (oFileName.str ().c_str (), "RECREATE");
      
      const int nObsLevel = Shower.GetNObservationLevels();
      map<int, ObsLevel> obsLevel;
      
      for (int iObsLevel=1; iObsLevel<=nObsLevel; ++iObsLevel) { 
        
        double height = Shower.GetObservationHeight(iObsLevel-1);            
        cout << " init obs-level " << iObsLevel << " at h=" << height << endl;
        ObsLevel emptyLevel;
        ostringstream tTitle, tName;
        tTitle << "Data at level " << iObsLevel;
        tName << "His_mouns1_" << iObsLevel;
        emptyLevel.His_mouns1 = new TNtupleD(tName.str().c_str(), tTitle.str().c_str(), "id:e:x:y:Px:Py:Pz:Elevation_Angle");
        /*
          emptyLevel.xEl  = 0;
          emptyLevel.yEl  = 0;
          emptyLevel.wEl  = 0;
          emptyLevel.x2El = 0;
          emptyLevel.y2El = 0;
        */
        obsLevel[iObsLevel] = emptyLevel;

      } // end loop observation levels
      
      
      crs::TSubBlock Data;
      while (cr.GetData (Data)) {
        
        switch (Data.GetBlockType ()) {
          
            case crs::TSubBlock::ePARTDATA:
            {
              const crs::MParticleBlock& ParticleData = Data;
              crs::MParticleBlock::ParticleListConstIterator iEntry;
              for (iEntry = ParticleData.FirstParticle();
                   iEntry != ParticleData.LastParticle();
                   ++iEntry) {

		// DUMP
		//iEntry->Dump();
                
                if (iEntry->IsParticle()) {
                  
                  crs::MParticle iPart(*iEntry);
                  
                  const int id    = iPart.GetParticleID();
                  const int level = iPart.GetObservationLevel();
                  const double w  = iPart.GetWeight();
                  const double e  = iPart.GetKinEnergy();
                  const double x  = iPart.GetX();
                  const double y  = iPart.GetY();
                  const double Px  = iPart.GetPx();
                  const double Py  = iPart.GetPy();
                  const double Pz  = iPart.GetPz();
                
		  if (obsLevel.count(level)==0) {
		    cout << " detected new obs-level " << level 
			 << ". Possibly INCLIN-Option " << endl;
		    ObsLevel emptyLevel;
		    ostringstream tTitle, tName;
		    tTitle << "Data at level " << level;
		    tName << "His_mouns1_" << level;
		    obsLevel[level] = emptyLevel;
		    obsLevel[level].His_mouns1 = new TNtupleD(tName.str().c_str(), 
							tTitle.str().c_str(),
							"id:e:x:y:w:Px:Py:Elevation_Angle");
		  }
		        if ((id==5) || (id==6)) {
					
					if (iPart.GetX()<(15000))
						{
							
					
			 cout << "    particle id: " << id
                  << " x=" << x
                  << " y=" << y
                  << "\n"; 
							if (iPart.GetX()>(-15000)){
							
								if (iPart.GetY()<(15000)){
									if (iPart.GetY()>(-15000)){
										obsLevel[level].His_mouns1->Fill(id, e, x, y, Px, Py, Pz, 57.29578*atan(Pz/sqrt(Px*Px+Py*Py)));
										}
									}
			  }
		  }
                  obsLevel[level].x  += x*w;
                  obsLevel[level].y  += y*w;
                  obsLevel[level].x2 += x*x*w;
                  obsLevel[level].y2 += y*y*w;
                  obsLevel[level].w  += w;



                  //cout << "    particle id: " << id
                  //<< " x=" << x
                  //<< " y=" << y
                  //<< " Px=" << Px
                  //<< " Py=" << Py
                  //<< " Pz=" << Pz
                  //<< " Pr=" << sqrt(Px*Px + Py*Py)
                  //<< " Pz/Pr=" << Pz/sqrt(Px*Px + Py*Py)
                  //<< " Angle=" << 57.29578*atan(Pz/sqrt(Px*Px+Py*Py)) 
                  //<< "\n"; 
                                  

			  }
                }
                
              } // end particle loop

              
              break;
            }
            
            case crs::TSubBlock::eLONG:
              break;
              
            default:
              break;
        } // end data block
        
      } // loop data
      
      crs::MEventEnd ShowerSummary;
      cr.GetShowerSummary(ShowerSummary);
      const double Xmax = ShowerSummary.GetXmax();
      
      cout << "---------------------------------\n"
           << " Shower info:\n"
           << "  Xmax = " << Xmax << "\n";
      
      oFile.cd();
      for (map<int, ObsLevel>::iterator iLevel = obsLevel.begin();
           iLevel != obsLevel.end();
           ++iLevel) {
        
        cout << "   observation level: " << iLevel->first 
	     << " with " << iLevel->second.His_mouns1->GetEntries() << " particles "
	     << iLevel->second.His_mouns1->GetName()
	     << "\n";
        
	iLevel->second.His_mouns1->Write();
        
        /*
          cout << "    particle id: " << iPId->first
          << " <x>=" << iPId->second.x/iPId->second.w
          << " <y>=" << iPId->second.y/iPId->second.w 
          << " <x_rms>=" << sqrt(iPId->second.x2/iPId->second.w-pow(iPId->second.x/iPId->second.w,2))
          << " <y_rms>=" << sqrt(iPId->second.y2/iPId->second.w-pow(iPId->second.y/iPId->second.w,2))
          << "\n";
        */
        
      } // loop observation levels
      
      oFile.Close();
      
    } // loop shower
    
  } // loop runs (usually just 1)    
  
  cout << " Read " << ShowerCounter << " showers from file " << endl;
  
  return 0;
}


  
