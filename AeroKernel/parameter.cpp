/********************************************************************************
 *  File Name:
 *    parameter.cpp
 *
 *  Description:
 *    Implements the Aero Kernel Parameter Manager.
 *
 *  2019 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

#include <AeroKernel/parameter.hpp>

#include <iostream>

namespace AeroKernel::Parameter
{
  /*------------------------------------------------
  Compile Time Checks
  ------------------------------------------------*/
  static constexpr size_t ParamCtrlBlkSize = sizeof( ParamCtrlBlk );
  static_assert( ( sizeof( ParamCtrlBlk ) * sizeof( uint32_t ) ) % 32 == 0, "Parameter control block is not 32-bit aligned" );

  static_assert( static_cast<uint8_t>( MemoryLocation::MAX_MEMORY_LOCATIONS ) == 8, "Incorrect supported memory locations" );

  /*------------------------------------------------
  Constants
  ------------------------------------------------*/
  static constexpr size_t param_buffer_cutoff = 3; /* Limit how close we can get to filling the parameter manager*/


  Manager::Manager()
  {
  }

  Manager::~Manager()
  {
  }

  bool Manager::init( const size_t numElements )
  {
    params.clear();
    params.resize( numElements );
    memoryDriver.fill( nullptr );

    initialized = true;

    return true;
  }

  bool Manager::registerParameter( const std::string_view &key, const ParamCtrlBlk &controlBlock )
  {
    bool result = true;

    if ( !initialized )
    {
      result = false;
    }
    else if ( ( params.max_size() - params.size() ) <= param_buffer_cutoff )
    {
      result = false;
    }
    else
    {
      /*------------------------------------------------
      If the key does not exist in the map, it will be added. Otherwise
      the existing key will be accessed and the value updated.
      ------------------------------------------------*/
      params[ key ] = controlBlock;
    }

    return result;
  }

  bool Manager::isRegistered( const std::string_view &key )
  {
    return params.contains( key );
  }

  bool Manager::read( const std::string_view &key, void *const param, const size_t size )
  {
    return false;
  }

  bool Manager::write( const std::string_view &key, const void *const param, const size_t size )
  {
    return false;
  }

  bool Manager::update( const std::string_view &key )
  {
    return false;
  }

  bool Manager::remove( const std::string_view &key )
  {
    return false;
  }

  bool Manager::registerMemoryDriver( const MemoryLocation storage, Chimera::Modules::Memory::Device_sPtr &driver )
  {
    bool result = true;

    if ( !initialized || ( storage >= MemoryLocation::MAX_MEMORY_LOCATIONS ) )
    {
      result = false;
    }
    else
    {
      memoryDriver[ static_cast<uint8_t>( storage ) ] = driver;    
    }

    return result;
  }

  bool Manager::registerMemorySpecs( const MemoryLocation storage, const Chimera::Modules::Memory::Descriptor &specs )
  {
    return false;
  }

  const AeroKernel::Parameter::ParamCtrlBlk &Manager::getControlBlock( const std::string_view &key )
  {
    return params[ key ];
  }

}  // namespace AeroKernel::Parameter
