#pragma once

#include "geometrycentral/surface/manifold_surface_mesh.h"
#include "geometrycentral/surface/surface_point.h"
#include "geometrycentral/surface/vertex_position_geometry.h"


namespace geometrycentral {
namespace surface {

/*
  This class provides a wide variety of functionality to

  Unlike much of geometry-central, which abstractly separates the idea of a mesh's connectivity and geometry, this class
  explicitly handles the common case of a manifold triangle mesh with 3D vertex positions.

  // TODO no reason this should be limited to manifold
*/
class MutationManager {


public:
  // ======================================================
  // ======== Members
  // ======================================================
  //
  // Core members which define the class.

  ManifoldSurfaceMesh& mesh;
  VertexPositionGeometry& geometry;

  // ======================================================
  // ======== Construtors
  // ======================================================

  // Create a new mutation manager, which modifies the mesh and geometry in-place.
  MutationManager(ManifoldSurfaceMesh& mesh, VertexPositionGeometry& geometry);

  // ======================================================
  // ======== Low-level mutations
  // ======================================================
  //
  // Low-level routines which modify the mesh, updating connectivity as well as geometry and invoking any relevant
  // callbacks.
  //
  // Note: every one of these operations corresponds to a callback below. We should probably be wary about adding too
  // many of these, since each adds the burden of a corresponding callback policy to update data.

  // Move a vertex in 3D space
  void repositionVertex(Vertex vert, Vector3 offset);

  // Flip an edge.
  bool flipEdge(Edge e);

  // Split an edge.
  // `tSplit` controls the split location from [0,1]; 0 splits at d.halfedge().tailVertex().
  // `newVertexPosition` can be specified to manually control the insertion location, rather than linearly interpolating
  // to `tSplit`.
  // In general, both the `tSplit` and the `newVertexPosition` are used; `tSplit` is necessary to allow callbacks to
  // interpolate data; if called with either the other will be inferred.
  void splitEdge(Edge e, double tSplit);
  void splitEdge(Edge e, Vector3 newVertexPosition);
  void splitEdge(Edge e, double tSplit, Vector3 newVertexPosition);

  // Collapse an edge.
  // Returns true if the edge could actually be collapsed.
  bool collapseEdge(Edge e, double tCollapse);
  bool collapseEdge(Edge e, Vector3 newVertexPosition);
  bool collapseEdge(Edge e, double tCollapse, Vector3 newVertexPosition);

  // ======================================================
  // ======== High-level mutations
  // ======================================================
  //
  // High-level routines which perform many low-level operations to modify a mesh

  // Improve mesh quality by alternating between perturbing vertices in their tangent plane, and splitting/collapsing
  // poorly-sized edges.
  // void isotopicRemesh();

  // Simplify the the mesh by performing edge collapses to minimize error in the sense of the quadric error metric.
  // see [Garland & Heckbert 1997] "Surface Simplification Using Quadric Error Metrics"
  // void quadricErrorSimplify(size_t nTargetVerts);

  // Split edges in the mesh until all edges satisfy the Delaunay criterion
  // see e.g. [Liu+ 2015] "Efficient Construction and Simplification of Delaunay Meshes"
  // void splitEdgesToDelaunay();


  // ======================================================
  // ======== Callbacks
  // ======================================================
  //
  // Get called whenever mesh mutations occur. Register a callback by inserting it in to this list.


  // == Automatic interface

  // These help you keep buffers of data up to date while modifying a mesh. Internally, they register functions for all
  // of the callbacks above to implement some policy.
  //
  // TODO need some way to stop managing and remove from the callback lists. Perhaps return a token class that
  // de-registers on destruction?

  // Update scalar values at vertices,
  void managePointwiseScalarData(VertexData<double>& data);

  // Update a paramterization
  // void manageParameterization(CornerData<Vector2>& uvData);

  // Update integrated data at faces (scalars which have been integrated over a face, like area)
  // void manageIntegratedScalarData(FaceData<double>& data);


  // == Manual interface: add callbacks on to these lists to do whatever you want.

  // These callbacks are invoked _after_ the operation is performed.
  // TODO possible problem. These callbacks either need to be called after the operation or before it, but neither is
  // obviously the right solution... Offering both seems excessive.

  // Vertex v is repositioned with offset Vector3
  std::list<std::function<void(Vertex, Vector3)>> repositionVertexCallbackList;

  // edge E if flipped
  std::list<std::function<void(Edge)>> edgeFlipCallbackList;

  // old edge E is split to halfedge HE1,HE2 both with he.vertex() as split vertex, new vertex at double tSplit
  // TODO be very precise here about meaning of parameters
  std::list<std::function<void(Halfedge, Halfedge, double)>> edgeSplitCallbackList;

  // old edge E is collapsed to vertex V, inserted at double tCollapse
  std::list<std::function<void(Edge, Vertex, double)>> edgeCollapseCallbackList;

  // ======================================================
  // ======== Utilities
  // ======================================================


private:
  VertexData<Vector3>& pos; // an alias for geometry->inputVertexPositions, solely for notational convenience
};

} // namespace surface
} // namespace geometrycentral
