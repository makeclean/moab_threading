#ifdef _OPENMP
 #include "omp.h"
#endif

#include <iostream>
#include <sstream>
#include <iomanip> // for setprecision
#include <limits> // for min/max values
#include <assert.h>
#include <math.h>
#include <time.h>
#include <vector>

#include "MBCore.hpp"
#include "MBTagConventions.hpp"
#include "MBRange.hpp"
#include "moab/Skinner.hpp"
#include "moab/GeomTopoTool.hpp"

#include <unistd.h> // for timing

#include "DagMC.hpp"
using moab::DagMC;

#define DAG DagMC::instance()

MBInterface *MBI(); // singleton
void setup_coords(double coordinates[8][3]);
MBErrorCode find_cell(double coord[3], int &success, MBEntityHandle &cell);

int main(int argc, char* argv[])
{
  // create empty meshset                                                                                                                
  MBErrorCode result, rval;
  MBEntityHandle input_set;

  rval = DAG->load_file(argv[1],1.0e-4);
  // vector of positions
  double coordinates [8][3];
  
  rval = DAG->init_OBBTree();
 
 // build the OBB

  // initial coordinates
  setup_coords(coordinates);

  // find the cell
  MBEntityHandle vol;
  int success;
 
  #pragma omp parallel for
  for ( int i = 0 ; i < 100000 ; i++ )
    {
      {
         rval = find_cell(coordinates[0],success,vol);
      }
      
      if( i%10000 == 0 )
	std::cout << "i = " << i << std::endl;
     }
  
  return 0;
}

void setup_coords(double coordinates[8][3])
{
  coordinates[0][0] = 0.0,coordinates[0][1]=0.0,coordinates[0][2]=5.0;
  coordinates[1][0] = 0.0,coordinates[1][1]=0.0,coordinates[1][2]=15.0;
  coordinates[2][0] = 0.0,coordinates[2][1]=0.0,coordinates[2][2]=25.0;
  coordinates[3][0] = 0.0,coordinates[3][1]=0.0,coordinates[3][2]=35.0;
  coordinates[4][0] = 0.0,coordinates[4][1]=0.0,coordinates[4][2]=45.0;
  coordinates[5][0] = 0.0,coordinates[5][1]=0.0,coordinates[5][2]=55.0;
  coordinates[6][0] = 0.0,coordinates[6][1]=0.0,coordinates[6][2]=65.0;
  coordinates[7][0] = 0.0,coordinates[7][1]=0.0,coordinates[7][2]=75.0;

  return;
}

MBErrorCode find_cell(double coord[3], int &success, MBEntityHandle &cell)
{
  MBErrorCode rval;
  int num_vols = DAG->num_entities(3);
  MBEntityHandle volume_ent;

  for ( int i = 1 ; i <= num_vols ; i++) 
    {
      volume_ent = DAG->entity_by_index(3,i);
      rval = DAG->point_in_volume(volume_ent, coord,success);
      if(success == 1 )
	{
	  cell = volume_ent;
	  return MB_SUCCESS;
	}
    }
  return MB_FAILURE;
}

MBInterface *MBI() 
{
  static MBCore instance;
  return &instance;
}
