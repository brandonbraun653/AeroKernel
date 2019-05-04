/********************************************************************************
 *  File Name:
 *    event.cpp
 *
 *  Description:
 *    Implements the Event Manager
 *
 *  2019 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

#include <AeroKernel/event.hpp>

#include <limits>

namespace AeroKernel
{
  int someEventThing( const uint32_t x)
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