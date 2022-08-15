#include "lsSupport.hpp"
#include <petsc/private/vecimpl.h>

// Return all cells which share an vertex or edge/face with a center cell
// dm - The mesh
// p - The cell to use
// maxDist - Maximum distance from p to consider adding
// useVertices - Should we include cells which share a vertex (TRUE) or an edge/face (FALSE)
// nCells - Number of cells found
// cells - The IDs of the cells found.
PetscErrorCode DMPlexGetNeighborCells_Internal(DM dm, PetscInt p, PetscReal maxDist, PetscBool useVertices, PetscInt *nCells, PetscInt *cells[]) {

  PetscInt        cStart, cEnd, vStart, vEnd;
  PetscInt        cl, nClosure, *closure = NULL;
  PetscInt        st, nStar, *star = NULL;
  PetscInt        n, list[10000];  // As of right now just make it a list big enough to hold everything. There must be a better way of doing this.
  PetscInt        i, dim;
  PetscReal       x0[3], x[3], dist;
  PetscErrorCode  ierr;

  PetscFunctionBegin;

  ierr = DMGetDimension(dm, &dim);CHKERRQ(ierr);

  ierr = DMPlexComputeCellGeometryFVM(dm, p, NULL, x0, NULL);CHKERRQ(ierr); // Center of the cell-of-interest

  ierr = DMPlexGetHeightStratum(dm, 0, &cStart, &cEnd);CHKERRQ(ierr);       // Range of cells

  if (useVertices) {
    ierr = DMPlexGetDepthStratum(dm, 0, &vStart, &vEnd);CHKERRQ(ierr);      // Range of vertices
  }
  else {
    ierr = DMPlexGetHeightStratum(dm, 1, &vStart, &vEnd);CHKERRQ(ierr);     // Range of edges (2D) or faces (3D)
  }

  n = 0;
  ierr = DMPlexGetTransitiveClosure(dm, p, PETSC_TRUE, &nClosure, &closure);CHKERRQ(ierr); // All points associated with the cell

  for (cl = 0; cl < nClosure*2; cl += 2) {
    if (closure[cl] >= vStart && closure[cl] < vEnd){ // Only use the points corresponding to either a vertex or edge/face.
      ierr = DMPlexGetTransitiveClosure(dm, closure[cl], PETSC_FALSE, &nStar, &star);CHKERRQ(ierr); // Get all points using this vertex or edge/face.

      for (st = 0; st< nStar*2; st += 2) {
        if( star[st] >= cStart && star[st] < cEnd){   // If the point is a cell add it.
          ierr = DMPlexComputeCellGeometryFVM(dm, star[st], NULL, x, NULL);CHKERRQ(ierr); // Center of the candidate cell.
          dist = 0;
          for (i = 0; i < dim; ++i) {
            dist += PetscSqr(x0[i] - x[i]);
          }

          if (PetscSqrtReal(dist) <= maxDist) {   // Only add if the distance is within maxDist
            list[n++] = star[st];
          }
        }
      }

      ierr = DMPlexRestoreTransitiveClosure(dm, closure[cl], PETSC_FALSE, &nStar, &star);CHKERRQ(ierr);
    }
  }

  ierr = DMPlexRestoreTransitiveClosure(dm, p, PETSC_TRUE, &nClosure, &closure);CHKERRQ(ierr);
  ierr = PetscSortRemoveDupsInt(&n, list);CHKERRQ(ierr);
  ierr = PetscMalloc1(n, cells);CHKERRQ(ierr);
  ierr = PetscArraycpy(*cells, list, n);CHKERRQ(ierr);
  *nCells = n;

  PetscFunctionReturn(0);
}


// Return the list of neighboring cells to cell p using a combination of number of levels and maximum distance
// dm - The mesh
// levels - Number of neighboring cells to check
// maxDist - Maximum distance to include
// minNumberCells - The minimum number of cells to return.
// nCells - Number of neighboring cells
// cells - The list of neighboring cell IDs
//
// Note: The intended use is to use either levels/maxDist OR minNumberCells. Right now a check isn't done on only selecting one, but that might be added in the future.
PetscErrorCode DMPlexGetNeighborCells(DM dm, PetscInt p, PetscInt levels, PetscReal maxDist, PetscInt minNumberCells, PetscBool useVertices, PetscInt *nCells, PetscInt *cells[]) {
  PetscInt        numAdd, *addList;
  PetscInt        n = 0, n0, list[10000];
  PetscInt        l, i;
  PetscErrorCode  ierr;

  PetscFunctionBegin;

  if (levels > 0 || minNumberCells > 0) {

    // Use minNumberCells if provided
    if (minNumberCells > 0) {
      levels = PETSC_MAX_INT;
      maxDist = PETSC_MAX_REAL;
    }
    else {

      minNumberCells = PETSC_MAX_INT;
      if (maxDist < 0.0) maxDist = PETSC_MAX_REAL;


#if 0
      // If the maximum distance isn't provided estimate it based on the number of levels
      if (maxDist < 0.0) {
        PetscReal       h;
        DMPlexGetMinRadius(dm, &h); // This returns the minimum distance from any cell centroid to a face.
        h *= 2.0;                   // Double it to get the grid spacing.
        maxDist = ((PetscReal)levels+1.0)*h;
      }
#endif
    }

    // Get one level of neighboring cells
    ierr = DMPlexGetNeighborCells_Internal(dm, p, maxDist, useVertices, &n, &addList);CHKERRQ(ierr);
    ierr = PetscArraycpy(&list[0], addList, n);
    ierr = PetscFree(addList);

    l = 1;

    while (l < levels && n<minNumberCells) {
      ++l;
      n0 = n;
      for (i = 0; i < n0; ++i) {
        ierr = DMPlexGetNeighborCells_Internal(dm, list[i], maxDist, useVertices, &numAdd, &addList);CHKERRQ(ierr);
        ierr = PetscArraycpy(&list[n], addList, numAdd);
        n += numAdd;
        ierr = PetscFree(addList);
      }
      ierr = PetscSortRemoveDupsInt(&n, list);CHKERRQ(ierr);
    }

    ierr = PetscMalloc1(n, cells);CHKERRQ(ierr);
    ierr = PetscArraycpy(*cells, list, n);CHKERRQ(ierr);
    *nCells = n;
  }


  PetscFunctionReturn(0);
}



PetscErrorCode DMGetFieldVec(DM dm, Vec v, PetscInt field, PetscInt height, IS *is, Vec *subv) {
  PetscSection    sectionLocal, sectionGlobal;
  PetscInt        cStart, cEnd;
  PetscErrorCode  ierr;

  PetscFunctionBegin;

  ierr = DMPlexGetHeightStratum(dm, height, &cStart, &cEnd);CHKERRQ(ierr);
  ierr = DMGetGlobalSection(dm, &sectionGlobal);CHKERRQ(ierr);
  ierr = DMGetLocalSection(dm, &sectionLocal);CHKERRQ(ierr);

  ierr = PetscSectionGetField_Internal(sectionLocal, sectionGlobal, v, field, cStart, cEnd, is, subv);CHKERRQ(ierr);

  PetscFunctionReturn(0);
}

PetscErrorCode DMRestoreFieldVec(DM dm, Vec v, PetscInt field, PetscInt height, IS *is, Vec *subv) {
  PetscErrorCode  ierr;

  PetscFunctionBegin;

  ierr = VecRestoreSubVector(v, *is, subv);CHKERRQ(ierr);
  ierr = ISDestroy(is);CHKERRQ(ierr);

  PetscFunctionReturn(0);
}

// Given a point in space and normal vector determine the vector of the plane with a given offset in the unit normal direction.
static PetscErrorCode DMPlaneVectors_2D_Internal(const PetscReal x0[], const PetscReal n[], const PetscReal offset, PetscReal segment[]) {

  // Get the base crossing assuming that the plane passes through the origin
  segment[0] = segment[1] = 0.0;
  segment[2] = -n[1];
  segment[3] =  n[0];

  // Now shift to the point-of-interest
  segment[0] += x0[0]; segment[2] += x0[0];
  segment[1] += x0[1]; segment[3] += x0[1];

  // Additional offset in normal direction
  const PetscReal off[2] = {offset*n[0], offset*n[1]};
  segment[0] += off[0]; segment[2] += off[0];
  segment[1] += off[1]; segment[3] += off[1];

  PetscFunctionReturn(0);

}

// Given a point in space and normal vector determine the vector of the plane with a given offset in the unit normal direction.
static PetscErrorCode DMPlaneVectors_3D_Internal(const PetscReal x0[], const PetscReal n[], const PetscReal offset, PetscReal segmentA[], PetscReal segmentB[]) {

  // Get the base crossing assuming that the plane passes through the origin.
  // Use the cross product of the normal and a unit vector in the direction of the smallest component of the normal.
  segmentA[0] = segmentA[1] = segmentA[2] = 0.0;
  if ((PetscAbsReal(n[2]) < PetscAbsReal(n[0])) || (PetscAbsReal(n[2]) < PetscAbsReal(n[1]))) {
    segmentA[3] = -n[1];
    segmentA[4] =  n[0];
    segmentA[5] =  0.0;
  }
  else if  ((PetscAbsReal(n[1]) < PetscAbsReal(n[0])) || (PetscAbsReal(n[1]) < PetscAbsReal(n[2]))) {
    segmentA[3] = -n[2];
    segmentA[4] =  0.0;
    segmentA[5] =  n[0];
  }
  else {
    segmentA[3] =  0.0;
    segmentA[4] = -n[2];
    segmentA[5] =  n[1];
  }

  // The other segment will be the cross product of segmentA and the normal
  segmentB[0] = n[1]*segmentA[2]-n[2]*segmentA[1];
  segmentB[1] = n[2]*segmentA[0]-n[0]*segmentA[2];
  segmentB[2] = n[0]*segmentA[1]-n[1]*segmentA[0];


  // Now shift to the point-of-interest
  segmentA[0] += x0[0]; segmentA[3] += x0[0];
  segmentA[1] += x0[1]; segmentA[4] += x0[1];
  segmentA[2] += x0[2]; segmentA[5] += x0[2];

  segmentB[0] += x0[0]; segmentB[3] += x0[0];
  segmentB[1] += x0[1]; segmentB[4] += x0[1];
  segmentB[2] += x0[2]; segmentB[5] += x0[2];

  // Additional offset in normal direction
  const PetscReal off[3] = {offset*n[0], offset*n[1], offset*n[2]};
  segmentA[0] += off[0]; segmentA[3] += off[0];
  segmentA[1] += off[1]; segmentA[4] += off[1];
  segmentA[2] += off[2]; segmentA[5] += off[2];

  segmentB[0] += off[0]; segmentB[3] += off[0];
  segmentB[1] += off[1]; segmentB[4] += off[1];
  segmentB[2] += off[2]; segmentB[5] += off[2];

  PetscFunctionReturn(0);
}

PetscErrorCode DMPlaneVectors(DM dm, const PetscReal x0[], const PetscReal n[], const PetscReal offset, PetscReal segmentA[], PetscReal segmentB[]) {
  PetscInt dim;
  PetscErrorCode err;

  DMGetDimension(dm, &dim);

  if (dim==1) {
   segmentA[0] = segmentB[0] = 0.0;
   err = 0;
  }
  else if (dim==2) {
    err = DMPlaneVectors_2D_Internal(x0, n, offset, segmentA);
    segmentB[0] = segmentB[1] = 0.0;
  }
  else {
    err = DMPlaneVectors_3D_Internal(x0, n, offset, segmentA, segmentB);
  }

  PetscFunctionReturn(err);

}


// These are internal PETSc code that we need. From dm/impls/plex/plexgeometry.c
PetscErrorCode DMPlexGetLineIntersection_2D_Internal(const PetscReal segmentA[], const PetscReal segmentB[], PetscReal intersection[], PetscBool *hasIntersection)
{
  const PetscReal p0_x  = segmentA[0*2+0];
  const PetscReal p0_y  = segmentA[0*2+1];
  const PetscReal p1_x  = segmentA[1*2+0];
  const PetscReal p1_y  = segmentA[1*2+1];
  const PetscReal p2_x  = segmentB[0*2+0];
  const PetscReal p2_y  = segmentB[0*2+1];
  const PetscReal p3_x  = segmentB[1*2+0];
  const PetscReal p3_y  = segmentB[1*2+1];
  const PetscReal s1_x  = p1_x - p0_x;
  const PetscReal s1_y  = p1_y - p0_y;
  const PetscReal s2_x  = p3_x - p2_x;
  const PetscReal s2_y  = p3_y - p2_y;
  const PetscReal denom = (-s2_x * s1_y + s1_x * s2_y);

  PetscFunctionBegin;
  *hasIntersection = PETSC_FALSE;
  /* Non-parallel lines */
  if (denom != 0.0) {
    const PetscReal s = (-s1_y * (p0_x - p2_x) + s1_x * (p0_y - p2_y)) / denom;
    const PetscReal t = ( s2_x * (p0_y - p2_y) - s2_y * (p0_x - p2_x)) / denom;

    if (s >= 0 && s <= 1 && t >= 0 && t <= 1) {
      *hasIntersection = PETSC_TRUE;
      if (intersection) {
        intersection[0] = p0_x + (t * s1_x);
        intersection[1] = p0_y + (t * s1_y);
      }
    }
  }
  PetscFunctionReturn(0);
}



/* The plane is segmentB x segmentC: https://en.wikipedia.org/wiki/Line%E2%80%93plane_intersection */
PetscErrorCode DMPlexGetLinePlaneIntersection_3D_Internal(const PetscReal segmentA[], const PetscReal segmentB[], const PetscReal segmentC[], PetscReal intersection[], PetscBool *hasIntersection)
{
  const PetscReal p0_x  = segmentA[0*3+0];
  const PetscReal p0_y  = segmentA[0*3+1];
  const PetscReal p0_z  = segmentA[0*3+2];
  const PetscReal p1_x  = segmentA[1*3+0];
  const PetscReal p1_y  = segmentA[1*3+1];
  const PetscReal p1_z  = segmentA[1*3+2];
  const PetscReal q0_x  = segmentB[0*3+0];
  const PetscReal q0_y  = segmentB[0*3+1];
  const PetscReal q0_z  = segmentB[0*3+2];
  const PetscReal q1_x  = segmentB[1*3+0];
  const PetscReal q1_y  = segmentB[1*3+1];
  const PetscReal q1_z  = segmentB[1*3+2];
  const PetscReal r0_x  = segmentC[0*3+0];
  const PetscReal r0_y  = segmentC[0*3+1];
  const PetscReal r0_z  = segmentC[0*3+2];
  const PetscReal r1_x  = segmentC[1*3+0];
  const PetscReal r1_y  = segmentC[1*3+1];
  const PetscReal r1_z  = segmentC[1*3+2];
  const PetscReal s0_x  = p1_x - p0_x;
  const PetscReal s0_y  = p1_y - p0_y;
  const PetscReal s0_z  = p1_z - p0_z;
  const PetscReal s1_x  = q1_x - q0_x;
  const PetscReal s1_y  = q1_y - q0_y;
  const PetscReal s1_z  = q1_z - q0_z;
  const PetscReal s2_x  = r1_x - r0_x;
  const PetscReal s2_y  = r1_y - r0_y;
  const PetscReal s2_z  = r1_z - r0_z;
  const PetscReal s3_x  = s1_y*s2_z - s1_z*s2_y; /* s1 x s2 */
  const PetscReal s3_y  = s1_z*s2_x - s1_x*s2_z;
  const PetscReal s3_z  = s1_x*s2_y - s1_y*s2_x;
  const PetscReal s4_x  = s0_y*s2_z - s0_z*s2_y; /* s0 x s2 */
  const PetscReal s4_y  = s0_z*s2_x - s0_x*s2_z;
  const PetscReal s4_z  = s0_x*s2_y - s0_y*s2_x;
  const PetscReal s5_x  = s1_y*s0_z - s1_z*s0_y; /* s1 x s0 */
  const PetscReal s5_y  = s1_z*s0_x - s1_x*s0_z;
  const PetscReal s5_z  = s1_x*s0_y - s1_y*s0_x;
  const PetscReal denom = -(s0_x*s3_x + s0_y*s3_y + s0_z*s3_z); /* -s0 . (s1 x s2) */

  PetscFunctionBegin;
  *hasIntersection = PETSC_FALSE;
  /* Line not parallel to plane */
  if (denom != 0.0) {
    const PetscReal t = (s3_x * (p0_x - q0_x) + s3_y * (p0_y - q0_y) + s3_z * (p0_z - q0_z)) / denom;
    const PetscReal u = (s4_x * (p0_x - q0_x) + s4_y * (p0_y - q0_y) + s4_z * (p0_z - q0_z)) / denom;
    const PetscReal v = (s5_x * (p0_x - q0_x) + s5_y * (p0_y - q0_y) + s5_z * (p0_z - q0_z)) / denom;

    if (t >= 0 && t <= 1 && u >= 0 && u <= 1 && v >= 0 && v <= 1) {
      *hasIntersection = PETSC_TRUE;
      if (intersection) {
        intersection[0] = p0_x + (t * s0_x);
        intersection[1] = p0_y + (t * s0_y);
        intersection[2] = p0_z + (t * s0_z);
      }
    }
  }
  PetscFunctionReturn(0);
}

