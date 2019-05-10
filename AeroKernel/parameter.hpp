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

namespace AeroKernel::Parameter
{
  class Manager
  {
  public:

    Manager();
    ~Manager();

    void registerParameter();

    void isRegistered();

    void read();
    
    void write();

    void update();

    void remove();

    // Driver needs to be lockable...is it? Maybe it should be a compile time check...
    void registerMemoryDriver(/*MemLocation, SharedPtr to Chimera MemDevice*/); 

  };
}

#endif /* !AERO_KERNEL_PARAMETER_MANAGER_HPP */