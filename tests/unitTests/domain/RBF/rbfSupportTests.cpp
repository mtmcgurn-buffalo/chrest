#include <petsc.h>
#include <memory>
#include "MpiTestFixture.hpp"
#include "domain/boxMesh.hpp"
#include "domain/modifiers/distributeWithGhostCells.hpp"
#include "PetscTestErrorChecker.hpp"
#include "domain/RBF/rbfSupport.hpp"
#include "environment/runEnvironment.hpp"
#include "gtest/gtest.h"
#include "utilities/petscUtilities.hpp"

using namespace ablate;

/********************   Begin unit tests for DMPlexGetContainingCell    *************************/

struct RBFSupportParameters_ReturnID {
    testingResources::MpiTestParameter mpiTestParameter;
    std::vector<int> meshFaces;
    std::vector<double> meshStart;
    std::vector<double> meshEnd;
    std::vector<std::shared_ptr<domain::modifiers::Modifier>> meshModifiers;
    bool meshSimplex;
    std::vector<PetscScalar> xyz;
    std::vector<PetscInt> expectedCell;
};

class RBFSupportTestFixture_ReturnID : public testingResources::MpiTestFixture, public ::testing::WithParamInterface<RBFSupportParameters_ReturnID> {
   public:
    void SetUp() override { SetMpiParameters(GetParam().mpiTestParameter); }
};


TEST_P(RBFSupportTestFixture_ReturnID, ShouldReturnCellIDs) {
    StartWithMPI
        {
            // initialize petsc and mpi
            ablate::environment::RunEnvironment::Initialize(argc, argv);
            ablate::utilities::PetscUtilities::Initialize();

            auto testingParam = GetParam();

            // Create the mesh
            // Note that using -dm_view :mesh.tex:ascii_latex -dm_plex_view_scale 10 -dm_plex_view_numbers_depth 1,0,1 will create a mesh, changing numbers_depth as appropriate
            auto mesh = std::make_shared<domain::BoxMesh>(
                "mesh", std::vector<std::shared_ptr<domain::FieldDescriptor>>{}, testingParam.meshModifiers, testingParam.meshFaces, testingParam.meshStart, testingParam.meshEnd, std::vector<std::string>{}, testingParam.meshSimplex);

            PetscInt cell = -2;
            DMPlexGetContainingCell(mesh->GetDM(), &testingParam.xyz[0], &cell) >> ablate::checkError;

            PetscMPIInt rank;
            MPI_Comm_rank(PetscObjectComm((PetscObject)mesh->GetDM()), &rank);
            ASSERT_EQ(cell, testingParam.expectedCell[rank]);
        }
        ablate::environment::RunEnvironment::Finalize();
    EndWithMPI
}

INSTANTIATE_TEST_SUITE_P(
    MeshTests, RBFSupportTestFixture_ReturnID,
    testing::Values((RBFSupportParameters_ReturnID){.mpiTestParameter = {.testName = "2DQuad", .nproc = 1},
                                              .meshFaces = {10, 5},
                                              .meshStart = {0.0, 0.0},
                                              .meshEnd = {1.0, 1.0},
                                              .meshModifiers = {},
                                              .meshSimplex = false,
                                              .xyz = {0.55, 0.25},
                                              .expectedCell = {15}},
                    (RBFSupportParameters_ReturnID){.mpiTestParameter = {.testName = "2DSimplex", .nproc = 1},
                                              .meshFaces = {10, 5},
                                              .meshStart = {0.0, 0.0},
                                              .meshEnd = {1.0, 1.0},
                                              .meshModifiers = {},
                                              .meshSimplex = true,
                                              .xyz = {0.55, 0.25},
                                              .expectedCell = {49}},
                    (RBFSupportParameters_ReturnID){.mpiTestParameter = {.testName = "3DQuad", .nproc = 1},
                                              .meshFaces = {2, 2, 2},
                                              .meshStart = {0.0, 0.0, 0.0},
                                              .meshEnd = {1.0, 1.0, 1.0},
                                              .meshModifiers = {},
                                              .meshSimplex = false,
                                              .xyz = {0.6, 0.42, 0.8},
                                              .expectedCell = {5}},
                    (RBFSupportParameters_ReturnID){.mpiTestParameter = {.testName = "3DSimplex", .nproc = 1},
                                              .meshFaces = {1, 1, 1},
                                              .meshStart = {0.0, 0.0, 0.0},
                                              .meshEnd = {2.0, 1.0, 1.0},
                                              .meshModifiers = {},
                                              .meshSimplex = true,
                                              .xyz = {0.1, 0.9, 0.9},
                                              .expectedCell = {4}},
                    (RBFSupportParameters_ReturnID){.mpiTestParameter = {.testName = "3DSimplexFail", .nproc = 1},
                                              .meshFaces = {1, 1, 1},
                                              .meshStart = {0.0, 0.0, 0.0},
                                              .meshEnd = {2.0, 1.0, 1.0},
                                              .meshModifiers = {},
                                              .meshSimplex = true,
                                              .xyz = {2.1, 0.9, 0.9},
                                              .expectedCell = {-1}},
                     (RBFSupportParameters_ReturnID){.mpiTestParameter = {.testName = "2DQuadMPI", .nproc = 2},
                                              .meshFaces = {10, 10},
                                              .meshStart = {0.0, 0.0},
                                              .meshEnd = {1.0, 1.0},
                                              .meshModifiers = {},
                                              .meshSimplex = false,
                                              .xyz = {0.55, 0.25},
                                              .expectedCell = {10, -1}},
                      (RBFSupportParameters_ReturnID){.mpiTestParameter = {.testName = "2DQuadMPIMod", .nproc = 2}, // This is mainly here to check if there is ever a change in how DMLocatePoints functions
                                              .meshFaces = {10, 10},
                                              .meshStart = {0.0, 0.0},
                                              .meshEnd = {1.0, 1.0},
                                              .meshModifiers = std::vector<std::shared_ptr<ablate::domain::modifiers::Modifier>>{std::make_shared<domain::modifiers::DistributeWithGhostCells>(1)},
                                              .meshSimplex = false,
                                              .xyz = {0.55, 0.25},
                                              .expectedCell = {10, -1}}
                  ),
    [](const testing::TestParamInfo<RBFSupportParameters_ReturnID>& info) { return info.param.mpiTestParameter.getTestName(); });

/********************   End unit tests for DMPlexGetContainingCell    *************************/

/********************   Begin unit tests for DMPlexGetNeighborCells    *************************/

struct RBFSupportParameters_NeighborCells {
    testingResources::MpiTestParameter mpiTestParameter;
    std::vector<int> meshFaces;
    std::vector<double> meshStart;
    std::vector<double> meshEnd;
    std::vector<std::shared_ptr<domain::modifiers::Modifier>> meshModifiers;
    bool meshSimplex;
    std::vector<PetscInt> centerCell;
    PetscInt numLevels;
    PetscReal maxDistance;
    PetscInt minNumberCells;
    PetscBool useVertices;
    std::vector<PetscInt> expectedNumberOfCells;
    std::vector<std::vector<PetscInt>> expectedCellList;
};



class RBFSupportTestFixture_NeighborCells : public testingResources::MpiTestFixture, public ::testing::WithParamInterface<RBFSupportParameters_NeighborCells> {
   public:
    void SetUp() override { SetMpiParameters(GetParam().mpiTestParameter); }
};



TEST_P(RBFSupportTestFixture_NeighborCells, ShouldReturnNeighborCells) {
    StartWithMPI
        {
            // initialize petsc and mpi
            ablate::environment::RunEnvironment::Initialize(argc, argv);
            ablate::utilities::PetscUtilities::Initialize();

            auto testingParam = GetParam();

            // Create the mesh
            // Note that using -dm_view :mesh.tex:ascii_latex -dm_plex_view_scale 10 -dm_plex_view_numbers_depth 1,0,1 will create a mesh, changing numbers_depth as appropriate
            auto mesh = std::make_shared<domain::BoxMesh>(
                "mesh", std::vector<std::shared_ptr<domain::FieldDescriptor>>{}, testingParam.meshModifiers, testingParam.meshFaces, testingParam.meshStart, testingParam.meshEnd, std::vector<std::string>{}, testingParam.meshSimplex);


            PetscInt nCells, *cells;
            PetscMPIInt rank;
            MPI_Comm_rank(PetscObjectComm((PetscObject)mesh->GetDM()), &rank);

            DMPlexGetNeighborCells(mesh->GetDM(), testingParam.centerCell[rank], testingParam.numLevels, testingParam.maxDistance, testingParam.minNumberCells, testingParam.useVertices, &nCells, &cells) >> ablate::checkError;


            ASSERT_EQ(nCells, testingParam.expectedNumberOfCells[rank]);

            // There may be a better way of doing this, but with DMPlexGetNeighborCells sticking with C-only code there may not be.
            // Also note that as cells is a dynamically allocated array there is not way (that I know of) to get the number of elements.
            for (int i = 0; i < nCells; ++i) {
              ASSERT_EQ(cells[i], testingParam.expectedCellList[rank][i]);
            }

        }
        ablate::environment::RunEnvironment::Finalize();
    EndWithMPI
}

// One thing that is NOT being tested right now is periodicity.
// This is the code used to generate the results. Modify as necessary. It would be relatively easy to automate the unit-test creation, but for right
//  now it's done manually as the results need to be initially checked.
//
//
//#include "domain/boxMesh.hpp"
//#include "domain/modifiers/distributeWithGhostCells.hpp"
//    auto mesh = std::make_shared<domain::BoxMesh>(
//                "mesh",
//                std::vector<std::shared_ptr<domain::FieldDescriptor>>{},
//                std::vector<std::shared_ptr<ablate::domain::modifiers::Modifier>>{std::make_shared<domain::modifiers::DistributeWithGhostCells>(3)},
//                std::vector<int>{10, 10},
//                std::vector<double>{0.0, 0.0},
//                std::vector<double>{1.0, 1.0},
//                std::vector<std::string>{},
//                false);


//      DMViewFromOptions(mesh->GetDM(), NULL, "-dm_view");


//      PetscMPIInt rank;
//      MPI_Comm_rank(PetscObjectComm((PetscObject)mesh->GetDM()), &rank);

//      PetscInt nCells, *cells;
//      PetscInt centerCell = -1;
//      if(rank==0) centerCell = 55;
//      else centerCell = 55;
//      PetscInt maxLevels = 2;
//      PetscReal maxDist = -1;
//      PetscInt minNumberCells = -1;

//      DMPlexGetNeighborCells(mesh->GetDM(), centerCell, maxLevels, maxDist, minNumberCells, PETSC_TRUE, &nCells, &cells) >> ablate::checkError;
//      printf("%d: center: %d\n", rank, centerCell);
//      printf("%d: nCells: %d\n", rank, nCells);
//      printf("%d: ", rank);
//      for(int i=0;i<nCells;++i){
//        printf("%d,", cells[i]);
//      }
//      printf("\n");


//    PetscInt cell;
//    PetscScalar xyz[2] = {0.55, 0.25};
//    DMPlexGetContainingCell(mesh->GetDM(), xyz, &cell) >> ablate::checkError;
//    printf("Cell: %d\n", cell);

INSTANTIATE_TEST_SUITE_P(
    MeshTests, RBFSupportTestFixture_NeighborCells,
    testing::Values((RBFSupportParameters_NeighborCells){.mpiTestParameter = {.testName = "2DQuadVert", .nproc = 1},
                                              .meshFaces = {10, 10},
                                              .meshStart = {0.0, 0.0},
                                              .meshEnd = {1.0, 1.0},
                                              .meshModifiers = {},
                                              .meshSimplex = false,
                                              .centerCell = {25},
                                              .numLevels = -1,
                                              .maxDistance = -1.0,
                                              .minNumberCells = 25,
                                              .useVertices = PETSC_TRUE,
                                              .expectedNumberOfCells = {25},
                                              .expectedCellList = {{3,4,5,6,7,13,14,15,16,17,23,24,25,26,27,33,34,35,36,37,43,44,45,46,47}}},
                    (RBFSupportParameters_NeighborCells){.mpiTestParameter = {.testName = "2DQuadVertCorner", .nproc = 1},
                                              .meshFaces = {10, 10},
                                              .meshStart = {0.0, 0.0},
                                              .meshEnd = {1.0, 1.0},
                                              .meshModifiers = {},
                                              .meshSimplex = false,
                                              .centerCell = {0},
                                              .numLevels = -1,
                                              .maxDistance = -1.0,
                                              .minNumberCells = 25,
                                              .useVertices = PETSC_TRUE,
                                              .expectedNumberOfCells = {25},
                                              .expectedCellList = {{0,1,2,3,4,10,11,12,13,14,20,21,22,23,24,30,31,32,33,34,40,41,42,43,44}}},
                    (RBFSupportParameters_NeighborCells){.mpiTestParameter = {.testName = "2DTriVert", .nproc = 1},
                                              .meshFaces = {10, 10},
                                              .meshStart = {0.0, 0.0},
                                              .meshEnd = {1.0, 1.0},
                                              .meshModifiers = {},
                                              .meshSimplex = true,
                                              .centerCell = {199},
                                              .numLevels = -1,
                                              .maxDistance = -1.0,
                                              .minNumberCells = 25,
                                              .useVertices = PETSC_TRUE,
                                              .expectedNumberOfCells = {39},
                                              .expectedCellList = {{40,41,42,45,70,71,72,73,74,75,76,77,78,79,80,81,82,94,95,98,109,110,111,112,113,114,117,120,122,149,150,151,152,153,154,156,158,159,199}}},
                    (RBFSupportParameters_NeighborCells){.mpiTestParameter = {.testName = "2DTriVertCorner", .nproc = 1},
                                              .meshFaces = {10, 10},
                                              .meshStart = {0.0, 0.0},
                                              .meshEnd = {1.0, 1.0},
                                              .meshModifiers = {},
                                              .meshSimplex = true,
                                              .centerCell = {0},
                                              .numLevels = -1,
                                              .maxDistance = -1.0,
                                              .minNumberCells = 25,
                                              .useVertices = PETSC_TRUE,
                                              .expectedNumberOfCells = {34},
                                              .expectedCellList = {{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,16,17,18,19,20,21,22,23,24,25,26,27,28,30,31,33,44,47,57}}},
                    (RBFSupportParameters_NeighborCells){.mpiTestParameter = {.testName = "2DTriVertNoOverlap", .nproc = 2},
                                              .meshFaces = {10, 10},
                                              .meshStart = {0.0, 0.0},
                                              .meshEnd = {1.0, 1.0},
                                              .meshModifiers = {},
                                              .meshSimplex = true,
                                              .centerCell = {56,19},
                                              .numLevels = -1,
                                              .maxDistance = -1.0,
                                              .minNumberCells = 10,
                                              .useVertices = PETSC_TRUE,
                                              .expectedNumberOfCells = {21,10},
                                              .expectedCellList = {{34,35,37,38,40,41,42,45,48,52,54,55,56,57,58,59,60,71,72,73,77},{16,17,18,19,20,21,22,23,35,102}}},
                    (RBFSupportParameters_NeighborCells){.mpiTestParameter = {.testName = "2DQuadVertOverlap", .nproc = 4},
                                              .meshFaces = {10, 10},
                                              .meshStart = {0.0, 0.0},
                                              .meshEnd = {1.0, 1.0},
                                              .meshModifiers = std::vector<std::shared_ptr<ablate::domain::modifiers::Modifier>>{std::make_shared<domain::modifiers::DistributeWithGhostCells>(1)},
                                              .meshSimplex = false,
                                              .centerCell = {24, 4, 20, 0},
                                              .numLevels = -1,
                                              .maxDistance = -1.0,
                                              .minNumberCells = 9,
                                              .useVertices = PETSC_TRUE,
                                              .expectedNumberOfCells = {9,9,9,9},
                                              .expectedCellList = { {18,19,23,24,28,29,30,34,35},
                                                                    {3,4,8,9,28,29,30,31,32},
                                                                    {15,16,20,21,28,29,30,31,32},
                                                                    {0,1,5,6,25,26,27,31,32}
                                                                  }},
                    (RBFSupportParameters_NeighborCells){.mpiTestParameter = {.testName = "2DQuadEdge", .nproc = 1},
                                              .meshFaces = {10, 10},
                                              .meshStart = {0.0, 0.0},
                                              .meshEnd = {1.0, 1.0},
                                              .meshModifiers = std::vector<std::shared_ptr<ablate::domain::modifiers::Modifier>>{},
                                              .meshSimplex = false,
                                              .centerCell = {54},
                                              .numLevels = -1,
                                              .maxDistance = -1.0,
                                              .minNumberCells = 9,
                                              .useVertices = PETSC_FALSE,
                                              .expectedNumberOfCells = {13},
                                              .expectedCellList = { {34,43,44,45,52,53,54,55,56,63,64,65,74} }},
                    (RBFSupportParameters_NeighborCells){.mpiTestParameter = {.testName = "2DTriEdge", .nproc = 1},
                                              .meshFaces = {10, 10},
                                              .meshStart = {0.0, 0.0},
                                              .meshEnd = {1.0, 1.0},
                                              .meshModifiers = std::vector<std::shared_ptr<ablate::domain::modifiers::Modifier>>{},
                                              .meshSimplex = true,
                                              .centerCell = {199},
                                              .numLevels = -1,
                                              .maxDistance = -1.0,
                                              .minNumberCells = 9,
                                              .useVertices = PETSC_FALSE,
                                              .expectedNumberOfCells = {10},
                                              .expectedCellList = { {76,78,79,80,98,111,149,150,159,199} }},
                    (RBFSupportParameters_NeighborCells){.mpiTestParameter = {.testName = "2DQuadEdgeOverlap", .nproc = 2},
                                              .meshFaces = {10, 10},
                                              .meshStart = {0.0, 0.0},
                                              .meshEnd = {1.0, 1.0},
                                              .meshModifiers = std::vector<std::shared_ptr<ablate::domain::modifiers::Modifier>>{std::make_shared<domain::modifiers::DistributeWithGhostCells>(1)},
                                              .meshSimplex = false,
                                              .centerCell = {11, 34},
                                              .numLevels = -1,
                                              .maxDistance = -1.0,
                                              .minNumberCells = 9,
                                              .useVertices = PETSC_FALSE,
                                              .expectedNumberOfCells = {13, 12},
                                              .expectedCellList = { {1,5,6,7,10,11,12,13,15,16,17,21,52},
                                                                    {24,28,29,32,33,34,38,39,44,55,56,57} }},
                     (RBFSupportParameters_NeighborCells){.mpiTestParameter = {.testName = "3DQuadFace", .nproc = 1},
                                              .meshFaces = {4, 4, 4},
                                              .meshStart = {0.0, 0.0, 0.0},
                                              .meshEnd = {1.0, 1.0, 1.0},
                                              .meshModifiers = std::vector<std::shared_ptr<ablate::domain::modifiers::Modifier>>{},
                                              .meshSimplex = false,
                                              .centerCell = {25},
                                              .numLevels = -1,
                                              .maxDistance = -1.0,
                                              .minNumberCells = 20,
                                              .useVertices = PETSC_FALSE,
                                              .expectedNumberOfCells = {22},
                                              .expectedCellList = { {5,8,9,10,13,17,20,21,22,24,25,26,27,28,29,30,37,40,41,42,45,57}}},
                      (RBFSupportParameters_NeighborCells){.mpiTestParameter = {.testName = "3DTriFace", .nproc = 1},
                                              .meshFaces = {4, 4, 4},
                                              .meshStart = {0.0, 0.0, 0.0},
                                              .meshEnd = {1.0, 1.0, 1.0},
                                              .meshModifiers = std::vector<std::shared_ptr<ablate::domain::modifiers::Modifier>>{},
                                              .meshSimplex = true,
                                              .centerCell = {25},
                                              .numLevels = -1,
                                              .maxDistance = -1.0,
                                              .minNumberCells = 20,
                                              .useVertices = PETSC_FALSE,
                                              .expectedNumberOfCells = {32},
                                              .expectedCellList = { {1,4,8,13,15,17,25,27,28,32,33,36,38,39,40,41,46,51,54,56,57,74,95,102,122,123,135,150,166,197,198,201}}},
                      (RBFSupportParameters_NeighborCells){.mpiTestParameter = {.testName = "2DQuadDistanceEdge", .nproc = 1},
                                              .meshFaces = {10, 10},
                                              .meshStart = {0.0, 0.0},
                                              .meshEnd = {1.0, 1.0},
                                              .meshModifiers = std::vector<std::shared_ptr<ablate::domain::modifiers::Modifier>>{},
                                              .meshSimplex = false,
                                              .centerCell = {55},
                                              .numLevels = -1,
                                              .maxDistance = 0.28,
                                              .minNumberCells = -1,
                                              .useVertices = PETSC_FALSE,
                                              .expectedNumberOfCells = {21},
                                              .expectedCellList = { {34,35,36,43,44,45,46,47,53,54,55,56,57,63,64,65,66,67,74,75,76}}},
                      (RBFSupportParameters_NeighborCells){.mpiTestParameter = {.testName = "2DQuadDistanceVert", .nproc = 1},
                                              .meshFaces = {10, 10},
                                              .meshStart = {0.0, 0.0},
                                              .meshEnd = {1.0, 1.0},
                                              .meshModifiers = std::vector<std::shared_ptr<ablate::domain::modifiers::Modifier>>{},
                                              .meshSimplex = false,
                                              .centerCell = {55},
                                              .numLevels = -1,
                                              .maxDistance = 0.28,
                                              .minNumberCells = -1,
                                              .useVertices = PETSC_TRUE,
                                              .expectedNumberOfCells = {21},
                                              .expectedCellList = { {34,35,36,43,44,45,46,47,53,54,55,56,57,63,64,65,66,67,74,75,76}}},


                      (RBFSupportParameters_NeighborCells){.mpiTestParameter = {.testName = "2DQuadDistanceEdgeMPI", .nproc = 2},
                                              .meshFaces = {10, 10},
                                              .meshStart = {0.0, 0.0},
                                              .meshEnd = {1.0, 1.0},
                                              .meshModifiers = std::vector<std::shared_ptr<ablate::domain::modifiers::Modifier>>{std::make_shared<domain::modifiers::DistributeWithGhostCells>(3)},
                                              .meshSimplex = false,
                                              .centerCell = {25, 29},
                                              .numLevels = -1,
                                              .maxDistance = 0.28,
                                              .minNumberCells = -1,
                                              .useVertices = PETSC_FALSE,
                                              .expectedNumberOfCells = {21, 21},
                                              .expectedCellList = { {15,16,20,21,22,25,26,27,30,31,32,35,36,61,63,64,66,67,69,70,73},
                                                                    {18,19,22,23,24,27,28,29,32,33,34,38,39,59,62,63,65,66,68,69,71}}},
                      (RBFSupportParameters_NeighborCells){.mpiTestParameter = {.testName = "2DQuadDistanceVertMPI", .nproc = 2},
                                              .meshFaces = {10, 10},
                                              .meshStart = {0.0, 0.0},
                                              .meshEnd = {1.0, 1.0},
                                              .meshModifiers = std::vector<std::shared_ptr<ablate::domain::modifiers::Modifier>>{std::make_shared<domain::modifiers::DistributeWithGhostCells>(3)},
                                              .meshSimplex = false,
                                              .centerCell = {25, 29},
                                              .numLevels = -1,
                                              .maxDistance = 0.28,
                                              .minNumberCells = -1,
                                              .useVertices = PETSC_TRUE,
                                              .expectedNumberOfCells = {21, 21},
                                              .expectedCellList = { {15,16,20,21,22,25,26,27,30,31,32,35,36,61,63,64,66,67,69,70,73},
                                                                    {18,19,22,23,24,27,28,29,32,33,34,38,39,59,62,63,65,66,68,69,71}}},
                      (RBFSupportParameters_NeighborCells){.mpiTestParameter = {.testName = "2DTriDistanceEdge", .nproc = 1},
                                              .meshFaces = {10, 10},
                                              .meshStart = {0.0, 0.0},
                                              .meshEnd = {1.0, 1.0},
                                              .meshModifiers = std::vector<std::shared_ptr<ablate::domain::modifiers::Modifier>>{},
                                              .meshSimplex = true,
                                              .centerCell = {199},
                                              .numLevels = -1,
                                              .maxDistance = 0.14,
                                              .minNumberCells = -1,
                                              .useVertices = PETSC_FALSE,
                                              .expectedNumberOfCells = {11},
                                              .expectedCellList = { {40,73,76,79,80,98,111,149,150,159,199}}},
                      (RBFSupportParameters_NeighborCells){.mpiTestParameter = {.testName = "2DTriDistanceVert", .nproc = 1},
                                              .meshFaces = {10, 10},
                                              .meshStart = {0.0, 0.0},
                                              .meshEnd = {1.0, 1.0},
                                              .meshModifiers = std::vector<std::shared_ptr<ablate::domain::modifiers::Modifier>>{},
                                              .meshSimplex = true,
                                              .centerCell = {199},
                                              .numLevels = -1,
                                              .maxDistance = 0.14,
                                              .minNumberCells = -1,
                                              .useVertices = PETSC_TRUE,
                                              .expectedNumberOfCells = {11},
                                              .expectedCellList = { {40,73,76,79,80,98,111,149,150,159,199}}},

                      (RBFSupportParameters_NeighborCells){.mpiTestParameter = {.testName = "2DTriDistanceEdgeMPI", .nproc = 2},
                                              .meshFaces = {10, 10},
                                              .meshStart = {0.0, 0.0},
                                              .meshEnd = {1.0, 1.0},
                                              .meshModifiers = std::vector<std::shared_ptr<ablate::domain::modifiers::Modifier>>{std::make_shared<domain::modifiers::DistributeWithGhostCells>(3)},
                                              .meshSimplex = true,
                                              .centerCell = {60, 102},
                                              .numLevels = -1,
                                              .maxDistance = 0.14,
                                              .minNumberCells = -1,
                                              .useVertices = PETSC_FALSE,
                                              .expectedNumberOfCells = {12, 11},
                                              .expectedCellList = { {38,40,41,45,56,58,60,71,113,114,132,141},
                                                                    {82,83,84,86,95,96,97,99,102,138,141}}},
                      (RBFSupportParameters_NeighborCells){.mpiTestParameter = {.testName = "2DTriDistanceVertMPI", .nproc = 2},
                                              .meshFaces = {10, 10},
                                              .meshStart = {0.0, 0.0},
                                              .meshEnd = {1.0, 1.0},
                                              .meshModifiers = std::vector<std::shared_ptr<ablate::domain::modifiers::Modifier>>{std::make_shared<domain::modifiers::DistributeWithGhostCells>(3)},
                                              .meshSimplex = true,
                                              .centerCell = {60, 102},
                                              .numLevels = -1,
                                              .maxDistance = 0.14,
                                              .minNumberCells = -1,
                                              .useVertices = PETSC_TRUE,
                                              .expectedNumberOfCells = {12, 11},
                                              .expectedCellList = { {38,40,41,45,56,58,60,71,113,114,132,141},
                                                                    {82,83,84,86,95,96,97,99,102,138,141}}},
                      (RBFSupportParameters_NeighborCells){.mpiTestParameter = {.testName = "2DQuadLevelVert", .nproc = 1},
                                              .meshFaces = {10, 10},
                                              .meshStart = {0.0, 0.0},
                                              .meshEnd = {1.0, 1.0},
                                              .meshModifiers = std::vector<std::shared_ptr<ablate::domain::modifiers::Modifier>>{std::make_shared<domain::modifiers::DistributeWithGhostCells>(3)},
                                              .meshSimplex = false,
                                              .centerCell = {55},
                                              .numLevels = 2,
                                              .maxDistance = -1.0,
                                              .minNumberCells = -1,
                                              .useVertices = PETSC_TRUE,
                                              .expectedNumberOfCells = {25},
                                              .expectedCellList = { {33,34,35,36,37,43,44,45,46,47,53,54,55,56,57,63,64,65,66,67,73,74,75,76,77}}},
                      (RBFSupportParameters_NeighborCells){.mpiTestParameter = {.testName = "2DQuadLevelEdge", .nproc = 1},
                                              .meshFaces = {10, 10},
                                              .meshStart = {0.0, 0.0},
                                              .meshEnd = {1.0, 1.0},
                                              .meshModifiers = std::vector<std::shared_ptr<ablate::domain::modifiers::Modifier>>{std::make_shared<domain::modifiers::DistributeWithGhostCells>(3)},
                                              .meshSimplex = false,
                                              .centerCell = {55},
                                              .numLevels = 2,
                                              .maxDistance = -1.0,
                                              .minNumberCells = -1,
                                              .useVertices = PETSC_FALSE,
                                              .expectedNumberOfCells = {13},
                                              .expectedCellList = { {35,44,45,46,53,54,55,56,57,64,65,66,75}}}
                  ),
    [](const testing::TestParamInfo<RBFSupportParameters_NeighborCells>& info) { return info.param.mpiTestParameter.getTestName(); });




struct RBFSupportParameters_ErrorChecking {
    testingResources::MpiTestParameter mpiTestParameter;
};

class RBFSupportTestFixture_ErrorChecking : public testingResources::MpiTestFixture, public ::testing::WithParamInterface<RBFSupportParameters_ErrorChecking> {
   public:
    void SetUp() override { SetMpiParameters(GetParam().mpiTestParameter); }
};

TEST_P(RBFSupportTestFixture_ErrorChecking, ShouldThrowErrorForTooManyInputs) {

    StartWithMPI
        {
            // initialize petsc and mpi
            ablate::environment::RunEnvironment::Initialize(argc, argv);
            ablate::utilities::PetscUtilities::Initialize();


            // Create the mesh
            auto mesh = std::make_shared<domain::BoxMesh>(
                        "mesh", std::vector<std::shared_ptr<domain::FieldDescriptor>>{}, std::vector<std::shared_ptr<domain::modifiers::Modifier>>{}, std::vector<int>{2, 2}, std::vector<double>{0.0, 0.0}, std::vector<double>{1.0, 1.0});

            PetscInt nCells, *cells;

            EXPECT_ANY_THROW(DMPlexGetNeighborCells(mesh->GetDM(), 0, 1, 1.0, 1, PETSC_TRUE, &nCells, &cells)>> ablate::checkError);

        }
        ablate::environment::RunEnvironment::Finalize();
    EndWithMPI
}

INSTANTIATE_TEST_SUITE_P(
    MeshTests, RBFSupportTestFixture_ErrorChecking,
    testing::Values((RBFSupportParameters_ErrorChecking){.mpiTestParameter = {.testName = "SingleProc", .nproc = 1}},
                      (RBFSupportParameters_ErrorChecking){.mpiTestParameter = {.testName = "DualProcs", .nproc = 2}}
                  ),
    [](const testing::TestParamInfo<RBFSupportParameters_ErrorChecking>& info) { return info.param.mpiTestParameter.getTestName(); });


/********************   End unit tests for DMPlexGetNeighborCells    *************************/