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

MBInterface *MBI();
void delete_triangle(MBEntityHandle meshset);
void print_status(MBEntityHandle meshset);

int main(int argc, char* argv[])
{
  // create empty meshset                                                                                                                
  MBErrorCode result, rval;
  MBEntityHandle input_set;
  rval = MBI()->create_meshset( MESHSET_SET | MESHSET_TRACK_OWNER, input_set );

  // load the input file
  std::string input_name = argv[1];
  rval = MBI()->load_file( input_name.c_str(), &input_set );
  if(MB_SUCCESS == rval ) {
    std::cout << "Loading input file..." << std::endl;
  } else {
    std::cout << "failed to load input file" << std::endl;
    return 1;
  }

  // Test 1
  // threads with id higher than num_thread/2 will be same as those below
  print_status(input_set);
  delete_triangle(input_set);

  // threads with id higher than num_thread/2 should have 1 less than 
  print_status(input_set);
  delete_triangle(input_set);

  // threads with id higher than num_thread/2 should have 2 less than 
  print_status(input_set);

  return 0;
}

void print_status(MBEntityHandle meshset){
  unsigned int microseconds;
  int num_ents;
  MBErrorCode rval;

  #pragma omp parallel private(num_ents)
  {
    #ifdef _OPENMP
    if(omp_get_thread_num() < omp_get_num_threads()/2 )
      {
        rval = MBI()->get_number_entities_by_dimension(meshset,1,num_ents);
	usleep(omp_get_thread_num()*1000);
	std::cout << "Thread " << omp_get_thread_num() << " Number of 1d entities " << num_ents << std::endl;
      }
    else
      {
        rval = MBI()->get_number_entities_by_dimension(meshset,2,num_ents);
	usleep(omp_get_thread_num()*1000);
	std::cout << "Thread " << omp_get_thread_num() << " Number of 2d entities " << num_ents << std::endl;;
      }
    #else
      rval = MBI()->get_number_entities_by_dimension(meshset,1,num_ents);
      std::cout << "Number of 1d entities " << num_ents << std::endl;  
      rval = MBI()->get_number_entities_by_dimension(meshset,2,num_ents);
      std::cout << "Number of 2d entities " << num_ents << std::endl;  
    #endif
  } 
  return;
}

void delete_triangle(MBEntityHandle meshset) {
  MBErrorCode rval;
  // remove first triangle
  MBRange all_triangles; // all tris
  MBRange single_tri;
  // get all tris
  rval = MBI()->get_entities_by_type(meshset,MBTRI,all_triangles);
  // insert first tri into range
  single_tri.insert(all_triangles[1]);
  // delete tri
  rval = MBI()->delete_entities(single_tri);
  if(rval!=MB_SUCCESS)
    std::cout << "could not delete entities" << std::endl;
  return;
}


MBInterface *MBI() 
{
  static MBCore instance;
  return &instance;
}
