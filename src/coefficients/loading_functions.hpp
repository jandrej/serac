// Copyright (c) 2019-2020, Lawrence Livermore National Security, LLC and
// other Serac Project Developers. See the top-level LICENSE file for
// details.
//
// SPDX-License-Identifier: (BSD-3-Clause)

#ifndef LOADING_FUNCTIONS
#define LOADING_FUNCTIONS

#include "mfem.hpp"

namespace serac {

// set kinematic functions and boundary condition functions
void referenceConfiguration(const mfem::Vector& x, mfem::Vector& y);
void initialDeformation(const mfem::Vector& x, mfem::Vector& y);

}  // namespace serac

#endif
