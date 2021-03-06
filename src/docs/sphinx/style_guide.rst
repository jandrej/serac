.. ## Copyright (c) 2019-2020, Lawrence Livermore National Security, LLC and
.. ## other Serac Project Developers. See the top-level COPYRIGHT file for details.
.. ##
.. ## SPDX-License-Identifier: (BSD-3-Clause)

===========
Style Guide
===========

Code Style
----------

This project follows Google's `C++ Style Guide <https://google.github.io/styleguide/cppguide.html>`_
with the following amendments:

    1. ``camelCase`` should be used for function names 
    #. ``ALL_CAPS`` should be used for constants (in addition to macros)

The Google style guide is meant for style enforcement only. The design principles outlined in the 
`C++ Core Guidelines <http://isocpp.github.io/CppCoreGuidelines/>`_ should be followed.

Documentation
-------------

Functions, structs, classes, and critical member variables should be annotated with `Doxygen <https://www.doxygen.nl/manual/>`_ 
comments.  These comments should be enclosed in `Javadoc-style <https://www.doxygen.nl/manual/docblocks.html#cppblock>`_ comment blocks.
For example, a variable can be documented as follows:

::

    /** 
     * The MPI communicator
     */
    MPI_Comm m_comm;

When annotating code, especially functions, Doxygen's `special commands <https://www.doxygen.nl/manual/commands.html>`_ 
can come in handy to provide additional information:

::

    /** 
     * Calculate du_dt = M^-1 (-Ku + f).
     * This is all that is needed for explicit methods
     * @param[in] u The state vector (input to the differentiation)
     * @param[out] du The derivative of \p u with respect to time
     * @see https://mfem.github.io/doxygen/html/classmfem_1_1TimeDependentOperator.html
     */
    virtual void Mult(const mfem::Vector &u, mfem::Vector &du_dt) const;

For non-``void`` functions, the ``@return`` command should be used to describe the return value.

