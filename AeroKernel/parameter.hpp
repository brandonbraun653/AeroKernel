/********************************************************************************
 *  File Name:
 *    parameter.hpp
 *
 *  Description:
 *    Implements the Aerospace Kernel Parameter Manager. This module allows a
 *    system to pass information around in a thread safe manner without the
 *    producers and consumers knowing implementation details of each other. The
 *    main benefit of this is decoupling of system modules so that different
 *    implementations can be swapped in/out without breaking the code. In its
 *    simplest form, this is just a glorified database.
 *
 *  Usage Example:
 *    An AHRS (Attitude Heading and Reference System) module is producing raw
 *    9-axis data from an IMU (Inertial Measurement Unit) containing gyroscope,
 *    accelerometer, and magnetometer data. Somehow this data needs to be filtered
 *    and transformed into a state estimation of a quadrotor, but the team wants to
 *    try out a couple of different algorithms. The mighty parameter manager is
 *    called upon as a buffer to safely abstract away the AHRS interface so that
 *    they only need to query the registered parameters for their latest data.
 *    The AHRS code will register itself with the Manager as a producer of data
 *    without knowing who will use it, and the state estimation code will consume
 *    the data without knowing who produced it. Decoupling of the two systems has
 *    been achieved! Hurray. Now the software engineers can rest easy knowing they
 *    can swap out the implementation of either side without breaking the code base.
 *
 *  Requirements Documentation:
 *    Repository: https://github.com/brandonbraun653/AeroKernelDev
 *    Location: doc/requirements/parameter_manager.req
 *
 *  2019 | Brandon Braun | brandonbraun653@gmail.com
 ********************************************************************************/

#pragma once
#ifndef AERO_KERNEL_PARAMETER_MANAGER_HPP
#define AERO_KERNEL_PARAMETER_MANAGER_HPP

/* C++ Includes */
#include <cstdint>
#include <string>
#include <memory>
#include <functional>

/* Hash Map Include */
#include <sparsepp/spp.h>

/* Chimera Includes */
#include <Chimera/modules/memory/device.hpp>
#include <Chimera/threading.hpp>

namespace AeroKernel::Parameter
{
  enum class StorageType : uint8_t
  {
    INTERNAL_SRAM,
    INTERNAL_FLASH,
    EXTERNAL_FLASH0,
    EXTERNAL_FLASH1,
    EXTERNAL_FLASH2,
    EXTERNAL_SRAM0,
    EXTERNAL_SRAM1,
    EXTERNAL_SRAM2,
    NONE,
    MAX_STORAGE_OPTIONS = NONE
  };

  using UpdateCallback_t = std::function<bool( const std::string_view &key )>;

  /**
   *  Data structure that fully describes a parameter that is stored
   *  somewhere in memory. This could be volatile or non-volatile
   *  memory, it does not matter. The actual data is not stored in
   *  this block, only the meta information describing it.
   *
   *  @requirement PM002.2
   */
  struct ControlBlock
  {
    /**
     *  The size of the data this control block describes.
     */
    size_t size = std::numeric_limits<size_t>::max();

    /**
     *  The address in memory the data should be stored at. Whether
     *  or not the address is valid is highly dependent upon the
     *  storage sink used.
     */
    size_t address = std::numeric_limits<size_t>::max();

    /**
     *  Configuration Options:
     *    Bits 0-2: Memory Storage Location, see MemoryLocation
     *
     *  @requirement PM002.2.1, PM002.2.2, PM002.2.3
     */
    size_t config = std::numeric_limits<size_t>::max();

    /**
     *  Optional function that can be used by client applications
     *  to request an update of the parameter. This allows fresh
     *  data to be acquired on demand.
     *
     *  @requirement PM002.3
     */
    UpdateCallback_t update = nullptr;
  };

  using ParamCtrlBlk_sPtr = std::shared_ptr<ControlBlock>;
  using ParamCtrlBlk_uPtr = std::unique_ptr<ControlBlock>;

  /**
   *  A generator for the control block data structure. Currently
   *  it's quite simple, but the data type is likely to change in 
   *  the future and necessitates a common interface.
   */
  class ControlBlockFactory
  {
  public:
    ControlBlockFactory();
    ~ControlBlockFactory();

    /**
     *	Compiles all the current settings and returns the fully
     *  configured control block.
     *
     *	@return AeroKernel::Parameter::ControlBlock
     */
    ControlBlock build();

    /**
     *	Clears all current settings and resets the factory to default
     *
     *	@return void
     */
    void clear();

    /**
     *	Encodes the sizing information associated with the parameter this
     *  control block describes.
     *
     *	@param[in]	size      The size of the parameter
     *	@return void
     */
    void setSize( const size_t size );

    /**
     *	Encodes the address information
     *
     *	@param[in]	address   The address the parameter will be stored at in NVM
     *	@return void
     */
    void setAddress( const size_t address );

    /**
     *	Encodes the storage device for the actual parameter data
     *
     *	@param[in]	type      Where the parameter data will be stored
     *	@return void
     */
    void setStorage( const StorageType type );

    /**
     *	Attaches an optional update function
     *
     *	@param[in]	callback  The update function to attach
     *	@return void
     */
    void setUpdateCallback( UpdateCallback_t callback );

  private:
    ControlBlock mold;
  };

  /**
   *  A static class that interprets the control block configuration
   *  and can return back non-encoded data. Currently this is just a 
   *  simple wrapper, but the control block data structure may change
   *  in the future, necessitating a common interface.
   */
  class ControlBlockInterpreter
  {
  public:
    ControlBlockInterpreter() = default;
    ~ControlBlockInterpreter() = default;

    static StorageType getStorage( const ControlBlock &ctrlBlk );

    static size_t getAddress( const ControlBlock &ctrlBlk );

    static size_t getSize( const ControlBlock &ctrlBlk );

    static UpdateCallback_t getUpdateCallback( const ControlBlock &ctrlBlk );
  };

  /**
   *  Parameter Manager Implementation
   */
  class Manager : public Chimera::Threading::Lockable
  {
  public:
    /**
     *	Initialize the parameter manager instance
     *
     *	@param[in]	lockTimeout_mS  How long to wait for the manager to be available
     *  @return Manager
     */
    Manager( const size_t lockTimeout_mS = 50 );
    ~Manager();

    /**
     *  Initializes the parameter manager to a default configuration and allocates
     *  the given number of parameters that can be actively registered. Ideally this
     *  is only performed once at startup and should not be called again to avoid
     *  dynamic memory allocation. If your system can handle that, then go wild.
     *
     *  @requirement PM001
     *
     *	@param[in]	numParameters   How many parameters can be managed by this class
     *	@return bool
     */
    bool init( const size_t numParameters );

    /**
     *  Registers a new parameter into the manager
     *
     *  @requirement PM002, PM002.1
     *
     *	@param[in]	key             The parameter's name
     *	@param[in]	controlBlock    Information describing where the parameter lives in memory
     *	@return bool
     */
    bool registerParameter( const std::string_view &key, const ControlBlock &controlBlock );

    /**
     *  Removes a parameter from the manager
     *
     *  @requirement PM006
     *
     *	@param[in]	key             The parameter's name
     *	@return bool
     */
    bool unregisterParameter( const std::string_view &key );

    /**
     *  Checks if the given parameter has been registered
     *
     *  @requirement PM003
     *
     *	@param[in]	key             The parameter's name
     *	@return bool
     */
    bool isRegistered( const std::string_view &key );

    /**
     *  Read the parameter data from wherever it has been stored
     *
     *  @requirement PM004
     *
     *	@param[in]	key             The parameter's name
     *	@param[in]	param           Where to place the read data
     *	@return bool
     */
    bool read( const std::string_view &key, void *const param );

    /**
     *  Write the parameter data to wherever it is stored
     *
     *  @requirement PM005
     *
     *	@param[in]	key             The parameter's name
     *	@param[in]	param           Where to write data from
     *	@return bool
     */
    bool write( const std::string_view &key, const void *const param );

    /**
     *  If registered, executes the parameter's update method
     *
     *  @requirement PM0011
     *
     *	@param[in]	key             The parameter's name
     *	@return bool
     */
    bool update( const std::string_view &key );

    /**
     *  Registers a memory sink with the manager backend
     *
     *  @requirement PM010
     *
     *	@param[in]	storage         The type of storage the driver represents as defined in the Location namespace
     *	@param[in]	driver          A fully configured instance of a memory driver
     *	@return bool
     */
    bool registerMemoryDriver( const StorageType storage, Chimera::Modules::Memory::Device_sPtr &driver );

    /**
     *  Allows the user to assign virtual memory specifications to a
     *  registered memory driver. This allows for partitioning the regions
     *  that the Parameter manager is allowed access to.
     *
     *  @requirement PM009
     *
     *	@param[in]	storage         The type of storage the driver represents as defined in the Location namespace
     *	@param[in]	specs           Memory configuration specs
     *	@return bool
     */
    bool registerMemorySpecs( const StorageType storage, const Chimera::Modules::Memory::Descriptor &specs );

    /**
     *  Gets the control block associated with a given parameter
     *
     *  @requirement PM012
     *
     *	@param[in]	key             The parameter's name
     *	@return const AeroKernel::Parameter::ParamCtrlBlk &
     */
    const ControlBlock &getControlBlock( const std::string_view &key );

  protected:
    bool initialized;
    size_t lockTimeout_mS;
    spp::sparse_hash_map<std::string_view, ControlBlock> params;
    std::array<Chimera::Modules::Memory::Device_sPtr, static_cast<size_t>( StorageType::MAX_STORAGE_OPTIONS )> memoryDriver;
    std::array<Chimera::Modules::Memory::Descriptor, static_cast<size_t>( StorageType::MAX_STORAGE_OPTIONS )> memorySpecs;
  };

  using Manager_sPtr = std::shared_ptr<Manager>;
  using Manager_uPtr = std::unique_ptr<Manager>;

}  // namespace AeroKernel::Parameter

#endif /* !AERO_KERNEL_PARAMETER_MANAGER_HPP */