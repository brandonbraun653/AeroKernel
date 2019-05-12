/********************************************************************************
 *  File Name:
 *    parameter.cpp
 *
 *  Description:
 *    Implements the AeroKernel Parameter Manager.
 *
 *  2019 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

#include <AeroKernel/parameter.hpp>

namespace AeroKernel::Parameter
{
  /*------------------------------------------------
  Compile Time Checks
  ------------------------------------------------*/
  static constexpr size_t ParamCtrlBlkSize = sizeof( ParamCtrlBlk );
  //static_assert( ( ( sizeof( ParamCtrlBlk ) * sizeof( size_t ) ) % std::numeric_limits<size_t>::digits ) == 0,
  //               "Parameter control block is not system architecture aligned" );

  static_assert( static_cast<uint8_t>( Location::MAX_MEMORY_LOCATIONS ) == 8, "Incorrect supported memory locations" );

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

  bool Manager::unregisterParameter( const std::string_view &key )
  {
    return ( params.erase( key ) > 0 );
  }

  bool Manager::isRegistered( const std::string_view &key )
  {
    return params.contains( key );
  }

  bool Manager::read( const std::string_view &key, void *const param, const size_t size )
  {
    bool result = true;

    if ( !initialized || !param )
    {
      result = false;
    }
    else if ( !params.contains( key ) )
    {
      result = false;
    }
    else
    {
      auto ctrlBlk = params[ key ];
      auto storage = ( ctrlBlk.config & Location::MEM_LOC_MSK ) >> Location::MEM_LOC_POS;
      auto driver  = memoryDriver[ storage ];

      if ( driver && ( ctrlBlk.size == size ) )
      {
        Chimera::Status_t error = driver->read( ctrlBlk.address, reinterpret_cast<uint8_t *const>( param ), size );

        if ( error != Chimera::CommonStatusCodes::OK )
        {
          result = false;
        }
      }
      else
      {
        result = false;
      }
    }

    return result;
  }

  bool Manager::write( const std::string_view &key, const void *const param, const size_t size )
  {
    bool result = true;

    if ( !initialized || !param )
    {
      result = false;
    }
    else if ( !params.contains( key ) )
    {
      result = false;
    }
    else
    {
      auto ctrlBlk = params[ key ];
      auto storage = ( ctrlBlk.config & Location::MEM_LOC_MSK ) >> Location::MEM_LOC_POS;
      auto driver  = memoryDriver[ storage ];

      if ( driver && ( ctrlBlk.size == size ) )
      {
        Chimera::Status_t error = driver->write( ctrlBlk.address, reinterpret_cast<const uint8_t *const>( param ), size );

        if ( error != Chimera::CommonStatusCodes::OK )
        {
          result = false;
        }
      }
      else
      {
        result = false;
      }
    }

    return result;
  }

  bool Manager::update( const std::string_view &key )
  {
    bool result = true;

    if ( !initialized )
    {
      result = false;
    }
    else if ( !params.contains( key ) )
    {
      result = false;
    }
    else
    {
      auto ctrlBlk = params[ key ];

      if ( ctrlBlk.update )
      {
        result = ctrlBlk.update( key );
      }
      else
      {
        result = false;
      }
    }

    return result;
  }

  bool Manager::registerMemoryDriver( const uint32_t storage, Chimera::Modules::Memory::Device_sPtr &driver )
  {
    bool result = true;

    if ( !initialized || ( storage >= Location::MAX_MEMORY_LOCATIONS ) )
    {
      result = false;
    }
    else
    {
      memoryDriver[ storage ] = driver;
    }

    return result;
  }

  bool Manager::registerMemorySpecs( const uint32_t storage, const Chimera::Modules::Memory::Descriptor &specs )
  {
    bool result = true;

    if ( !initialized || ( storage >= Location::MAX_MEMORY_LOCATIONS ) )
    {
      result = false;
    }
    else
    {
      memorySpecs[ storage ] = specs;
    }

    return result;
  }

  const AeroKernel::Parameter::ParamCtrlBlk &Manager::getControlBlock( const std::string_view &key )
  {
    return params[ key ];
  }

}  // namespace AeroKernel::Parameter
