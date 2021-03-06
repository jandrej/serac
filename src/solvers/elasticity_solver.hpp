// Copyright (c) 2019-2020, Lawrence Livermore National Security, LLC and
// other Serac Project Developers. See the top-level LICENSE file for
// details.
//
// SPDX-License-Identifier: (BSD-3-Clause)

#ifndef LINEARELASTIC_SOLVER
#define LINEARELASTIC_SOLVER

#include "base_solver.hpp"
#include "mfem.hpp"

namespace serac {

/** This is a generic linear elasticity oeprator of the form
 *
 *    -div(sigma(u)) = f
 *    sigma(u) = lambda div(u) + mu(grad(u) + grad(u)^T
 *
 *  where u is the displacement vector, f is the body force,
 *  and lambda and mu are the lame parameters */
class ElasticitySolver : public BaseSolver {
 protected:
  std::shared_ptr<serac::FiniteElementState> displacement_;

  /// Stiffness bilinear form
  std::unique_ptr<mfem::ParBilinearForm> K_form_;

  /// Load bilinear form
  std::unique_ptr<mfem::ParLinearForm> l_form_;

  /// Stiffness matrix
  std::unique_ptr<mfem::HypreParMatrix> K_mat_;

  /// Eliminated stiffness matrix
  std::unique_ptr<mfem::HypreParMatrix> K_e_mat_;

  /// RHS vector
  std::unique_ptr<mfem::HypreParVector> rhs_;

  /// Eliminated RHS vector
  std::unique_ptr<mfem::HypreParVector> bc_rhs_;

  /// Solver for the stiffness matrix
  std::unique_ptr<mfem::Solver> K_solver_;

  /// Preconditioner for the stiffness
  std::unique_ptr<mfem::Solver> K_prec_;

  /// Lame mu parameter coefficient
  mfem::Coefficient* mu_ = nullptr;

  /// Lame lambda parameter coefficient
  mfem::Coefficient* lambda_ = nullptr;

  /// Body source coefficient
  mfem::VectorCoefficient* body_force_ = nullptr;

  /// Linear solver parameters
  serac::LinearSolverParameters lin_params_;

  /// Driver for a quasi-static solve
  void QuasiStaticSolve();

 public:
  /// Constructor using order and mesh
  ElasticitySolver(const int order, std::shared_ptr<mfem::ParMesh> pmesh);

  /// Set the vector-valued essential displacement boundary conditions
  void setDisplacementBCs(const std::set<int>& disp_bdr, std::shared_ptr<mfem::VectorCoefficient> disp_bdr_coef,
                          const int component = -1);

  /// Set the vector-valued natural traction boundary conditions
  void setTractionBCs(const std::set<int>& trac_bdr, std::shared_ptr<mfem::VectorCoefficient> trac_bdr_coef,
                      const int component = -1);

  /// Driver for advancing the timestep
  void advanceTimestep(double& dt) override;

  /// Set the elastic lame parameters
  void setLameParameters(mfem::Coefficient& lambda, mfem::Coefficient& mu);

  /// Set the vector-valued body force coefficient
  void setBodyForce(mfem::VectorCoefficient& force);

  /// Finish the setup and allocate the associate data structures
  void completeSetup() override;

  /// Set the linear solver parameters object
  void setLinearSolverParameters(const serac::LinearSolverParameters& params);

  /// The destructor
  virtual ~ElasticitySolver();
};

}  // namespace serac

#endif
