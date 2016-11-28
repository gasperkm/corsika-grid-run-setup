# corsika-grid-run-setup
This software prepares a list of input files for a grid simulation with CORSIKA and reconstruction with Offline. After compiling the software (make), run the program with:<br/><br/>
   ./corsika_grid_run <GRID XRSL script> <CORSIKA-input> <GRID shell script> [<CORSIKA-stackin>]

# Input file (corsika_input and corsika_grid_run_input.dat):
The input file (corsika_input) determines the input parameters for CORSIKA. When running this software, it is possible to define/change the following input parameters:<br/>
- CORSIKA run number (RUNNR)<br/>
- Number of simulated showers (NSHOW)<br/>
- First event number (EVTNR)<br/>
- Random seed sequence numbers (SEED)<br/>
- Fractional thinning value (THIN)<br/>
- Primary particle type (PRMPAR)<br/>
-                  energy (ERANGE)<br/>
-                  zenith angle (THETAP)<br/>
-                  azimuth angle (PHIP)<br/>
This can automatically be setup, by using an input to this software (ex. corsika_grid_run_input.dat). The lines are as followed:<br/>
1.  Continue with the program (ex. <b>y</b>)<br/>
2.  CORSIKA run number (ex. <b>31</b>), this will create a uniform file naming scheme of DATXXXXXX (ex. DAT000031)<br/>
3.  The total number of simulated events (ex. <b>750</b>)<br/>
4.  The number of events per job (ex. <b>1</b>), this will create [total num.]/[per job] grid files (ex. 750/1 = 750)<br/>
5.  The number of the very first shower (ex. <b>31</b>), set most commonly to the same number as the run number<br/>
6.  The first random seed number (ex. <b>900050</b>), needed for the hadron shower part<br/>
7.  The second random seed number (ex. <b>900750</b>), needed for the EGS4 shower part (electromagnetic)<br/>
8.  The fractional thinning (ex. <b>1.e-7</b>), this enables thinning for energies below [primary energy]x[frac thinning] (more negative exponent means more detailed and longer simulation)<br/>
9.  Primary particle type (ex. <b>14</b> = proton)<br/>
10. Primary particle energy/energy range in units of eV (ex. <b>1.e+18</b>), for a range of energies split with a comma
11. Primary particle zenith angle/zenith angle range (ex. <b>30</b>), for a range of zenith angles split with comma
12. Primary particle azimuth angle/azimuth angle range (ex. <b>-360,360</b> = complete range), for a range of azimuth angles split with comma<br/>
13. The type of high energy hadron interaction model (ex. <b>QGSJETII</b>), the available models are QGSJET01, QGSJETII, SIBYLL, VENUS, EPOS, DPMJET and NEXUS<br/>
14. The setting for installation of CORSIKA (ex. <b>true</b>), for true CORSIKA is already installed on grid and for false it will be installed before running the simulation
15. The reconstruction type for Offline (ex. <b>HdInfill</b>), this determines the type of reconstruction that Offline will perform (available reconstructions are Hd, HdInfill, Sd and SdInfill)<br/>
This is done by running as an example:<br/>
   ./corsika_grid_run corsika_offline_both.xrsl corsika_input_both simulation_both < corsika_grid_run_input.dat
