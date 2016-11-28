#!/bin/bash

#============ SETUP OF DIRECTORY STRUCTURE ================
# current directory where the bash script is located
startdir=$(pwd)

# creating a results directory
if [ ! -d "results" ]; then
  mkdir $startdir/results
fi
resultpath=$startdir/results

# creating a corsika working directory
if [ ! -d "corsika" ]; then
  mkdir $startdir/corsika
fi
corsikapath=$startdir/corsika

# creating an offline working directory

# Create the working directory
if [ ! -d "aperun" ]; then
  mkdir $startdir/aperun
fi
offdir=$startdir/aperun

echo "ls -lh $startdir"
ls -lh $startdir

# name the arguments
simname=$1
hadmod=$2
presetup=$3
recontype=$4
#==========================================================

#============== TIME DIFFERENCE FUNCTION ==================
function timedif {
  argstart=(${1#0} ${2#0} ${3#0} ${4#0} ${5#0} ${6#0})
  temp1=${10} temp2=${11} temp3=${12}
  argend=(${7#0} ${8#0} ${9#0} ${temp1#0} ${temp2#0} ${temp3#0})

  # checking for leap year
  if [ ${argstart[2]} -eq ${argend[2]} ]; then
    temp1=$((${argstart[2]}/4))
    temp2=$(($temp1*4))
    temp3=$((${argstart[2]} - $temp2))

    if [ $temp3 -eq 0 ]; then
      leapyear=1
    else
      leapyear=0
    fi
  else
    echo "Error! Year date stamps are not the same."
  fi

  # if job runs between two months
  temp1=$((${argend[0]} - ${argstart[0]}))
  if [ $temp1 -lt 0 ]; then
    if [ ${argstart[1]} -eq 1 -o ${argstart[1]} -eq 3 -o ${argstart[1]} -eq 5 -o ${argstart[1]} -eq 7 -o ${argstart[1]} -eq 8 -o ${argstart[1]} -eq 10 -o ${argstart[1]} -eq 12 ]; then
      argend[0]=$((${argend[0]} + 31))
    fi

    if [ ${argstart[1]} -eq 4 -o ${argstart[1]} -eq 6 -o ${argstart[1]} -eq 9 -o ${argstart[1]} -eq 11 ]; then
      argend[0]=$((${argend[0]} + 30))
    fi

    if [ $leapyear -eq 1 -a ${argstart[1]} -eq 2 ]; then
      argend[0]=$((${argend[0]} + 29))
    elif [ $leapyear -eq 0 -a ${argstart[1]} -eq 2 ]; then
      argend[0]=$((${argend[0]} + 28))
    fi

    temp1=$((${argend[0]} - ${argstart[0]}))
  fi

  outarray[2]=$temp1
  temp1=$((${argstart[5]} + ${argstart[4]}*60 + ${argstart[3]}*3600 + ${argstart[0]}*86400))
  temp2=$((${argend[5]} + ${argend[4]}*60 + ${argend[3]}*3600 + ${argend[0]}*86400))
  temp3=$(($temp2 - $temp1))

  outarray[5]=$temp3

  # calculating seconds
  temp1=$((${outarray[5]}/60))
  if [ $temp1 -gt 0 ]; then
    temp2=$(($temp1*60))
    temp3=$((${outarray[5]} - $temp2))
    outarray[5]=$temp3
    outarray[4]=$temp1
  else
    outarray[4]=0
  fi

  # calculating minutes
  temp1=$((${outarray[4]}/60))
  if [ $temp1 -gt 0 ]; then
    temp2=$(($temp1*60))
    temp3=$((${outarray[4]} - $temp2))
    outarray[4]=$temp3
    outarray[3]=$temp1
  else
    outarray[3]=0
  fi

  # calculating hours
  temp1=$((${outarray[3]}/24))
  if [ $temp1 -gt 0 ]; then
    temp2=$(($temp1*24))
    temp3=$((${outarray[3]} - $temp2))
    outarray[3]=$temp3
  fi

  # adding leading zeroes
  if [ ${outarray[3]} -lt 10 ]; then
    outarray[3]=$(echo "0${outarray[3]}")
  fi
  if [ ${outarray[4]} -lt 10 ]; then
    outarray[4]=$(echo "0${outarray[4]}")
  fi
  if [ ${outarray[5]} -lt 10 ]; then
    outarray[5]=$(echo "0${outarray[5]}")
  fi

  if [ ${outarray[2]} -lt 1 ]; then
    echo "Simulation time: ${outarray[3]}:${outarray[4]}:${outarray[5]}" >> $resultpath/timing.txt
  else
    echo "Simulation time: ${outarray[2]}d ${outarray[3]}:${outarray[4]}:${outarray[5]}" >> $resultpath/timing.txt
  fi
}
#==========================================================

#=============== COLOR OUTPUT FUNCTIONS ===================
# option to color the standard outputs
colorful="no"

# red coloring function for errors
function errorcolour 
{
  if [ $colorful == "yes" ]; then
    echo -e "\033[31m$1\033[39m"
  else
    echo $1
  fi
}

# yellow coloring function to indicate bash commands
function bashcolour
{
  if [ $colorful == "yes" ]; then
    echo -e$2 "\033[33m$1\033[39m"
  else
    echo $1
  fi
}

# error function
function error_exit
{
  errorcolour "$1"
  exit 1
}

# function for printing timing information to a log
function timelog
{
  echo $1 >> $resultpath/timing.txt
}
#==========================================================

#========== SETTING OFFLINE AND COMPILING IT ==============
# Check the version of offline
offversion="new"

# Check if we supplied an .aperc file as an input file
echo "Setting up offline environment variables:"
if [ ! -f $HOME/.aperc ]; then
  cp -v $APEINSTALLDIR/.aperc $HOME/
  echo ".aperc file copied from $APEINSTALLDIR."
else
  echo ".aperc file supplied by user."
fi

# Setup offline environment variables
if [ $offversion == "old" ]; then
  echo "Setting up old version of Offline."
  eval `ape sh offline`
elif [ $offversion == "new" ]; then
  echo "Setting up new version of Offline."
  source $APEINSTALLDIR/env_offline.sh
fi

# Copy example script from offline tutorials
if [ $offversion == "old" ]; then
  if [ $recontype == "Hd" ]; then
    echo "Copying the HdSimulationReconstruction files."
    cp -r $APEINSTALLDIR/offline/2.9.1-Valentine/share/auger-offline/doc/StandardApplications/HdSimulationReconstruction/* $offdir/
  elif [ $recontype == "HdInfill" ]; then
    echo "Copying the HdInfillSimulationReconstruction files."
    cp -r $APEINSTALLDIR/offline/2.9.1-Valentine/share/auger-offline/doc/StandardApplications/HdInfillSimulationReconstruction/* $offdir/
  elif [ $recontype == "Sd" ]; then
    echo "Copying the SdSimulationReconstruction files."
    cp -r $APEINSTALLDIR/offline/2.9.1-Valentine/share/auger-offline/doc/StandardApplications/SdSimulationReconstruction/* $offdir/
  elif [ $recontype == "SdInfill" ]; then
    echo "Copying the SdInfillSimulationReconstruction files."
    cp -r $APEINSTALLDIR/offline/2.9.1-Valentine/share/auger-offline/doc/StandardApplications/SdInfillSimulationReconstruction/* $offdir/
  fi
elif [ $offversion == "new" ]; then
  if [ $recontype == "Hd" ]; then
    echo "Copying the HdSimulationReconstruction files."
    cp -r $APEINSTALLDIR/offline/share/auger-offline/doc/StandardApplications/HdSimulationReconstruction/* $offdir/
  elif [ $recontype == "HdInfill" ]; then
    echo "Copying the HdInfillSimulationReconstruction files."
    cp -r $APEINSTALLDIR/offline/share/auger-offline/doc/StandardApplications/HdInfillSimulationReconstruction/* $offdir/
  elif [ $recontype == "Sd" ]; then
    echo "Copying the SdSimulationReconstruction files."
    cp -r $APEINSTALLDIR/offline/share/auger-offline/doc/StandardApplications/SdSimulationReconstruction/* $offdir/
  elif [ $recontype == "SdInfill" ]; then
    echo "Copying the SdInfillSimulationReconstruction files."
    cp -r $APEINSTALLDIR/offline/share/auger-offline/doc/StandardApplications/SdInfillSimulationReconstruction/* $offdir/
  fi
fi

# Update the example script with the custom one
tar -zxf set_offline.tar.gz
cp $startdir/set_offline/* $offdir
sed "s/DAT....../$simname/" $startdir/set_offline/EventFileReader.xml.in > $offdir/EventFileReader.xml.in
#sed "s/DAT....../$simname/" $startdir/set_offline/MyModule.xml.in > $offdir/MyModule.xml.in
echo "ls -lh $startdir/set_offline/"
ls -lh $startdir/set_offline/
echo "ls -lh $offdir"
ls -lh $offdir
rm -r set_offline*

# Compiling offline
cd $offdir
make
if [ $? != 0 ]; then
  error_exit "Error! Make of offline not succesful."
fi
cd $startdir
#==========================================================

# timing information
timelog "Running of ./simulation.sh"
timelog $(echo "Start time: "$(date +%d.%m.%Y)", "$(date +%H:%M:%S))

# check if there are any arguments
if [ $# -gt 0 ]; then

#================= SETTING UP CORSIKA =====================
  timelog $(echo "Setting up CORSIKA: "$(date +%d.%m.%Y)", "$(date +%H:%M:%S))

  if [ "$presetup" != "" ]; then
    echo "Setting the preset variable ($presetup)."
  fi

  # check if corsika is preinstalled or not
  if [ "$presetup" == "true" ]; then
    echo "Corsika has been preinstalled"
    if [ "$SLURM_CLUSTER_NAME" == "ung" ]; then
      echo "Cluster is UNG"
      export FLUPRO=/grid/software/auger/CORSIKA-74005_Fluka.2011.2c.1/fluka
      export FLUFOR=gfortran
      export CORSIKADIR=/grid/software/auger/CORSIKA-74005_Fluka.2011.2c.1
      export CORSIKAHOME=/grid/software/auger/CORSIKA-74005_Fluka.2011.2c.1/run
    elif [ "$SLURM_CLUSTER_NAME" == "arnes" ]; then
      echo "Cluster is ARNES"
#      export FLUPRO=/cvmfs/auger.egi.eu/CORSIKA-74005_Fluka.2011.2c.2/fluka
 #    export FLUPRO=/grid/arc/sw/corsika/fluka
 #    export FLUFOR=gfortran
#      export CORSIKADIR=/cvmfs/auger.egi.eu/CORSIKA-74005_Fluka.2011.2c.2
#      export CORSIKADIR=/grid/arc/sw/corsika/corsika-74005
      echo "CORSIKADIR: $CORSIKADIR, CORSIKAHOME: $CORSIKAHOME, FLUPRO: $FLUPRO, FLUFOR: $FLUFOR"
    else
      error_exit "Unknown cluster."
    fi
  elif [ "$presetup" == "false" ]; then
    echo "Corsika has not been preinstalled"
    echo "CORSIKADIR: $CORSIKADIR, CORSIKAHOME: $CORSIKAHOME, FLUPRO: $FLUPRO, FLUFOR: $FLUFOR"
  else
    error_exit "Preset argument not set correctly."
  fi

  # only setup corsika, if using a version that has already been installed
  if [ "$presetup" == "false" ]; then
    # copying corsika files to working corsika directory
    echo "Copying the complete corsika directory ($CORSIKADIR)"
    cp -r $CORSIKADIR/* $corsikapath/

    cd $startdir

    # compiling corsika through ./configure
    if [ -d $corsikapath ]; then
      cd $corsikapath
      bashcolour "Compiling CORSIKA through ./configure (Hadron model is $hadmod)"
      # can find out the arguments by running ./coconut --help
      ./configure CORHEMODEL=$hadmod --with-fluka --enable-SLANT --enable-THIN CORDETECTOR=HORIZONTAL CORLEMODEL=FLUKA CORTIMELIB=TIMEAUTO --prefix=$corsikapath --bindir=$corsikapath/run --libdir=$corsikapath/lib/unknown --program-suffix="_executable_$hadmod"
      make
      make install
      cd $startdir
      bashcolour "Done!"
    fi
  elif [ "$presetup" == "true" ]; then
    echo "Copying only the corsika run directory ($CORSIKAHOME)"
    mkdir $corsikapath/run
    cp -r $CORSIKAHOME/* $corsikapath/run/

    cd $startdir
  fi
#==========================================================

#================ INPUT AND OUTPUT FILES ==================
  timelog $(echo "Setting input and output files: "$(date +%d.%m.%Y)", "$(date +%H:%M:%S))
  if [ "$simname" != "" ]; then
    bashcolour "Copying the input files to CORSIKA run directory."
    if [ -d $corsikapath ]; then
      # copying the input files
      cd $corsikapath
      infile=$(echo $simname"-input")
      mkdir $corsikapath/run/input
      mkdir $corsikapath/run/output

      sed -e "s/TODOOUT/$(echo $corsikapath | sed 's/\//\\\//g')\/run\/output\//g" $startdir/$infile > $corsikapath/run/input/$infile
      echo "Checking the input file"
      cat $corsikapath/run/input/$infile

      echo "corsikapath: ls -lh $corsikapath/"
      ls -lh $corsikapath/
      echo "corsikapath/run: ls -lh $corsikapath/run/"
      ls -lh $corsikapath/run/
      echo "CORSIKADIR: ls -lh $CORSIKADIR/"
      ls -lh $CORSIKADIR/
      echo "CORSIKADIR/run: ls -lh $CORSIKAHOME/"
      ls -lh $CORSIKAHOME/
      echo "FLUPRO: ls -lh $FLUPRO"
      ls -lh $FLUPRO
      echo "FLUFOR: $FLUFOR"
      echo "HADRON INTERACTION MODEL: $hadmod"
#==========================================================

#============== RUNNING CORSIKA SIMULATION ================
      timelog $(echo "Running CORSIKA simulation: "$(date +%d.%m.%Y)", "$(date +%H:%M:%S))
      startdate=($(date +%d) $(date +%m) $(date +%Y) $(date +%H) $(date +%M) $(date +%S))

      corrun=$corsikapath/run
      if [ "$presetup" == "false" ]; then
        runcommand=corsika_executable_$hadmod
        bashcolour "Running the CORSIKA simulation with $corrun/$runcommand < ./input/$infile"
        cd $corrun
        $corrun/$runcommand < ./input/$infile
        if [ $? -ne 0 ]; then
          error_exit "Error! Run of CORSIKA failed."
        else
          bashcolour "Done!"
        fi
      elif [ "$presetup" == "true" ]; then
#        runcommand=corsika74005Linux_QGSII_fluka_thin
        runcommand=corsika_executable_$hadmod
        bashcolour "Running the CORSIKA simulation with $CORSIKAHOME/$runcommand < ./input/$infile"
        cd $corrun
        $CORSIKAHOME/$runcommand < ./input/$infile
        if [ $? -ne 0 ]; then
          error_exit "Error! Run of CORSIKA failed."
        else
          bashcolour "Done!"
        fi
      fi

      # check to see, if simulation ended correctly
      if [ -f $startdir/corsika_offline.log ]; then
         endcheck=$(grep -c "No space left" $startdir/corsika_offline.log)
         if [ $endcheck != 0 ]; then
            error_exit "Error! The CORSIKA simulation failed."
         fi
      fi

      # calculation of time spent during simulation
      enddate=($(date +%d) $(date +%m) $(date +%Y) $(date +%H) $(date +%M) $(date +%S))
      timedif ${startdate[0]} ${startdate[1]} ${startdate[2]} ${startdate[3]} ${startdate[4]} ${startdate[5]} ${enddate[0]} ${enddate[1]} ${enddate[2]} ${enddate[3]} ${enddate[4]} ${enddate[5]}
#==========================================================

#============= COPYING CORSIKA RESULT FILES ===============
      # copying the output files
      timelog $(echo "Copying output files: "$(date +%d.%m.%Y)", "$(date +%H:%M:%S))
      bashcolour "Copying the output files to base directory."
      cd $corsikapath/run/input
      cp ./* $resultpath
      cd $corsikapath/run/output
      find ./ -type f ! -name $simname -exec cp -t $resultpath/ {} +

      # waiting so the copy of CORSIKA output files is complete
      sleep 60
      echo "ls -lh $resultpath/"
      ls -lh $resultpath/
#==========================================================
    else
      error_exit "Error! No CORSIKA directory $corsikapath"
    fi
  else
    error_exit "Error! No input files for CORSIKA were given as third argument."
  fi

#============= RUNNING OFFLINE RECONSTRUCT ================
  timelog $(echo "Running offline simulation: "$(date +%d.%m.%Y)", "$(date +%H:%M:%S))
  startdate=($(date +%d) $(date +%m) $(date +%Y) $(date +%H) $(date +%M) $(date +%S))
  cd $offdir
  ./userAugerOffline -b bootstrap.xml

  # check to see, if reconstruction ended correctly
  if [ -f $startdir/corsika_offline.log ]; then
    endcheck=$(grep -c "corrupt" $startdir/corsika_offline.log)
    if [ $endcheck != 0 ]; then
      error_exit "Error! The Offline reconstruction failed."
    fi
  fi

  # calculation of time spent during simulation
  enddate=($(date +%d) $(date +%m) $(date +%Y) $(date +%H) $(date +%M) $(date +%S))
  timedif ${startdate[0]} ${startdate[1]} ${startdate[2]} ${startdate[3]} ${startdate[4]} ${startdate[5]} ${enddate[0]} ${enddate[1]} ${enddate[2]} ${enddate[3]} ${enddate[4]} ${enddate[5]}
#==========================================================

#============= COPYING OFFLINE RESULT FILES ===============
  # Copy offline results to the result directory
  cp $offdir/*.root $offdir/HybridRec.dat $offdir/mymodule* $resultpath/
#==========================================================

#=============== BACK TO BASE AND REMOVE ==================
  timelog $(echo "Cleaning the installation directory: "$(date +%d.%m.%Y)", "$(date +%H:%M:%S))
  cd $startdir
  bashcolour "Removing the OFFLINE and CORSIKA installation directories."
  rm -rf $corsikapath $offdir
  bashcolour "Creating the archive with output files."

  # Creating the tar-ball and getting rid of the large particle file (already used it in offline analysis, so we do not need it anymore)
  ls -lh $resultpath/
  rm -fr $resultpath/$simname

  cd $startdir
  # Creating the tar-ball for user to get immediately (without the corsika particle output)
  tar -cvzf $(echo $simname".tar.gz") ./results
#==========================================================
else
  error_exit "Error! No CORSIKA input files supplied."
fi

timelog $(echo "End time: "$(date +%d.%m.%Y)", "$(date +%H:%M:%S))

exit 0
