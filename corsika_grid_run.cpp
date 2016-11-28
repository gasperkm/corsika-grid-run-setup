#include <iostream>
#include <cstdio>
#include <string>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdlib.h>     // for the system() command
#include <assert.h>     // for the assert() command
#include <ctime>
#include <vector>

using namespace std;

void introduction();
string inttostring(int number);
string corsika_file_format(int iRun);
string stackin_old_format(int iRun, string prefix);
string IntToStr(int nr);
string DblToStr(double nr);
string GetParticle(int par);
string GetEnergy(double *ener);
string GetThin(double thin);
string GetZenith(double *zen);

int main(int argc, char* argv[])
{
  // Definition of variables
  string sGridRefName, sCorRefName, sShellName, sStackName;	// will hold filenames for initial grid xrsl script, corsika input, offline input, grid shell script and corsika stackin files
  string sNewInitial; // new initial filename (DAT00...)
  string sFolder; // name of the folder where we store everything
  string sJobname; // the constructed job name
  
  int iRunNum, iMaxRuns, iPerJob, iShowNum, iSeed[2], iJobs, iPrmPar; // definitions for: file numbering system, total nr. of simulations, nr. of showers per job, starting shower number, starting seed number, number of jobs
  double dEnergy[2]; // starting energy of the primary if no stackin (possibility to have a range)
  double dZenith[2]; // starting zenith angle of the primary (possibility to have a range)
  double dAzimuth[2]; // starting azimuth angle of the primary (possibility to have a range)
  bool bStackin; // checks if stackin is used
  char cSkip; // switch to stop program
  string sTempCom; // temporary command to give to system
  int iTemp; // temporary int for returns from functions

  time_t tTime; // holder for current time
  struct tm* localTime; // conversion to local time

  char *readTemp; // temporary dynamically allocated char to hold line information

  double dThinFrac; // fractional value for thinning

  // Print introduction and ask, if we wish to continue
  introduction();
  cin >> cSkip;
  if( (cSkip == 'n') || (cSkip == 'N') )
  {
    cout << "Exiting program to edit input files." << endl;
    assert(0);
  }

  // Check the input arguments
  if(argc == 1)
  {
    cout << "  Error! You did not enter any arguments (input files). Program is terminating." << endl;
    assert(0);
  }
  else
  {
    sGridRefName = string(argv[1]);
    sCorRefName = string(argv[2]);
    sShellName = string(argv[3]);
    
    if(argc == 5)
    {
      sStackName = string(argv[4]);
      bStackin = true;
    }
    else
    {
      cout << "No STACKIN file supplied, CORSIKA will run in non-STACKIN mode." << endl;
      bStackin = false;
    }
  }

  // Ask for all the used information
  cout << "Please enter all the needed information:" << endl;
  cout << "   - CORSIKA run number (RUNNR): ";
  cin >> iRunNum;
  cout << "   - Total nr. of simulations to be performed: ";
  cin >> iMaxRuns;
  cout << "   - Nr. of showers per job (NSHOW): ";
  cin >> iPerJob;
  cout << "   - Nr. of the very first shower (EVTNR): ";
  cin >> iShowNum;
  cout << "   - Starting random seed numbers: " << endl;
  cout << "   - Seed sequence 1 (SEED): ";
  cin >> iSeed[0];
  cout << "   - Seed sequence 2 (SEED): ";
  cin >> iSeed[1];
  cout << "   - Fractional thinning (THIN): ";
  cin >> dThinFrac;
  cout << "   - Primary particle number (see page 120 in the manual, PRMPAR): ";
  cin >> iPrmPar;

  // Ask for energy, if no stackin input
  if(!bStackin)
  {
    dEnergy[0] = -1., dEnergy[1] = -1.;
    cout << "   - Primary energy (in eV, if using range, separate with comma, ERANGE): ";
    scanf("%lf,%lf", &dEnergy[0], &dEnergy[1]);
    if(dEnergy[1] == -1.)
       dEnergy[1] = dEnergy[0];

    // Transform eV to GeV that corsika uses
    dEnergy[0] = dEnergy[0]*1.e-9;
    dEnergy[1] = dEnergy[1]*1.e-9;
  }

  // Ask for zenith and azimuth angles
  dZenith[0] = -1., dZenith[1] = -1.;
  dAzimuth[0] = -1., dAzimuth[1] = -1.;
  cout << "   - Zenith angle (if using range, separate with comma, THETAP): ";
  scanf("%lf,%lf", &dZenith[0], &dZenith[1]);
  if(dZenith[1] == -1.)
     dZenith[1] = dZenith[0];
  cout << "   - Azimuth angle (if using range, separate with comma, PHIP): ";
  scanf("%lf,%lf", &dAzimuth[0], &dAzimuth[1]);
  if(dAzimuth[1] == -1.)
     dAzimuth[1] = dAzimuth[0];

  // Setting up initial filename
  sNewInitial = corsika_file_format(iRunNum);

  // Construct the jobname from gathered information
  sJobname = sNewInitial + "_" + GetParticle(iPrmPar) + "_en" + GetEnergy(dEnergy) + "_thin" + GetThin(dThinFrac) + "_zenith" + GetZenith(dZenith);

  cout << endl << "Parameters used:" << endl
     << "iRunNum   = " << iRunNum << endl
     << "iMaxRuns  = " << iMaxRuns << endl
     << "iPerJob   = " << iPerJob << endl
     << "iShowNum  = " << iShowNum << endl
     << "iSeed[0]  = " << iSeed[0] << endl
     << "iSeed[1]  = " << iSeed[1] << endl
     << "dThinFrac = " << dThinFrac << endl
     << "iPrmPar   = " << iPrmPar << endl
     << "dEnergy   = " << dEnergy[0] << ", " << dEnergy[1] << endl
     << "dZenith   = " << dZenith[0] << ", " << dZenith[1] << endl
     << "dAzimuth  = " << dAzimuth[0] << ", " << dAzimuth[1] << endl;

  // Setting up workspace (new folder)
  sTempCom = "mkdir " + sNewInitial;
  iTemp = system(sTempCom.c_str());
  sTempCom = "mkdir " + sNewInitial + "/all_results";
  iTemp = system(sTempCom.c_str());
  sTempCom = "mkdir " + sNewInitial + "/all_simulations";
  iTemp = system(sTempCom.c_str());

  // Copying the ADST copy script to all_results
  sTempCom = "cp get_all_ADST_results.sh " + sNewInitial + "/all_results/";
  iTemp = system(sTempCom.c_str());

/*  if(iTemp != 0)		Temporarily disabling this check.
  {
    time(&tTime);
    localTime = localtime(&tTime);
    cout << "Directory " << sNewInitial << " already exists. Creating new one, based on current local time." << endl;

    sFolder = sNewInitial + "_";
    if((localTime->tm_hour)%24 < 10)
      sFolder = sFolder + "0" + inttostring((localTime->tm_hour)%24) + inttostring(localTime->tm_min) + inttostring(localTime->tm_sec);
    else
      sFolder = sFolder + inttostring((localTime->tm_hour)%24) + inttostring(localTime->tm_min) + inttostring(localTime->tm_sec);

    sTempCom = "mkdir " + sFolder;
    system(sTempCom.c_str());
  }
  else*/
    sFolder = sNewInitial + "/all_simulations";

  cout << "Created folder is " << sNewInitial << endl;

  // Setting number of jobs
  iJobs = iMaxRuns/iPerJob;

  // Copying STACKIN files to folder, with sequential file names
  if(bStackin)
  {
    cout << endl << "Copying STACKIN files to folder we just created." << endl;

    string sOldName;
    for(int i = 0; i < iJobs; i++)
    {
      int iNewNr;
      string sNewName;

      iNewNr = iRunNum + i;
      sOldName = "./stackin/" + stackin_old_format(i+1, sStackName);
      cout << sOldName << endl;
      sNewName = corsika_file_format(iNewNr) + "-stackin";

      sTempCom = "cp -v " + sOldName + " ./" + sFolder + "/" + sNewName;
      system(sTempCom.c_str());
    }

    // Checking the primary energy from STACKIN files for the use with thinning
    ifstream inStack;

    inStack.open(sOldName.c_str(), ifstream::in);
    if(inStack.is_open())
    {
      while(inStack.good())
      {
        inStack >> iTemp;
        inStack >> dEnergy[0];

        cout << endl << "Primary energy read from STACKIN file: " << dEnergy[0] << endl;
	dEnergy[1] = dEnergy[0];

        break;
      }
    }
  }

  // Copying the offline changes tar file to folder (both versions)
  cout << endl << "Copying Offline changes tar file to folder we just created." << endl;
  sTempCom = "cp set_offline.tar.gz ./" + sFolder + "/";
  iTemp = system(sTempCom.c_str());
  sTempCom = "cp set_offline_infill.tar.gz ./" + sFolder + "/";
  iTemp = system(sTempCom.c_str());

  // Copying the grid shell script to folder
  cout << endl << "Copying GRID shell script to folder we just created." << endl;
  sTempCom = "cp " + sShellName + " ./" + sFolder + "/";
  iTemp = system(sTempCom.c_str());

  // Editing CORSIKA input files
  ifstream inFile;
  ofstream outFile;

  for(int i = 0; i < iJobs; i++)
  {
    int iNewNr, iCurStartEvt, iSeedCount; // the new run number, the starting event number for current job, counter for random seeds
    double dThin; // the thinning value
    iSeedCount = 0;
    string sNewName;

    iNewNr = iRunNum + i;
    sNewName = "./" + sFolder + "/" + corsika_file_format(iNewNr) + "-input";

    iCurStartEvt = iShowNum + i*iPerJob;

    inFile.open(sCorRefName.c_str(), ifstream::in);
    outFile.open(sNewName.c_str(), ofstream::out);

    if(inFile.is_open())
    {
      while(inFile.good())
      {
        readTemp = new char[256];
        inFile.getline(readTemp,256); // read a full line

        if(inFile.eof())
          break;

        // check at which parameter we are
        if(string(readTemp).substr(0,6) == "RUNNR ")
          outFile << "RUNNR " << "  " << iNewNr << endl;
        else if(string(readTemp).substr(0,6) == "EVTNR ")
          outFile << "EVTNR " << "  " << iCurStartEvt << endl;
        else if(string(readTemp).substr(0,6) == "NSHOW ")
          outFile << "NSHOW " << "  " << iPerJob << endl;
        else if(string(readTemp).substr(0,6) == "PRMPAR")
          outFile << "PRMPAR" << "  " << iPrmPar << endl;
        else if( (string(readTemp).substr(0,6) == "INFILE") && bStackin )
          outFile << "INFILE" << "  " << "\'./input/" << corsika_file_format(iNewNr) << "-stackin" << "\'" << endl;
        else if( (string(readTemp).substr(0,6) == "ERANGE") && !bStackin )
          outFile << "ERANGE" << "  " << dEnergy[0] << "  " << dEnergy[1] << endl;
        else if(string(readTemp).substr(0,6) == "THETAP")
          outFile << "THETAP" << "  " << dZenith[0] << "  " << dZenith[1] << endl;
        else if(string(readTemp).substr(0,6) == "PHIP  ")
          outFile << "PHIP  " << "  " << dAzimuth[0] << "  " << dAzimuth[1] << endl;
        else if(string(readTemp).substr(0,6) == "SEED  ")
        {
          if(iSeedCount == 0)
          {
            outFile << "SEED  " << "  " << iSeed[0]+i << "   0   0" << endl;
            iSeedCount++;
          }
          else if(iSeedCount == 1)
          {
            outFile << "SEED  " << "  " << iSeed[1]+i << "   0   0" << endl;
            iSeedCount++;
          }
        }
        else if(string(readTemp).substr(0,6) == "THIN  ")
        {
	  if(dEnergy[0] == dEnergy[1])
	     dThin = dEnergy[0]*dThinFrac;
	  else
             dThin = (dEnergy[1]+dEnergy[0])*dThinFrac/2.;
          outFile << "THIN  " << "  " << dThinFrac << "  " << dThin << "  0" << endl;
        }
        else
          outFile << string(readTemp) << endl;

        delete[] readTemp;
      }
    }

    outFile.close();
    inFile.close();
  }

  // Editing GRID job xrsl scripts
  ifstream inGrid;
  string sSubstitute;
  vector<string> clustername;	// cluster names
  clustername.push_back("jost");
  clustername.push_back("zorro");

  cout << endl;
  for(int i = 0; i < clustername.size(); i++)
     cout << clustername[i] << endl;
  
  cout << endl << "Preparing GRID job xrsl scripts:" << endl;

  string sHadronmod, sPreinst, sRectype, sRectypetar;
  cout << "   - Select the high energy hadron model that will be used in CORSIKA (QGSJET01, QGSJETII, SIBYLL, VENUS, EPOS, DPMJET, NEXUS): ";
  cin >> sHadronmod;
  while( (sHadronmod != "QGSJET01") && (sHadronmod != "QGSJETII") && (sHadronmod != "SIBYLL") && (sHadronmod != "VENUS") && (sHadronmod != "EPOS") && (sHadronmod != "DPMJET") && (sHadronmod != "NEXUS") )
  {
     cout << "   - Error! Select the high energy hadron model that will be used in CORSIKA (QGSJET01, QGSJETII, SIBYLL, VENUS, EPOS, DPMJET, NEXUS): ";
     cin >> sHadronmod;
  }

  cout << "   - Select which CORSIKA version to use (true = preinstalled, false = fresh install from source): ";
  cin >> sPreinst;
  while( (sPreinst != "true") && (sPreinst != "false") )
  {
     cout << "   - Error! Select which CORSIKA version to use (true = preinstalled, false = fresh install from source): ";
     cin >> sPreinst;
  }

  cout << "   - Select which Offline reconstruction type to use (Hd = hybrid, HdInfill = hybrid infill, Sd = SD only, SdInfill = SD infill only): ";
  cin >> sRectype;
  while( (sRectype != "Hd") && (sRectype != "HdInfill") && (sRectype != "Sd") && (sRectype != "SdInfill") )
  {
     cout << "   - Error! Select which Offline reconstruction type to use (Hd = hybrid, HdInfill = hybrid infill, Sd = SD only, SdInfill = SD infill only): ";
     cin >> sRectype;
  }
  if( (sRectype == "Hd") || (sRectype == "Sd") )
     sRectypetar = "set_offline.tar.gz";
  else if( (sRectype == "HdInfill") || (sRectype == "SdInfill") )
     sRectypetar = "set_offline_infill.tar.gz";

  cout << "Used hadron model: " << sHadronmod << endl;
  cout << "Used CORSIKA version: " << sPreinst << endl;
  cout << "Used reconstruction type: " << sRectype << endl;

  inGrid.open(sGridRefName.c_str(), ifstream::in);

  if(inGrid.is_open())
  {
    while(inGrid.good())
    {
      readTemp = new char[256];
      inGrid.getline(readTemp,256); // read a full line

      if( string(readTemp).find("inputfiles") < string(readTemp).length() ) // find where we can find input files
      {
        inGrid.getline(readTemp,256);
        sSubstitute = string(readTemp).substr( (int)string(readTemp).find("DAT"), (int)string(readTemp).find("DAT")+5 );

        int iNewNr;
        string sNewName;

        for(int j = 0; j < clustername.size(); j++)
	{
           cout << "Preparing XRSL for cluster " << clustername[j] << endl;
	   
           for(int i = 0; i < iJobs; i++)
           {
             iNewNr = iRunNum + i;
             sNewName = "./" + sFolder + "/corsika_" + clustername[j] + "_" + corsika_file_format(iNewNr) + ".xrsl";

             if(clustername[j] == "jost")
	     {
                cout << "Preparing jost XRSL file " << sNewName << endl;
                sTempCom = "sed -e \'s/" + sSubstitute + "/" + corsika_file_format(iNewNr) + "/g\' -e \'s/JOBNAME/" + sJobname + "/g\' -e \'s/executable=jobrun.sh/executable=" + sShellName + "/g\' -e \'s/bothgrid/" + clustername[j] + "/g\' -e \'s/BOTHAPP/UNG/g\' -e \'s/HADRONMOD/" + sHadronmod + "/g\' -e \'s/PREINST/" + sPreinst + "/g\' -e \'s/RECTYPE/" + sRectype + "/g\' -e \'s/localtarname/" + sRectypetar + "/g\' " + sGridRefName + " > " + sNewName;
	     }
	     else if(clustername[j] == "zorro")
	     {
                cout << "Preparing zorro XRSL file " << sNewName << endl;
                sTempCom = "sed -e \'s/" + sSubstitute + "/" + corsika_file_format(iNewNr) + "/g\' -e \'s/JOBNAME/" + sJobname + "/g\' -e \'s/executable=jobrun.sh/executable=" + sShellName + "/g\' -e \'s/bothgrid/" + clustername[j] + "/g\' -e \'s/BOTHAPP/AUGER/g\' -e \'s/HADRONMOD/" + sHadronmod + "/g\' -e \'s/PREINST/" + sPreinst + "/g\' -e \'s/RECTYPE/" + sRectype + "/g\' -e \'s/localtarname/" + sRectypetar + "/g\' " + sGridRefName + " > " + sNewName;
	     }
           
//             cout << sTempCom << endl;
           
             system(sTempCom.c_str());
           }
	}

        break;
      }

      delete[] readTemp;
    }
  }

  inGrid.close();

  return 0;
}

// FUNCTIONS --------------------------------------------------------------------------------------
void introduction()
{
  cout << "-------------------------------------------------------" << endl
       << "---    Automatic GRID setup for running CORSIKA     ---" << endl
       << "-------------------------------------------------------" << endl << endl;
  cout << "This program will only edit the following corsika parameters:" << endl
       << "   - Filenames, according to CORSIKA file numbering" << endl
       << "     system (RUNNR)" << endl
       << "   - The number of first shower event of a job (EVTNR)" << endl
       << "   - Number of simulated showers per job (NSHOW)" << endl
       << "   - Random seed numbers for 1st and 2nd sequences (SEED)" << endl
       << "   - Primary type/number (PRMPAR)" << endl
       << "   - Primary energy (if no stackin file is supplied, ERANGE)" << endl
       << "   - Primary zenith angle (THETAP)" << endl
       << "   - Primary azimuth angle (PHIP)" << endl
       << "   - Thinning value (THIN)" << endl
       << "The rest should be prepared beforehand." << endl
       << "Would you like to continue (y/n)? ";
}

string inttostring(int number)
{
   stringstream ss;
   ss << number;

   return ss.str();
}

string corsika_file_format(int iRun)
{
  string sFilename;

  if( (iRun > 0) && (iRun < 10) )
    sFilename = "DAT00000" + inttostring(iRun);
  else if( (iRun >= 10) && (iRun < 100) )
    sFilename = "DAT0000" + inttostring(iRun);
  else if( (iRun >= 100) && (iRun < 1000) )
    sFilename = "DAT000" + inttostring(iRun);
  else if( (iRun >= 1000) && (iRun < 10000) )
    sFilename = "DAT00" + inttostring(iRun);
  else if( (iRun >= 10000) && (iRun < 100000) )
    sFilename = "DAT0" + inttostring(iRun);
  else if( (iRun >= 100000) && (iRun < 1000000) )
    sFilename = "DAT" + inttostring(iRun);
  else
  {
    cout << "Error! Run number can't be larger than 1000000." << endl;
    assert(0);
  }

  return sFilename;
}

string stackin_old_format(int iRun, string prefix)
{
  string sFilename;

  if( (iRun > 0) && (iRun < 10) )
    sFilename = prefix + "_00000" + inttostring(iRun);
  else if( (iRun >= 10) && (iRun < 100) )
    sFilename = prefix + "_0000" + inttostring(iRun);
  else if( (iRun >= 100) && (iRun < 1000) )
    sFilename = prefix + "_000" + inttostring(iRun);
  else if( (iRun >= 1000) && (iRun < 10000) )
    sFilename = prefix + "_00" + inttostring(iRun);
  else if( (iRun >= 10000) && (iRun < 100000) )
    sFilename = prefix + "_0" + inttostring(iRun);
  else if( (iRun >= 100000) && (iRun < 1000000) )
    sFilename = prefix + "_" + inttostring(iRun);
  else
  {
    cout << "Error! Run number can't be larger than 1000000." << endl;
    assert(0);
  }

  return sFilename;
}

// Int to string conversion
string IntToStr(int nr)
{
   stringstream ss;
   ss << nr;
   return ss.str();
}

// Double to string conversion
string DblToStr(double nr)
{
   stringstream ss;
//   ss << precision(3) << nr;
   ss << nr;
   return ss.str();
}

string GetParticle(int par)
{
   if(par == 1) return "photon";
   if(par == 2) return "positron";
   if(par == 3) return "electron";
   if(par == 5) return "muplus";
   if(par == 6) return "muminus";
   if(par == 13) return "neutron";
   if(par == 14) return "proton";
   if(par == 66) return "eneutrino";
   if(par == 68) return "muneutrino";
   if(par == 1206) return "carbon";
   if(par == 1608) return "oxygen";
   if(par == 5626) return "iron";
   else
      return "par" + IntToStr(par);
}

string GetEnergy(double *ener)
{
   if(ener[0] == ener[1])
      return DblToStr(log10(ener[0])+9.);
   else
      return DblToStr(log10(ener[0])+9.) + "-" + DblToStr(log10(ener[1])+9.);
}

string GetThin(double thin)
{
   return IntToStr((int)log10(thin));
}

string GetZenith(double *zen)
{
   if(zen[0] == zen[1])
      return DblToStr(zen[0]);
   else
      return DblToStr(zen[0]) + "-" + DblToStr(zen[1]);
}
