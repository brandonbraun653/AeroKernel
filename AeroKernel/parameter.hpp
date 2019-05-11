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

#include <cstdint>
#include <string>
#include <memory>

#include <sparsepp/spp.h>

/* Chimera Includes */
#include <Chimera/modules/memory/device.hpp>

namespace AeroKernel::Parameter
{

  enum class MemoryLocation : uint8_t
  {
    INTERNAL_SRAM = 0,
    INTERNAL_FLASH,
    EXTERNAL_FLASH0,
    EXTERNAL_FLASH1,
    EXTERNAL_FLASH2,
    EXTERNAL_SRAM0,
    EXTERNAL_SRAM1,
    EXTERNAL_SRAM2,

    MAX_MEMORY_LOCATIONS
  };

  struct ParamCtrlBlk
  {
    size_t size;
    uint32_t address;
    uint32_t config;
    bool ( *update )( void *const, const size_t );
  };

  using ParamCtrlBlk_sPtr = std::shared_ptr<ParamCtrlBlk>;
  using ParamCtrlBlk_uPtr = std::unique_ptr<ParamCtrlBlk>;


  class Manager
  {
  public:
    Manager();
    ~Manager();

    bool init( const size_t numElements );

    bool registerParameter( const std::string_view &key, const ParamCtrlBlk &controlBlock );

    bool isRegistered( const std::string_view &key );

    bool read( const std::string_view &key, void *const param, const size_t size );

    bool write( const std::string_view &key, const void *const param, const size_t size );

    bool update( const std::string_view &key );

    bool remove( const std::string_view &key );

    bool registerMemoryDriver( const MemoryLocation storage, Chimera::Modules::Memory::Device_sPtr &driver );

    bool registerMemorySpecs( const MemoryLocation storage, const Chimera::Modules::Memory::Descriptor &specs );

    const ParamCtrlBlk &getControlBlock( const std::string_view &key );

  protected:
    bool initialized = false;
    spp::sparse_hash_map<std::string_view, ParamCtrlBlk> params;
    std::array<Chimera::Modules::Memory::Device_sPtr, static_cast<size_t>( MemoryLocation::MAX_MEMORY_LOCATIONS )> memoryDriver;
  };


}  // namespace AeroKernel::Parameter

#endif /* !AERO_KERNEL_PARAMETER_MANAGER_HPP */