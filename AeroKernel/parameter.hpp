/********************************************************************************
 *  File Name:
 *    parameter.hpp
 *
 *  Description:
 *    Implements the Aero Kernel Parameter Manager
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
  namespace Location
  {
    static constexpr uint8_t MEM_LOC_POS = 0u;
    static constexpr uint8_t MEM_LOC_MSK = 0x7 << MEM_LOC_POS;

    static constexpr size_t INTERNAL_SRAM   = 0u << MEM_LOC_POS;
    static constexpr size_t INTERNAL_FLASH  = 1u << MEM_LOC_POS;
    static constexpr size_t EXTERNAL_FLASH0 = 2u << MEM_LOC_POS;
    static constexpr size_t EXTERNAL_FLASH1 = 3u << MEM_LOC_POS;
    static constexpr size_t EXTERNAL_FLASH2 = 4u << MEM_LOC_POS;
    static constexpr size_t EXTERNAL_SRAM0  = 5u << MEM_LOC_POS;
    static constexpr size_t EXTERNAL_SRAM1  = 6u << MEM_LOC_POS;
    static constexpr size_t EXTERNAL_SRAM2  = 7u << MEM_LOC_POS;

    static constexpr size_t MAX_MEMORY_LOCATIONS = 8u;
  };  // namespace Location

  struct ParamCtrlBlk
  {
    size_t size;    /**< The size of the data this control block describes */
    size_t address; /**< The address in memory the data should be stored at */

    /**
     *  Configuration Options:
     *    Bits 0-2: Memory Storage Location, see MemoryLocation
     */
    size_t config;
    std::function<bool( const std::string_view &key )> update;
  };

  using ParamCtrlBlk_sPtr = std::shared_ptr<ParamCtrlBlk>;
  using ParamCtrlBlk_uPtr = std::unique_ptr<ParamCtrlBlk>;


  class Manager : public Chimera::Threading::Lockable 
  {
  public:
    Manager(const size_t lockTimeout_mS = 50 );
    ~Manager();

    /**
     *  Initializes the parameter manager to a default configuration and allocates
     *  the given number of parameters that can be actively registered.
     *
     *	@param[in]	numParameters   How many parameters can be managed by this class
     *	@return bool
     */
    bool init( const size_t numParameters );

    /**
     *  Registers a new parameter into the manager
     *
     *	@param[in]	key             The parameter's name
     *	@param[in]	controlBlock    Information describing where the parameter lives in memory
     *	@return bool
     */
    bool registerParameter( const std::string_view &key, const ParamCtrlBlk &controlBlock );

    /**
     *  Removes a parameter from the manager
     *
     *	@param[in]	key             The parameter's name
     *	@return bool
     */
    bool unregisterParameter( const std::string_view &key );

    /**
     *  Checks if the given parameter has been registered
     *
     *	@param[in]	key             The parameter's name
     *	@return bool
     */
    bool isRegistered( const std::string_view &key );

    /**
     *  Read the parameter data from wherever it has been stored
     *
     *	@param[in]	key             The parameter's name
     *	@param[in]	param           Where to place the read data
     *	@return bool
     */
    bool read( const std::string_view &key, void *const param );

    /**
     *  Write the parameter data to wherever it is stored
     *
     *	@param[in]	key             The parameter's name
     *	@param[in]	param           Where to write data from
     *	@return bool
     */
    bool write( const std::string_view &key, const void *const param );

    /**
     *  If registered, executes the parameter's update method
     *
     *	@param[in]	key             The parameter's name
     *	@return bool
     */
    bool update( const std::string_view &key );

    /**
     *  Registers a memory sink with the manager backend
     *
     *	@param[in]	storage         The type of storage the driver represents as defined in the Location namespace
     *	@param[in]	driver          A fully configured instance of a memory driver
     *	@return bool
     */
    bool registerMemoryDriver( const uint32_t storage, Chimera::Modules::Memory::Device_sPtr &driver );

    /**
     *  Allows the user to assign virtual memory specifications to a
     *  registered memory driver. This allows for partitioning the regions
     *  that the Parameter manager is allowed access to.
     *
     *	@param[in]	storage         The type of storage the driver represents as defined in the Location namespace
     *	@param[in]	specs           Memory configuration specs
     *	@return bool
     */
    bool registerMemorySpecs( const uint32_t storage, const Chimera::Modules::Memory::Descriptor &specs );

    /**
     *  Gets the control block associated with a given parameter
     *
     *	@param[in]	key             The parameter's name
     *	@return const AeroKernel::Parameter::ParamCtrlBlk &
     */
    const ParamCtrlBlk &getControlBlock( const std::string_view &key );

  protected:
    bool initialized;
    size_t lockTimeout_mS;
    spp::sparse_hash_map<std::string_view, ParamCtrlBlk> params;
    std::array<Chimera::Modules::Memory::Device_sPtr, static_cast<size_t>( Location::MAX_MEMORY_LOCATIONS )> memoryDriver;
    std::array<Chimera::Modules::Memory::Descriptor, static_cast<size_t>( Location::MAX_MEMORY_LOCATIONS )> memorySpecs;
  };


}  // namespace AeroKernel::Parameter

#endif /* !AERO_KERNEL_PARAMETER_MANAGER_HPP */