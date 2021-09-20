# ablate::parameters::Parameters
## ablate::parameters::FactoryParameters*
Creates a parameter list based upon a factory.  Should be default for factory parsing.

# ablate::mathFunctions::MathFunction
## ablate::mathFunctions::ParsedFunction*
a string based function to be parsed with muparser. The (string) formula that may accept x, y, z, t as variables

## ablate::mathFunctions::ParsedSeries
 computes a series result from a string function with variables x, y, z, t, and i where i index of summation. $$\sum_{i = m}^n formula(x, y, z, t, n)$$

formula (req) 
: (string) see ParsedFunction for details on the string formatting.

lowerBound (req) 
: (int) the inclusive lower bound of summation (m)

upperBound (req) 
: (int) the inclusive upper bound of summation (n)

## ablate::mathFunctions::LinearTable
A table that is built from a spreadsheet that allows linear interpolation of variables based on monotonically increasing independent variables

file (req) 
: (file path or url) a file with csv data and header

independent (req) 
: (string) the name of the independent column name as defined in the header

dependent (req) 
: (string list) the names of the dependent column in the order in which to apply them

mappingFunction (req) 
: (ablate::mathFunctions::MathFunction)  the function that maps from the physical x,y,z, and t space to the table independent variable

## ablate::mathFunctions::ParsedNested
 computes string function with variables x, y, z, and t where additional variables can be specified using other functions

formula (req) 
: (string) see ParsedFunction for details on the string formatting.

nested (req) 
: (std::map<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, ablate::mathFunctions::MathFunction, std::less<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const, ablate::mathFunctions::MathFunction> > >) a map of nested MathFunctions.  These functions are assumed to compute a single scalar value

# ablate::mathFunctions::FieldFunction
## ablate::flow::fieldFunctions::Euler
initializes the euler conserved field variables based upon a CompressibleFlowState

state (req) 
: (ablate::flow::fieldFunctions::CompressibleFlowState) The CompressibleFlowState used to initialize

## ablate::flow::fieldFunctions::MassFractions
initializes the yi field function variables based upon a the list of functions and eos

eos (req) 
: (ablate::eos::EOS) The eos with the list of species

values (req) 
: (std::vector<ablate::mathFunctions::FieldFunction, std::allocator<ablate::mathFunctions::FieldFunction> >) The list of mass fraction functions

## ablate::flow::fieldFunctions::DensityMassFractions
initializes the densityYi conserved field variables based upon a CompressibleFlowState

state (req) 
: (ablate::flow::fieldFunctions::CompressibleFlowState) The CompressibleFlowState used to initialize

## ablate::mathFunctions::FieldFunction*
a field description that can be used for initialization or exact solution 

fieldName (req) 
: (string) the field name

field (req) 
: (ablate::mathFunctions::MathFunction) the math function used to describe the field

timeDerivative
: (ablate::mathFunctions::MathFunction) the math function used to describe the field time derivative

# ablate::io::Serializer
## ablate::io::Hdf5Serializer*
default serializer for IO

interval (req) 
: (ablate::io::interval::Interval) The interval object used to determine write interval.

# ablate::io::interval::Interval
## ablate::io::interval::FixedInterval*
Default interval that outputs every n steps

## ablate::io::interval::SimulationTimeInterval
Outputs every dt simulation seconds. This will not result in uniform output unless the simulation dt matches.

## ablate::io::interval::WallTimeInterval
Outputs approximately every n wall time seconds

# ablate::eos::EOS
## ablate::eos::PerfectGas
perfect gas eos

parameters (req) 
: (ablate::parameters::Parameters) parameters for the perfect gas eos

species
: (string list) species to track.  Note: species mass fractions do not change eos

## ablate::eos::TChem
TChem ideal gas eos

mechFile (req) 
: (file path or url) the mech file (CHEMKIN Format)

thermoFile (req) 
: (file path or url) the thermo file (CHEMKIN Format)

# ablate::eos::transport::TransportModel
## ablate::eos::transport::Constant*
constant value transport model (often used for testing)

k
: (double) thermal conductivity [W/(m K)]

mu
: (double) viscosity [Pa s]

diff
: (double) diffusivity [m2/s]

## ablate::eos::transport::Sutherland
Sutherland Transport model

eos (req) 
: (ablate::eos::EOS) The EOS used to compute Cp (needed for Conductivity)

# ablate::flow::fieldFunctions::CompressibleFlowState
## ablate::flow::fieldFunctions::CompressibleFlowState*
a simple structure used to describe a compressible flow field using an EOS, T, pressure, vel, Yi

eos (req) 
: (ablate::eos::EOS) the eos used for the flow field

temperature (req) 
: (ablate::mathFunctions::MathFunction) the temperature field (K)

pressure (req) 
: (ablate::mathFunctions::MathFunction) the pressure field (Pa)

velocity (req) 
: (ablate::mathFunctions::MathFunction) the velocity field (m/2)

massFractions
: (ablate::mathFunctions::FieldFunction) a fieldFunctions used to describe all mass fractions

# ablate::flow::fluxCalculator::FluxCalculator
## ablate::flow::fluxCalculator::Ausm
AUSM Flux Spliting: "A New Flux Splitting Scheme" Liou and Steffen, pg 26, Eqn (6), 1993

## ablate::flow::fluxCalculator::AverageFlux
Takes the average of the left/right faces.  Only useful for debugging.

## ablate::flow::fluxCalculator::OffFlux
Turns of convective flux through the face.

## ablate::flow::fluxCalculator::AusmpUp
A sequel to AUSM, Part II: AUSM+-up for all speeds, Meng-Sing Liou, Pages 137-170, 2006

mInf (req) 
: (double) the reference mach number

# ablate::flow::boundaryConditions::BoundaryCondition
## ablate::flow::boundaryConditions::Essential
essential (Dirichlet condition) for FE based problems

boundaryName (req) 
: (string) the name for this boundary condition

labelIds (req) 
: (int list) the ids on the mesh to apply the boundary condition

boundaryValue (req) 
: (ablate::mathFunctions::FieldFunction) the field function used to describe the boundary

labelName
: (string) the mesh label holding the boundary ids (default marker)

## ablate::flow::boundaryConditions::EssentialGhost
essential (Dirichlet condition) for ghost cell based boundaries

boundaryName (req) 
: (string) the name for this boundary condition

labelIds (req) 
: (int list) the ids on the mesh to apply the boundary condition

boundaryValue (req) 
: (ablate::mathFunctions::FieldFunction) the field function used to describe the boundary

labelName
: (string) the mesh label holding the boundary ids (default Face Sets)

# ablate::flow::FlowFieldDescriptor
## ablate::flow::FlowFieldDescriptor*
flow field description

# ablate::flow::Flow
## ablate::flow::LowMachFlow
incompressible FE flow

name (req) 
: (string) the name of the flow field

mesh (req) 
: (ablate::mesh::Mesh) the mesh

parameters (req) 
: (ablate::parameters::Parameters) the flow field parameters

options
: (ablate::parameters::Parameters) options for the flow passed directly to PETSc

initialization (req) 
: (std::vector<ablate::mathFunctions::FieldFunction, std::allocator<ablate::mathFunctions::FieldFunction> >) the solution used to initialize the flow field

boundaryConditions (req) 
: (std::vector<ablate::flow::boundaryConditions::BoundaryCondition, std::allocator<ablate::flow::boundaryConditions::BoundaryCondition> >) the boundary conditions for the flow field

auxFields (req) 
: (std::vector<ablate::mathFunctions::FieldFunction, std::allocator<ablate::mathFunctions::FieldFunction> >) enables and sets the update functions for the auxFields

exactSolution
: (std::vector<ablate::mathFunctions::FieldFunction, std::allocator<ablate::mathFunctions::FieldFunction> >) optional exact solutions that can be used for error calculations

## ablate::flow::IncompressibleFlow
incompressible FE flow

name (req) 
: (string) the name of the flow field

mesh (req) 
: (ablate::mesh::Mesh) the mesh

parameters (req) 
: (ablate::parameters::Parameters) the flow field parameters

options
: (ablate::parameters::Parameters) options for the flow passed directly to PETSc

initialization (req) 
: (std::vector<ablate::mathFunctions::FieldFunction, std::allocator<ablate::mathFunctions::FieldFunction> >) the solution used to initialize the flow field

boundaryConditions (req) 
: (std::vector<ablate::flow::boundaryConditions::BoundaryCondition, std::allocator<ablate::flow::boundaryConditions::BoundaryCondition> >) the boundary conditions for the flow field

auxFields
: (std::vector<ablate::mathFunctions::FieldFunction, std::allocator<ablate::mathFunctions::FieldFunction> >) enables and sets the update functions for the auxFields

exactSolution
: (std::vector<ablate::mathFunctions::FieldFunction, std::allocator<ablate::mathFunctions::FieldFunction> >) optional exact solutions that can be used for error calculations

## ablate::flow::FVFlow
finite volume flow

name (req) 
: (string) the name of the flow field

mesh (req) 
: (ablate::mesh::Mesh) the  mesh and discretization

parameters
: (ablate::parameters::Parameters) the parameters used by the flow

fields (req) 
: (std::vector<ablate::flow::FlowFieldDescriptor, std::allocator<ablate::flow::FlowFieldDescriptor> >) field descriptions

processes (req) 
: (std::vector<ablate::flow::processes::FlowProcess, std::allocator<ablate::flow::processes::FlowProcess> >) the processes used to describe the flow

options
: (ablate::parameters::Parameters) the options passed to PETSC for the flow

initialization
: (std::vector<ablate::mathFunctions::FieldFunction, std::allocator<ablate::mathFunctions::FieldFunction> >) the flow field initialization

boundaryConditions
: (std::vector<ablate::flow::boundaryConditions::BoundaryCondition, std::allocator<ablate::flow::boundaryConditions::BoundaryCondition> >) the boundary conditions for the flow field

auxiliaryFields
: (std::vector<ablate::mathFunctions::FieldFunction, std::allocator<ablate::mathFunctions::FieldFunction> >) the aux flow field initialization

exactSolution
: (std::vector<ablate::mathFunctions::FieldFunction, std::allocator<ablate::mathFunctions::FieldFunction> >) optional exact solutions that can be used for error calculations

## ablate::flow::CompressibleFlow
compressible finite volume flow

name (req) 
: (string) the name of the flow field

mesh (req) 
: (ablate::mesh::Mesh) the  mesh and discretization

eos (req) 
: (ablate::eos::EOS) the equation of state used to describe the flow

parameters (req) 
: (ablate::parameters::Parameters) the compressible flow parameters cfl, gamma, etc.

transport
: (ablate::eos::transport::TransportModel) the diffusion transport model

fluxCalculator
: (ablate::flow::fluxCalculator::FluxCalculator) the flux calculators (defaults to AUSM)

options
: (ablate::parameters::Parameters) the options passed to PETSc

initialization
: (std::vector<ablate::mathFunctions::FieldFunction, std::allocator<ablate::mathFunctions::FieldFunction> >) the flow field initialization

boundaryConditions
: (std::vector<ablate::flow::boundaryConditions::BoundaryCondition, std::allocator<ablate::flow::boundaryConditions::BoundaryCondition> >) the boundary conditions for the flow field

exactSolution
: (std::vector<ablate::mathFunctions::FieldFunction, std::allocator<ablate::mathFunctions::FieldFunction> >) optional exact solutions that can be used for error calculations

## ablate::flow::ReactingCompressibleFlow
reacting compressible finite volume flow

name (req) 
: (string) the name of the flow field

mesh (req) 
: (ablate::mesh::Mesh) the  mesh and discretization

eos (req) 
: (ablate::eos::EOS) the TChem v1 equation of state used to describe the flow

parameters (req) 
: (ablate::parameters::Parameters) the compressible flow parameters cfl, gamma, etc.

transport
: (ablate::eos::transport::TransportModel) the diffusion transport model

fluxCalculator
: (ablate::flow::fluxCalculator::FluxCalculator) the flux calculator (defaults to AUSM)

options
: (ablate::parameters::Parameters) the options passed to PETSc

initialization
: (std::vector<ablate::mathFunctions::FieldFunction, std::allocator<ablate::mathFunctions::FieldFunction> >) the flow field initialization

boundaryConditions
: (std::vector<ablate::flow::boundaryConditions::BoundaryCondition, std::allocator<ablate::flow::boundaryConditions::BoundaryCondition> >) the boundary conditions for the flow field

exactSolution
: (std::vector<ablate::mathFunctions::FieldFunction, std::allocator<ablate::mathFunctions::FieldFunction> >) optional exact solutions that can be used for error calculations

# ablate::flow::processes::FlowProcess
## ablate::flow::processes::EulerAdvection
build advection for the euler field and species

parameters
: (ablate::parameters::Parameters) the parameters used by advection

eos (req) 
: (ablate::eos::EOS) the equation of state used to describe the flow

fluxCalculator
: (ablate::flow::fluxCalculator::FluxCalculator) the flux calculator (defaults to AUSM)

## ablate::flow::processes::EulerDiffusion
diffusion for the euler field

eos (req) 
: (ablate::eos::EOS) the equation of state used to describe the flow

parameters
: (ablate::eos::transport::TransportModel) the diffusion transport model

## ablate::flow::processes::TChemReactions
reactions using the TChem v1 library

eos (req) 
: (ablate::eos::EOS) the tChem v1 eos

options
: (ablate::parameters::Parameters) any PETSc options for the chemistry ts

## ablate::flow::processes::SpeciesDiffusion
diffusion for the species yi field

eos (req) 
: (ablate::eos::EOS) the equation of state used to describe the flow

parameters
: (ablate::eos::transport::TransportModel) the diffusion transport model

# ablate::mesh::Mesh
## ablate::mesh::BoxMesh
simple uniform box mesh

name (req) 
: (string) the name of the domain/mesh object

faces (req) 
: (int list) the number of faces in each direction

lower (req) 
: (double list) the lower bound of the mesh

upper (req) 
: (double list) the upper bound of the mesh

boundary
: (string list) custom boundary types (NONE, GHOSTED, MIRROR, PERIODIC)

simplex
: (bool) sets if the elements/cells are simplex

options
: (ablate::parameters::Parameters) any PETSc options

## ablate::mesh::DMPlex
DMPlex that can be set using PETSc options

name (req) 
: (string) the mesh dm name

options
: (ablate::parameters::Parameters) options used to setup the DMPlex

## ablate::mesh::FileMesh
read a DMPlex from a file

name (req) 
: (string) the name of the domain/mesh object

path (req) 
: (file path or url) the path to the mesh file

options
: (ablate::parameters::Parameters) any PETSc options

# ablate::solve::TimeStepper
## ablate::solve::TimeStepper*
the basic stepper

name (req) 
: (string) the time stepper name

arguments (req) 
: (argument map) arguments to be passed to petsc

io
: (ablate::io::Serializer) the serializer used with this timestepper

# ablate::monitors::logs::Log
## ablate::monitors::logs::CsvLog
Writes the result of the log to a csv file.  Only prints data to the log.

name (req) 
: (string) the name of the log file

## ablate::monitors::logs::StreamLog
Writes to the std::cout stream

## ablate::monitors::logs::FileLog
Writes the result of the log to a file

name (req) 
: (string) the name of the log file

## ablate::monitors::logs::StdOut
Writes to the standard out

# ablate::monitors::Monitor
## ablate::monitors::FieldErrorMonitor
Computes and reports the error every time step

log
: (ablate::monitors::logs::Log) where to record log (default is stdout)

## ablate::monitors::SolutionErrorMonitor
Computes and reports the error every time step

scope (req) 
: (ablate::parser::EnumWrapper<ablate::monitors::SolutionErrorMonitor::Scope>) how the error should be calculated ('vector', 'component')

type (req) 
: (ablate::parser::EnumWrapper<ablate::monitors::SolutionErrorMonitor::Norm>) norm type ('l2', 'linf', 'l2_norm')

log
: (ablate::monitors::logs::Log) where to record log (default is stdout)

## ablate::monitors::TimeStepMonitor
Reports the current step, time, and dt

log
: (ablate::monitors::logs::Log) where to record log (default is stdout)

interval
: (ablate::io::interval::Interval) report interval object, defaults to every

## ablate::monitors::IgnitionDelayPeakYi
Compute the ignition time based upon peak mass fraction

species (req) 
: (string) the species used to determine the peak Yi

location (req) 
: (double list) the monitor location

log
: (ablate::monitors::logs::Log) where to record the final ignition time (default is stdout)

historyLog
: (ablate::monitors::logs::Log) where to record the time and yi history (default is none)

## ablate::monitors::IgnitionDelayTemperature
Compute the ignition time based upon temperature change

eos (req) 
: (ablate::eos::EOS) the eos used to compute temperature

location (req) 
: (double list) the monitor location

thresholdTemperature (req) 
: (double) the temperature used to define ignition delay

log
: (ablate::monitors::logs::Log) where to record the final ignition time (default is stdout)

historyLog
: (ablate::monitors::logs::Log) where to record the time and yi history (default is none)

## ablate::monitors::CurveMonitor
Outputs the results along a line as a curve file (beta)

interval (req) 
: (int) output interval

prefix (req) 
: (string) the file prefix

start (req) 
: (double list) the line start location

end (req) 
: (double list) the line end location

outputFields (req) 
: (string list) a list of fields to write to the curve

outputAuxFields (req) 
: (string list) a list of aux fields to write to the curve 

## ablate::monitors::DmViewFromOptions
replicates the DMViewFromOptions function in PETSC

scope (req) 
: (ablate::parser::EnumWrapper<ablate::monitors::DmViewFromOptions::Scope>) determines if DMViewFromOptions is called initially (initial) or every time step (monitor)

options
: (string) if provided these options are used for the DMView call, otherwise global options is used

optionName
: (string) if provided the optionsName is used for DMViewFromOptions.  Needed if using global options.

## ablate::monitors::ParticleCount
Outputs the total number of particles in the domain

interval
: (int) output interval

log
: (ablate::monitors::logs::Log) where to record log (default is stdout)

## ablate::monitors::ParticleAverage
Outputs the average particle location in the domain

interval
: (int) output interval

log
: (ablate::monitors::logs::Log) where to record log (default is stdout)

# ablate::particles::initializers::Initializer
## ablate::particles::initializers::CellInitializer
simple cell initializer that puts particles in every element

particlesPerCellPerDim (req) 
: (int) particles per cell per dimension

## ablate::particles::initializers::BoxInitializer
simple box initializer that puts particles in a defined box

lower (req) 
: (double list) the lower bound of the box

upper (req) 
: (double list) the upper bound of the box

particlesPerDim (req) 
: (int) the particles per box dimension

# ablate::particles::Particles
## ablate::particles::Tracer
massless particles that advect with the flow

name (req) 
: (string) the name of the particle group

ndims (req) 
: (int) the number of dimensions for the particle

initializer (req) 
: (ablate::particles::initializers::Initializer) the initial particle setup methods

exactSolution
: (ablate::mathFunctions::MathFunction) the particle location exact solution

options (req) 
: (ablate::parameters::Parameters) options to be passed to petsc

## ablate::particles::Inertial
particles (with mass) that advect with the flow

name (req) 
: (string) the name of the particle group

ndims (req) 
: (int) the number of dimensions for the particle

parameters (req) 
: (ablate::parameters::Parameters) fluid parameters for the particles (fluidDensity, fluidViscosity, gravityField)

initializer (req) 
: (ablate::particles::initializers::Initializer) the initial particle setup methods

fieldInitialization (req) 
: (std::vector<ablate::mathFunctions::FieldFunction, std::allocator<ablate::mathFunctions::FieldFunction> >) the initial particle fields setup methods

exactSolution
: (ablate::mathFunctions::MathFunction) the particle location/velocity exact solution

options (req) 
: (ablate::parameters::Parameters) options to be passed to petsc


