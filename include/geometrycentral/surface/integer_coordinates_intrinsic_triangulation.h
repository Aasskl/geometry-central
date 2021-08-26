#pragma once

#include "geometrycentral/surface/barycentric_coordinate_helpers.h"
#include "geometrycentral/surface/common_subdivision.h"
#include "geometrycentral/surface/intrinsic_geometry_interface.h"
#include "geometrycentral/surface/intrinsic_mollification.h"
#include "geometrycentral/surface/intrinsic_triangulation.h"
#include "geometrycentral/surface/manifold_surface_mesh.h"
#include "geometrycentral/surface/normal_coordinates.h"
#include "geometrycentral/surface/surface_point.h"
#include "geometrycentral/surface/trace_geodesic.h"
#include "geometrycentral/utilities/elementary_geometry.h"

#include <deque>

namespace geometrycentral {
namespace surface {

class IntegerCoordinatesIntrinsicTriangulation : public IntrinsicTriangulation {

public:
  // Construct an intrinsic triangulation which sits atop this input mesh.
  // Initially, the input triangulation will just be a copy of the input mesh.
  IntegerCoordinatesIntrinsicTriangulation(ManifoldSurfaceMesh& mesh, IntrinsicGeometryInterface& inputGeom);

  // ======================================================
  //                   Core Members
  // ======================================================

  // The actual normal coordinates (and roundabouts) encoding the triangulation. These normal coordinates are defined on
  // top of the _intrinsic_ mesh---for each intrinsic edge, the encode how many original edges cross it.
  NormalCoordinates normalCoordinates;

  // ======================================================
  // ======== Queries & Accessors
  // ======================================================

  std::vector<SurfacePoint> traceHalfedge(Halfedge he) override;
  
  std::unique_ptr<CommonSubdivision> extractCommonSubdivision() override;

  SurfacePoint equivalentPointOnIntrinsic(const SurfacePoint& pointOnInput) override;

  SurfacePoint equivalentPointOnInput(const SurfacePoint& pointOnIntrinsic) override;

  // ======================================================
  // ======== Low-Level Mutators
  // ======================================================

  // If the edge is not Delaunay, flip it. Returns true if flipped.
  bool flipEdgeIfNotDelaunay(Edge e) override;

  // If the edge can be flipped, flip it (must be combinatorially flippable and inside a convex quad). Returns true if
  // flipped.
  bool flipEdgeIfPossible(Edge e) override;

  // Flip an edge, where the caller specifies geometric data for the updated edge, rather than it being computed. Must
  // be flippable. Experts only.
  void flipEdgeManual(Edge e, double newLength, double forwardAngle, double reverseAngle, bool isOrig,
                      bool reverseFlip = false) override;

  // Insert a new vertex in to the intrinsic triangulation
  Vertex insertVertex(SurfacePoint newPositionOnIntrinsic) override;

  // Remove an (inserted) vertex from the triangulation.
  // Note: if something goes terribly (numerically?) wrong, will exit without removing the vertex.
  Face removeInsertedVertex(Vertex v) override;

  // Split an edge
  Halfedge splitEdge(Halfedge he, double tSplit) override;

  // Basic operations to modify the intrinsic triangulation
  // NOTE: The individual operations do not call refreshQuantities(), so you
  // should call it if you want quantities updated.

  // If the edge is not Delaunay, flip it. Returns true if flipped.
  double checkFlip(Edge e);

  // Insert circumcenter or split segment, from NIT, geometrycentral
  // TODO: do edge encroachment properly?
  Vertex insertCircumcenterOrSplitSegment(Face f, bool verbose = false);

  Vertex splitFace(Face f, Vector3 bary, bool verbose = false);
  Vertex splitEdge(Edge e, double bary, bool verbose = false);
  Vertex splitInteriorEdge(Edge e, double bary, bool verbose = false);
  Vertex splitBoundaryEdge(Edge e, double bary, bool verbose = false);

  // Move a vertex `v` in direction `vec`, represented as a vector in the
  // vertex's tangent space.
  Vertex moveVertex(Vertex v, Vector2 vec);

  // Assumes intrinsicEdgeLengths is up to date
  void updateCornerAngle(Corner c);

  // Assumes cornerAngles, vertexAngleSums exist and are up to date
  void updateHalfedgeVectorsInVertex(Vertex v);

  // ======================================================
  //                Low-Level Queries
  // ======================================================

  // Takes in a halfedge of the intrinsic mesh whose edge's normal coordinate
  // is negative (meaning that it lies along an edge of the input mesh) and
  // returns the halfedge in the input mesh pointing in the same direction
  // e.vertex() must live in both meshes
  Halfedge getSharedInputEdge(Halfedge e) const;

  // Compute the number of vertices in the common subdivision
  // i.e. intrinsicMesh->nVertices() plus the sum of the normal coordinates
  size_t nSubdividedVertices() const;

  // HACK: represents arcs parallel to a mesh edge with a single pair {-n,
  // he} where n is the number of arcs parallel to he.edge() Trace an edge
  // of the input mesh over the intrinsic triangulation
  NormalCoordinatesCompoundCurve traceInputEdge(Edge e, bool verbose = false) const;

  std::pair<bool, NormalCoordinatesCurve> traceNextCurve(const NormalCoordinatesCurve& oldCurve,
                                                         bool verbose = false) const;

  // Inverse to traceInputEdge
  Halfedge identifyInputEdge(const NormalCoordinatesCurve& path, bool verbose = false) const;

  // Identify shared halfedge, throw exception if halfedge is not shared
  // (i.e. edgeCoords[he.edge()] must be negative)
  Halfedge identifyInputEdge(Halfedge he) const;

  std::array<Vector2, 3> vertexCoordinatesInFace(Face face) const;

  void setFixedEdges(const EdgeData<bool>& fixedEdges);

  // If f is entirely contained in some face of the input mesh, return that
  // face Otherwise return Face()
  Face getParentFace(Face f) const;

private:
  // Implementation details
};

// Compute the cotan weight of halfedge ij in terms of the lengths of its
// neighbors
double halfedgeCotanWeight(double lij, double ljk, double lki);

FaceData<Vector2> interpolateTangentVectorsB(const IntegerCoordinatesIntrinsicTriangulation& tri,
                                             const CommonSubdivision& cs, const FaceData<Vector2>& dataB);


} // namespace surface
} // namespace geometrycentral

#include "integer_coordinates_intrinsic_triangulation.ipp"
