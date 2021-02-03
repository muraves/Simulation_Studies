//########################################################################################
//===============================> simTrackFit.C <========================================
//########################################################################################
// A routine that performs the 4-points track fitting in the same way as
// the reconstruction software. 
//
//*************************************************************************************
// It follows the following steps:
// #1. The cluster alignment test (i.e., check the middle two points are aligned
//     with the two outer points withint ~2 x stripWidth) and if the test condition
//     is satisfied, set boolean isTrk to true
//
// #2. A fit is performed using ROOT TF1 class and fit parameters are determined
//
// #3. Using fit parameters, the residuals in each stations are computed
//
// #4. The value of isTrk, fit parameters and residuals are returned using fitResults
//     struct
//****************************************************************************************
// Usage example (#include this file in your main macro):
// 
// Float_t nthModulePositions[4] = [-0.26,0.,0.262,1.494];    //these positions are fixed
// Float_t clusterPos[4]; //(or hit positions) for each stations outputted by MuravesSim
// fitResults myFitted_results = TrackFit_4p(nthModulePositions, clusterPos);
// 
// To access outputs:
// bool for valid track          --> myFitted_results.isTrk()
// Float_t for fit intercept     --> myFitted_results.fitParams[0]
// Float_t for fit slope         --> myFitted_results.fitParams[1]
// Float_t for fit chisq         --> myFitted_results.fitParams[2]
// Float_t for residuals in stn1 --> myFitted_results.residuals[0]
// Float_t for residuals in stn1 --> myFitted_results.residuals[1]
// Float_t for residuals in stn1 --> myFitted_results.residuals[2]
// Float_t for residuals in stn1 --> myFitted_results.residuals[3]
//
//****************************************************************************************
// To be added:
// #1. A flag for debugging
// #2. Make the routine fail-safe
// #3. A different fitting function (similar to TrackFit_4p) for 3 points fitting
//                                                                  
// -S.B. (CP3, Muography, Jan. 2021)
//
// (Note:This routine is adapted from MURAVES reconstruction software track-fitting.)
//########################################################################################

//=========================================================================
//  struct "fitResults" to record results after the fitting is performed
//=========================================================================
struct fitResults {
  Bool_t isTrack;
  Float_t fitParams[3];
  Float_t residuals[4];
};

//=========================================================================
// fitting function for 4 points data that outputs "fitResults", i.e.,
// a boolean for valid tracks and an array with fit results and residuals
//=========================================================================
fitResults TrackFit_4p(Float_t pointX[4], Float_t pointY[4]){


  Bool_t isTrk = false;
  Float_t strp_wdth = 0.033;  //required if we were to calcuate cluster position uncertainty
  Float_t limit = 0.05;       //limit to check if the middle two points are aligned two the outer ones with ~2 x stripWidth     
  Float_t fitIntercept=0., fitSlope=0., chiSq=0.;
  Float_t resP1=0., resP2=0., resP3=0., resP4=0.;
  //uncertainties in nth module position and cluster position both set to 10mm here
  Float_t errX[4] = {0.01,0.01,0.01,0.01};        //0.085
  Float_t errY[4] = {0.01,0.01,0.01,0.01};        //0.065  <-- these values best reproduce NERO chisq dists 

  //===============================
  //  cluster alignment test
  //===============================
  Float_t slope = (pointY[3]-pointY[0])/(pointX[3]-pointX[0]);
  Float_t intercept = pointY[0] - (pointX[0]*slope);
  Float_t y2Exp = (slope*pointX[1]) + intercept;
  Float_t y3Exp = (slope*pointX[2]) + intercept;

  /*==== some checks for debugging ========
  cout << "Slope     :  " <<  slope << endl; 
  cout << "intercept :  " <<  intercept << endl;
  cout << "y2_pred   :  " <<  y2Exp << endl;
  cout << "y3_pred   :  " <<  y3Exp << endl;
  cout << "y2 diff   :  " << abs(y2Exp - pointY[1]) << endl;
  cout << "y3 diff   :  " << abs(y3Exp - pointY[2]) << endl;
  */

  if(abs(y2Exp - pointY[1]) < limit && abs(y3Exp - pointY[2]) < limit){
    
    //set valid track true
    isTrk = true;
    
    // setting up the fitting function
    TF1 *linFit_func = new TF1("linFit_func","pol1",-1.,2.);
    TGraphErrors *linGrph = new TGraphErrors(4,pointX,pointY,errX,errY); 
    linGrph->Fit("linFit_func","RQS");

    //get fit parameters 
    fitIntercept = linFit_func->GetParameter(0);
    fitSlope = linFit_func->GetParameter(1);
    chiSq = linFit_func->GetChisquare(); 

    //calculate the residuals
    resP1 = pointY[0] - (fitSlope*pointX[0]) - fitIntercept; 
    resP2 = pointY[1] - (fitSlope*pointX[1]) - fitIntercept;
    resP3 = pointY[2] - (fitSlope*pointX[2]) - fitIntercept;
    resP4 = pointY[3] - (fitSlope*pointX[3]) - fitIntercept;

    /*
    //==== some more checks for debugging =================================
    cout << "=====================================================" << endl;
    cout << "Inside the TrackFit_4p fuction"  << endl;
    cout << "=====================================================" << endl;
    cout << "Boolean          :  " << isTrk << endl;
    cout << "Fitted Intercept :  " << fitIntercept << endl;
    cout << "Fitted Slope     :  " << fitSlope << endl;
    cout << "Chi Squared      :  " << chiSq << endl;
    cout << "ResP1            : " << resP1 << endl;
    cout << "ResP2            : " << resP2 << endl;
    cout << "ResP3            : " << resP3 << endl;
    cout << "ResP4            : " << resP4 << endl;  
    */
  }

  //==========================
  //  setting up the output
  //==========================
  fitResults result;

  result.isTrack = isTrk;
  result.fitParams[0] = fitIntercept;
  result.fitParams[1] = fitSlope;
  result.fitParams[2] = chiSq;
  result.residuals[0] = resP1;
  result.residuals[1] = resP2;
  result.residuals[2] = resP3;
  result.residuals[3] = resP4;

  return result;
}

