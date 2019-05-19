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
  static_assert( static_cast<uint8_t>( Location::MAX_MEMORY_LOCATIONS ) == 8, "Incorrect supported memory locations" );


  Manager::Manager(const size_t lockTimeout_mS) : initialized(false), lockTimeout_mS(lockTimeout_mS)
  {
  }

  Manager::~Manager()
  {
  }

  bool Manager::init( const size_t numParameters )
  {
    params.clear();
    params.resize( numParameters );
    params.set_resizing_parameters( 0.0f, 0.1f );

    memoryDriver.fill( nullptr );

    initialized = true;

    return true;
  }

  bool Manager::registerParameter( const std::string_view &key, const ParamCtrlBlk &controlBlock )
  {
    bool result = false;

    /*------------------------------------------------
    If the key does not exist in the map, it will be added. Otherwise
    the existing key will be accessed and the value updated.
    ------------------------------------------------*/
    if ( initialized && ( reserve( lockTimeout_mS ) == Chimera::CommonStatusCodes::OK ) )
    {
      params[ key ] = controlBlock;
      release();
      result = true;
    }

    return result;
  }

  bool Manager::unregisterParameter( const std::string_view &key )
  {
    bool result = false;

    if ( initialized && ( reserve( lockTimeout_mS ) == Chimera::CommonStatusCodes::OK ) )
    {
      result = params.erase( key ) > 0;
      release();
    }
    
    return result;
  }

  bool Manager::isRegistered( const std::string_view &key )
  {
    bool result = false;
    
    if ( initialized && ( reserve( lockTimeout_mS ) == Chimera::CommonStatusCodes::OK ) )
    {
      result = params.contains( key );
      release();
    }

    return result;
  }

  bool Manager::read( const std::string_view &key, void *const param )
  {
    bool result = true;

    if ( !initialized || !param || !params.contains( key ) || ( reserve( lockTimeout_mS ) != Chimera::CommonStatusCodes::OK ) )
    {
      result = false;
    }
    else
    {
      auto ctrlBlk = params[ key ];
      auto storage = ( ctrlBlk.config & Location::MEM_LOC_MSK ) >> Location::MEM_LOC_POS;
      auto driver  = memoryDriver[ storage ];
      release();

      if ( driver )
      {
        Chimera::Status_t error = driver->read( ctrlBlk.address, reinterpret_cast<uint8_t *const>( param ), ctrlBlk.size );

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

  bool Manager::write( const std::string_view &key, const void *const param )
  {
    bool result = false;

    if ( initialized && param && params.contains( key ) && ( reserve( lockTimeout_mS ) == Chimera::CommonStatusCodes::OK ) )
    {
      auto ctrlBlk = params[ key ];
      auto storage = ( ctrlBlk.config & Location::MEM_LOC_MSK ) >> Location::MEM_LOC_POS;
      auto driver  = memoryDriver[ storage ];
      release();

      if ( driver )
      {
        Chimera::Status_t error = driver->write( ctrlBlk.address, reinterpret_cast<const uint8_t *const>( param ), ctrlBlk.size );
        result = ( error == Chimera::CommonStatusCodes::OK );
      }
    }

    return result;
  }

  bool Manager::update( const std::string_view &key )
  {
    bool result = false;

    if ( initialized && params.contains( key ) )
    {
      auto ctrlBlk = params[ key ];

      if ( ctrlBlk.update )
      {
        result = ctrlBlk.update( key );
      }
    }

    return result;
  }

  bool Manager::registerMemoryDriver( const uint32_t storage, Chimera::Modules::Memory::Device_sPtr &driver )
  {
    bool result = false;

    if ( initialized && ( storage < Location::MAX_MEMORY_LOCATIONS )
         && ( reserve( lockTimeout_mS ) == Chimera::CommonStatusCodes::OK ) )
    {
      memoryDriver[ storage ] = driver;
      release();
      result = true;
    }

    return result;
  }

  bool Manager::registerMemorySpecs( const uint32_t storage, const Chimera::Modules::Memory::Descriptor &specs )
  {
    bool result = false;

    if ( initialized && ( storage < Location::MAX_MEMORY_LOCATIONS )
         && ( reserve( lockTimeout_mS ) == Chimera::CommonStatusCodes::OK ) )
    {
      memorySpecs[ storage ] = specs;
      release();
      result = true;
    }

    return result;
  }

  const AeroKernel::Parameter::ParamCtrlBlk &Manager::getControlBlock( const std::string_view &key )
  {
    return params[ key ];
  }

}  // namespace AeroKernel::Parameter
