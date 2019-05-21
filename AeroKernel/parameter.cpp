/********************************************************************************
 *  File Name:
 *    parameter.cpp
 *
 *  Description:
 *    Implements the AeroKernel Parameter Manager.
 *
 *  2019 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

/* C++ Includes */
#include <type_traits>

#include <AeroKernel/parameter.hpp>

namespace AeroKernel::Parameter
{
  namespace Location
  {
    static constexpr uint8_t MEM_LOC_POS = 0u;                 /**< ParamCtrlBlk.config bit position for memory locator */
    static constexpr uint8_t MEM_LOC_MSK = 0x7 << MEM_LOC_POS; /**< Memory locator config bit width mask */

    static constexpr size_t INVALID              = 0u;
    static constexpr size_t INTERNAL_SRAM        = 1u << MEM_LOC_POS; /**< Location option for internal SRAM */
    static constexpr size_t INTERNAL_FLASH       = 2u << MEM_LOC_POS; /**< Location option for internal FLASH */
    static constexpr size_t EXTERNAL_FLASH0      = 3u << MEM_LOC_POS; /**< Location option for external FLASH #0 */
    static constexpr size_t EXTERNAL_FLASH1      = 4u << MEM_LOC_POS; /**< Location option for external FLASH #1 */
    static constexpr size_t EXTERNAL_FLASH2      = 5u << MEM_LOC_POS; /**< Location option for external FLASH #2 */
    static constexpr size_t EXTERNAL_SRAM0       = 6u << MEM_LOC_POS; /**< Location option for external SRAM #0 */
    static constexpr size_t EXTERNAL_SRAM1       = 7u << MEM_LOC_POS; /**< Location option for external SRAM #1 */
    static constexpr size_t EXTERNAL_SRAM2       = 8u << MEM_LOC_POS; /**< Location option for external SRAM #2 */
    static constexpr size_t MAX_MEMORY_LOCATIONS = 8u;                /**< Total number of memory locations possible */
  };                                                                  // namespace Location

  /*------------------------------------------------
  Compile Time Checks
  ------------------------------------------------*/
  static_assert( static_cast<uint8_t>( Location::MAX_MEMORY_LOCATIONS ) == 8, "Incorrect supported memory locations" );


  Manager::Manager( const size_t lockTimeout_mS ) : initialized( false ), lockTimeout_mS( lockTimeout_mS )
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

  bool Manager::registerParameter( const std::string_view &key, const ControlBlock &controlBlock )
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
      auto storage = ControlBlockInterpreter::getStorage( ctrlBlk );
      auto driver  = memoryDriver[ static_cast<uint8_t>( storage ) ];
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
      auto storage = ControlBlockInterpreter::getStorage( ctrlBlk );
      auto driver  = memoryDriver[ static_cast<uint8_t>( storage ) ];
      release();

      if ( driver )
      {
        Chimera::Status_t error = driver->write( ctrlBlk.address, reinterpret_cast<const uint8_t *const>( param ),
                                                 ctrlBlk.size );
        result                  = ( error == Chimera::CommonStatusCodes::OK );
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
      auto updateFunc = ControlBlockInterpreter::getUpdateCallback( ctrlBlk );

      if ( updateFunc )
      {
        result = updateFunc( key );
      }
    }

    return result;
  }

  bool Manager::registerMemoryDriver( const StorageType storage, Chimera::Modules::Memory::Device_sPtr &driver )
  {
    bool result = false;

    if ( initialized && ( storage != StorageType::NONE )
         && ( reserve( lockTimeout_mS ) == Chimera::CommonStatusCodes::OK ) )
    {
      memoryDriver[ static_cast<uint8_t>(storage) ] = driver;
      release();
      result = true;
    }

    return result;
  }

  bool Manager::registerMemorySpecs( const StorageType storage, const Chimera::Modules::Memory::Descriptor &specs )
  {
    bool result = false;

    if ( initialized && ( storage != StorageType::NONE )
         && ( reserve( lockTimeout_mS ) == Chimera::CommonStatusCodes::OK ) )
    {
      memorySpecs[ static_cast<uint8_t>( storage ) ] = specs;
      release();
      result = true;
    }

    return result;
  }

  const AeroKernel::Parameter::ControlBlock &Manager::getControlBlock( const std::string_view &key )
  {
    return params[ key ];
  }

  AeroKernel::Parameter::ControlBlock ControlBlockFactory::build()
  {
    return mold;
  }

  ControlBlockFactory::ControlBlockFactory()
  {
    clear();
  }

  ControlBlockFactory::~ControlBlockFactory()
  {
  }

  void ControlBlockFactory::clear()
  {
    mold.address = std::numeric_limits<decltype( ControlBlock::address )>::max();
    mold.config  = std::numeric_limits<decltype( ControlBlock::config )>::max();
    mold.size    = std::numeric_limits<decltype( ControlBlock::size )>::min();
    mold.update  = nullptr;
  }

  void ControlBlockFactory::setSize( const size_t size )
  {
    mold.size = size;
  }

  void ControlBlockFactory::setAddress( const size_t address )
  {
    mold.address = address;
  }

  void ControlBlockFactory::setStorage( const StorageType type )
  {
    size_t bitSettings;

    switch ( type )
    {
      case StorageType::INTERNAL_SRAM:
        bitSettings = Location::INTERNAL_SRAM & Location::MEM_LOC_MSK;
        break;

      case StorageType::INTERNAL_FLASH:
        bitSettings = Location::INTERNAL_FLASH & Location::MEM_LOC_MSK;
        break;

      case StorageType::EXTERNAL_FLASH0:
        bitSettings = Location::EXTERNAL_FLASH0 & Location::MEM_LOC_MSK;
        break;

      case StorageType::EXTERNAL_FLASH1:
        bitSettings = Location::EXTERNAL_FLASH1 & Location::MEM_LOC_MSK;
        break;

      case StorageType::EXTERNAL_FLASH2:
        bitSettings = Location::EXTERNAL_FLASH2 & Location::MEM_LOC_MSK;
        break;

      case StorageType::EXTERNAL_SRAM0:
        bitSettings = Location::EXTERNAL_SRAM0 & Location::MEM_LOC_MSK;
        break;

      case StorageType::EXTERNAL_SRAM1:
        bitSettings = Location::EXTERNAL_SRAM1 & Location::MEM_LOC_MSK;
        break;

      case StorageType::EXTERNAL_SRAM2:
        bitSettings = Location::EXTERNAL_SRAM2 & Location::MEM_LOC_MSK;
        break;

      default:
        bitSettings = Location::INVALID & Location::MEM_LOC_MSK;
        break;
    };

    mold.config &= ~( Location::MEM_LOC_MSK );
    mold.config |= bitSettings;
  }

  void ControlBlockFactory::setUpdateCallback( UpdateCallback_t callback )
  {
    mold.update = callback;
  }


  StorageType ControlBlockInterpreter::getStorage( const ControlBlock &ctrlBlk )
  {
    auto setBits = ctrlBlk.config & Location::MEM_LOC_MSK;
    StorageType result = StorageType::NONE;

    switch ( setBits )
    {
      case Location::EXTERNAL_FLASH0:
        result = StorageType::EXTERNAL_FLASH0;
        break;

      case Location::EXTERNAL_FLASH1:
        result = StorageType::EXTERNAL_FLASH1;
        break;

      case Location::EXTERNAL_FLASH2:
        result = StorageType::EXTERNAL_FLASH2;
        break;

      case Location::EXTERNAL_SRAM0:
        result = StorageType::EXTERNAL_SRAM0;
        break;

      case Location::EXTERNAL_SRAM1:
        result = StorageType::EXTERNAL_SRAM1;
        break;

      case Location::EXTERNAL_SRAM2:
        result = StorageType::EXTERNAL_SRAM2;
        break;

      case Location::INTERNAL_FLASH:
        result = StorageType::INTERNAL_FLASH;
        break;

      case Location::INTERNAL_SRAM:
        result = StorageType::INTERNAL_SRAM;
        break;

      default:
        result = StorageType::NONE;
        break;
    };

    return result;
  }

  size_t ControlBlockInterpreter::getAddress( const ControlBlock &ctrlBlk )
  {
    return ctrlBlk.address;
  }

  size_t ControlBlockInterpreter::getSize( const ControlBlock &ctrlBlk )
  {
    return ctrlBlk.size;
  }

  AeroKernel::Parameter::UpdateCallback_t ControlBlockInterpreter::getUpdateCallback( const ControlBlock &ctrlBlk )
  {
    return ctrlBlk.update;
  }

}  // namespace AeroKernel::Parameter
