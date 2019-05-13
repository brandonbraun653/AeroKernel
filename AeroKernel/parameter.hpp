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
    size_t size;      /**< The size of the data this control block describes */
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


  class Manager
  {
  public:
    Manager();
    ~Manager();

    /**
     *
     *
     *	@param[in]	numElements
     *	@return bool
     */
    bool init( const size_t numElements );

    /**
     *
     *
     *	@param[in]	key
     *	@param[in]	controlBlock
     *	@return bool
     */
    bool registerParameter( const std::string_view &key, const ParamCtrlBlk &controlBlock );

    /**
     *
     *
     *	@param[in]	key
     *	@return bool
     */
    bool unregisterParameter( const std::string_view &key );

    /**
     *
     *
     *	@param[in]	key
     *	@return bool
     */
    bool isRegistered( const std::string_view &key );

    /**
     *
     *
     *	@param[in]	key
     *	@param[in]	param
     *	@param[in]	size
     *	@return bool
     */
    bool read( const std::string_view &key, void *const param, const size_t size );

    /**
     *
     *
     *	@param[in]	key
     *	@param[in]	param
     *	@param[in]	size
     *	@return bool
     */
    bool write( const std::string_view &key, const void *const param, const size_t size );

    /**
     *
     *
     *	@param[in]	key
     *	@return bool
     */
    bool update( const std::string_view &key );

    /**
     *
     *
     *	@param[in]	storage
     *	@param[in]	driver
     *	@return bool
     */
    bool registerMemoryDriver( const uint32_t storage, Chimera::Modules::Memory::Device_sPtr &driver );

    /**
     *
     *
     *	@param[in]	storage
     *	@param[in]	specs
     *	@return bool
     */
    bool registerMemorySpecs( const uint32_t storage, const Chimera::Modules::Memory::Descriptor &specs );

    /**
     *
     *
     *	@param[in]	key
     *	@return const AeroKernel::Parameter::ParamCtrlBlk &
     */
    const ParamCtrlBlk &getControlBlock( const std::string_view &key );

  protected:
    bool initialized = false;
    spp::sparse_hash_map<std::string_view, ParamCtrlBlk> params;
    std::array<Chimera::Modules::Memory::Device_sPtr, static_cast<size_t>( Location::MAX_MEMORY_LOCATIONS )> memoryDriver;
    std::array<Chimera::Modules::Memory::Descriptor, static_cast<size_t>( Location::MAX_MEMORY_LOCATIONS )> memorySpecs;
  };


}  // namespace AeroKernel::Parameter

#endif /* !AERO_KERNEL_PARAMETER_MANAGER_HPP */