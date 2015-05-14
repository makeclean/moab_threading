#include <iostream>
#include <sstream>
#include <iomanip> // for setprecision                                                                                  
#include <limits> // for min/max values                                                                                 
#include <assert.h>
#include <math.h>
#include <time.h>
#include <vector>

#include "MBCore.hpp"
#include "MBTypes.h"
#include "MBTagConventions.hpp"
#include "MBRange.hpp"
#include "moab/Skinner.hpp"
#include "moab/GeomTopoTool.hpp"

#include "moab/OrientedBoxTreeTool.hpp"

#ifdef _OPENMP
 #include "omp.h"
#endif

MBInterface *MBI(); // Singleton moab
MBErrorCode setup_geometry(MBRange &surfaces, MBRange &volumes);
MBErrorCode build_obbs(moab::OrientedBoxTreeTool *obbTree, MBRange &surf, MBRange &volumes);

int main(int argc, char* argv[])
{
  // create empty meshset                                                                                                                      
  MBErrorCode result, rval;
  MBEntityHandle input_set;
  rval = MBI()->create_meshset( MESHSET_SET | MESHSET_TRACK_OWNER, input_set );

  // load the input file                                                                                                
  std::string input_name = argv[1];
  rval = MBI()->load_file( input_name.c_str(), &input_set );

  // setup the geometry
  MBRange surfs,volumes;
  rval = setup_geometry(surfs,volumes);
  
  std::cout << "Number of surfaces: " << surfs.size() << std::endl;
  std::cout << "Number of volumes: " << volumes.size() << std::endl;

  #pragma omp parallel 
  {
    #pragma omp critical
    {
      std::cout << omp_get_thread_num() << std::endl;
    }

    moab::OrientedBoxTreeTool *obb = new moab::OrientedBoxTreeTool(MBI());  
    rval = build_obbs(obb,surfs,volumes);
    
    #pragma omp taskwait

    #pragma omp critical
    {
      std::cout << omp_get_thread_num() << std::endl;
    }
  }

  return 0;
}


MBErrorCode build_obbs(moab::OrientedBoxTreeTool *obbTree, MBRange &surfs, MBRange &volumes)
{
  MBErrorCode rval = MB_SUCCESS;

  for (MBRange::iterator i = surfs.begin(); i != surfs.end(); ++i) {
    MBEntityHandle root;
    MBRange tris;
    rval = MBI()->get_entities_by_dimension( *i, 2, tris );
    if (MB_SUCCESS != rval)
      return rval;
    if (tris.empty())
      std::cerr << "WARNING: Surface " << *i << " has no facets." << std::endl;
    rval = obbTree->build( tris, root );
    if (MB_SUCCESS != rval)
      return rval;

    #pragma omp critical
    rval = MBI()->add_entities( root, &*i, 1 );
    if (MB_SUCCESS != rval)
      return rval;
  }

  return MB_SUCCESS;
}


MBErrorCode setup_geometry(MBRange &surfs, MBRange &volumes)
{
  MBErrorCode rval;

  MBTag geomTag;

  rval = MBI()->tag_get_handle( GEOM_DIMENSION_TAG_NAME, 1, MB_TYPE_INTEGER, geomTag, MB_TAG_DENSE|moab::MB_TAG_CREAT );

  const int three = 3;
  const void* const three_val[] = {&three};
  rval = MBI()->get_entities_by_type_and_tag( 0, MBENTITYSET, &geomTag,
                                            three_val, 1, volumes );
  if (MB_SUCCESS != rval)
    return rval;

  const int two = 2;
  const void* const two_val[] = {&two};
  rval = MBI()->get_entities_by_type_and_tag( 0, MBENTITYSET, &geomTag,
                                            two_val, 1, surfs );
  if (MB_SUCCESS != rval)
    return rval;

  return MB_SUCCESS;
}

MBInterface *MBI()
{
  static MBCore instance;
  return &instance;
}


