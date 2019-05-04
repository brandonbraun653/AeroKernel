/********************************************************************************
 *  File Name:
 *    log.cpp
 *
 *  Description:
 *    Implements the log manager
 *
 *  2019 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

#include <AeroKernel/log.hpp>

#include <limits>

namespace AeroKernel
{
  int someLogThing( const uint32_t x)
  {
    int retVal = 500;

    if( x < ( std::numeric_limits<uint32_t>::max() / 2 ) )
    {
      retVal = 100;
    }
    else
    {
      retVal = 200;
    }
    
    return retVal;
  }
}