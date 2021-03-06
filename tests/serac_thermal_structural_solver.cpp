// Copyright (c) 2019-2020, Lawrence Livermore National Security, LLC and
// other Serac Project Developers. See the top-level LICENSE file for
// details.
//
// SPDX-License-Identifier: (BSD-3-Clause)

#include <gtest/gtest.h>

#include <fstream>

#include "coefficients/stdfunction_coefficient.hpp"
#include "common/mesh_utils.hpp"
#include "mfem.hpp"
#include "serac_config.hpp"
#include "solvers/thermal_structural_solver.hpp"

namespace serac {

TEST(dynamic_solver, dyn_solve)
{
  MPI_Barrier(MPI_COMM_WORLD);

  // Open the mesh
  std::string mesh_file = std::string(SERAC_REPO_DIR) + "/data/beam-hex.mesh";

  auto pmesh = buildParallelMesh(mesh_file, 1, 0);

  int dim = pmesh->Dimension();

  // define a boundary attribute set
  std::set<int> ess_bdr = {1};

  auto deform = std::make_shared<StdFunctionVectorCoefficient>(dim, [](mfem::Vector& x, mfem::Vector& y) {
    y    = x;
    y(1) = y(1) + x(0) * 0.01;
  });

  auto velo = std::make_shared<StdFunctionVectorCoefficient>(dim, [](mfem::Vector&, mfem::Vector& v) { v = 0.0; });

  auto temp = std::make_shared<StdFunctionCoefficient>([](mfem::Vector& x) {
    double temp = 2.0;
    if (x(0) < 1.0) {
      temp = 5.0;
    }
    return temp;
  });

  auto kappa = std::make_shared<mfem::ConstantCoefficient>(0.5);

  // set the traction boundary
  std::set<int> trac_bdr = {2};

  // define the traction vector
  mfem::Vector traction(dim);
  traction           = 0.0;
  traction(1)        = 1.0e-3;
  auto traction_coef = std::make_shared<mfem::VectorConstantCoefficient>(traction);

  // initialize the dynamic solver object
  ThermalStructuralSolver ts_solver(1, pmesh);
  ts_solver.SetDisplacementBCs(ess_bdr, deform);
  ts_solver.SetTractionBCs(trac_bdr, traction_coef);
  ts_solver.SetHyperelasticMaterialParameters(0.25, 5.0);
  ts_solver.SetConductivity(kappa);
  ts_solver.SetDisplacement(*deform);
  ts_solver.SetVelocity(*velo);
  ts_solver.SetTemperature(*temp);
  ts_solver.setTimestepper(serac::TimestepMethod::SDIRK33);
  ts_solver.SetCouplingScheme(serac::CouplingScheme::OperatorSplit);

  // Make a temperature-dependent viscosity
  double offset = 0.1;
  double scale  = 1.0;

  auto temp_gf_coef = std::make_shared<mfem::GridFunctionCoefficient>(ts_solver.GetTemperature()->gf.get());
  auto visc_coef    = std::make_shared<TransformedScalarCoefficient>(
      temp_gf_coef, [offset, scale](const double x) { return scale * x + offset; });
  ts_solver.SetViscosity(visc_coef);

  // Set the linear solver parameters
  serac::LinearSolverParameters params;
  params.prec        = serac::Preconditioner::BoomerAMG;
  params.abs_tol     = 1.0e-8;
  params.rel_tol     = 1.0e-4;
  params.max_iter    = 500;
  params.lin_solver  = serac::LinearSolver::GMRES;
  params.print_level = 0;

  // Set the nonlinear solver parameters
  serac::NonlinearSolverParameters nl_params;
  nl_params.rel_tol     = 1.0e-4;
  nl_params.abs_tol     = 1.0e-8;
  nl_params.print_level = 1;
  nl_params.max_iter    = 500;
  ts_solver.SetSolidSolverParameters(params, nl_params);
  ts_solver.SetThermalSolverParameters(params);

  // Initialize the VisIt output
  ts_solver.initializeOutput(serac::OutputType::VisIt, "dynamic_thermal_solid");

  // Construct the internal dynamic solver data structures
  ts_solver.completeSetup();

  double t       = 0.0;
  double t_final = 6.0;
  double dt      = 1.0;

  // Ouput the initial state
  ts_solver.outputState();

  // Perform time-integration
  // (looping over the time iterations, ti, with a time-step dt).
  bool last_step = false;
  for (int ti = 1; !last_step; ti++) {
    double dt_real = std::min(dt, t_final - t);
    t += dt_real;
    last_step = (t >= t_final - 1e-8 * dt);

    ts_solver.advanceTimestep(dt_real);
  }

  // Output the final state
  ts_solver.outputState();

  // Check the final displacement and velocity L2 norms
  mfem::Vector zero(dim);
  zero = 0.0;
  mfem::VectorConstantCoefficient zerovec(zero);

  double v_norm    = ts_solver.GetVelocity()->gf->ComputeLpError(2.0, zerovec);
  double x_norm    = ts_solver.GetDisplacement()->gf->ComputeLpError(2.0, zerovec);
  double temp_norm = ts_solver.GetTemperature()->gf->ComputeLpError(2.0, zerovec);

  EXPECT_NEAR(13.28049, x_norm, 0.001);
  EXPECT_NEAR(0.005227, v_norm, 0.001);
  EXPECT_NEAR(6.491872, temp_norm, 0.001);

  MPI_Barrier(MPI_COMM_WORLD);
}

}  // namespace serac

int main(int argc, char* argv[])
{
  int result = 0;

  ::testing::InitGoogleTest(&argc, argv);

  MPI_Init(&argc, &argv);

  result = RUN_ALL_TESTS();

  MPI_Finalize();

  return result;
}
