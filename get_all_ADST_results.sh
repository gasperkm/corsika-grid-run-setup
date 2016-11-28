#!/bin/bash

# option to color the standard outputs
colorful="yes"

# current directory where the bash script is located
startdir=$(pwd)

###########################################
# creating a results directory
if [ ! -d "adst_results" ]; then
  mkdir ./adst_results
fi
resultpath=$(echo $startdir"/adst_results")
###########################################

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

failedjobs=""
firsterr=1

# check if there are any arguments
if [ $# -gt 0 ]; then

  echo "Emptying results folder."
  rm $resultpath/*.root

  for var in "$@"
  do
    if [ -d $var ]; then
      echo "Moving to folder: $var"
      cd $var
      echo "Checking if corsika and offline reconstructions finished correctly."
      grep -q "std::bad_alloc" corsika_offline.log
      if [ $? == 0 ]; then
        isgood=0
      else
        isgood=1
      fi
      echo "isgood = $isgood"

      if [ $isgood == 1 ]; then
        if [ ! -d "results" ]; then
          bashcolour "Untarring the results."
  	  tar -zxf *.tar.gz
        fi
        echo "Copying ADST results file to $resultpath/"
        cd results
        cp ADST.root $resultpath/ADST_${var}.root
      else
        errorcolour "Reconstruction for $var did not finish correctly."
	if [ $firsterr == 1 ]; then
	  failedjobs="$var"
	  firsterr=0
	else
	  failedjobs="$failedjobs\n$var"
	fi
      fi
    else
      errorcolour "Can not find folder: $var"
    fi
    cd $startdir
  done

  # report which jobs failed
  echo ""
  bashcolour "The jobs that failed are:"
  echo -e $failedjobs

  cd $resultpath
#  bashcolour "Combining all results into a single tar-ball."
#  tar -zcf massanalysis_results.tar.gz massanalysis*.root
fi

exit 0
