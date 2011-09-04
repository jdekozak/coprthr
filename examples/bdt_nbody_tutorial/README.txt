
This program implements a simple N-Body calculation using OpenCL and
corresponds to the bdt_nbody_tutorial.  The program is intended to be 
used as a tutorial and not designed for performance or sophistication.

See the bdt_nbody demo for a more sophisticated N-Body implementation.

In order to build the program you must have coprthr_sdk-1.0 installed
along with the ATI Stream SDK version 2.1.  

Type make to build and ./nbody.x to run.


export PYTHONPATH=/usr/lib/paraview
export LD_LIBRARY_PATH=<install_dir>/coprthr/src/libstdcl/
rm *csv *png; ./nbody.x ; ./post_process.py; mplayer output.avi
