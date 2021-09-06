#include "geometrycentral/surface/integer_coordinates_intrinsic_triangulation.h"
#include "geometrycentral/surface/intrinsic_triangulation.h"
#include "geometrycentral/surface/manifold_surface_mesh.h"
#include "geometrycentral/surface/meshio.h"
#include "geometrycentral/surface/signpost_intrinsic_triangulation.h"
#include "geometrycentral/surface/transfer_functions.h"
#include "geometrycentral/surface/vertex_position_geometry.h"

#include "load_test_meshes.h"

#include "gtest/gtest.h"

#include <iostream>
#include <string>
#include <unordered_set>

using namespace geometrycentral;
using namespace geometrycentral::surface;

class IntrinsicTriangulationSuite : public MeshAssetSuite {};

TEST_F(IntrinsicTriangulationSuite, SignpostFlip) {
  for (const MeshAsset& a : {getAsset("fox.ply", true)}) {
    a.printThyName();
    ManifoldSurfaceMesh& mesh = *a.manifoldMesh;
    VertexPositionGeometry& origGeometry = *a.geometry;

    SignpostIntrinsicTriangulation tri(mesh, origGeometry);

    tri.flipToDelaunay();

    EXPECT_TRUE(tri.isDelaunay());
  }
}

TEST_F(IntrinsicTriangulationSuite, IntegerFlip) {
  for (const MeshAsset& a : {getAsset("fox.ply", true)}) {
    a.printThyName();
    ManifoldSurfaceMesh& mesh = *a.manifoldMesh;
    VertexPositionGeometry& origGeometry = *a.geometry;

    IntegerCoordinatesIntrinsicTriangulation tri(mesh, origGeometry);

    tri.flipToDelaunay();

    EXPECT_TRUE(tri.isDelaunay());
  }
}

TEST_F(IntrinsicTriangulationSuite, DelaunayTriangulationsAgree) {
  for (const MeshAsset& a : {getAsset("fox.ply", true)}) {
    a.printThyName();
    ManifoldSurfaceMesh& mesh = *a.manifoldMesh;
    VertexPositionGeometry& origGeometry = *a.geometry;

    IntegerCoordinatesIntrinsicTriangulation tri_int(mesh, origGeometry);
    SignpostIntrinsicTriangulation tri_sign(mesh, origGeometry);

    tri_int.flipToDelaunay();
    tri_sign.flipToDelaunay();

    for (size_t iE = 0; iE < tri_int.intrinsicMesh->nEdges(); iE++) {
      double l_int = tri_int.edgeLengths[tri_int.intrinsicMesh->edge(iE)];
      double l_sign = tri_sign.edgeLengths[tri_sign.intrinsicMesh->edge(iE)];
      EXPECT_NEAR(l_int, l_sign, 1e-5);
    }
  }
}


TEST_F(IntrinsicTriangulationSuite, SignpostTrace) {
  for (const MeshAsset& a : {getAsset("fox.ply", true)}) {
    a.printThyName();
    ManifoldSurfaceMesh& mesh = *a.manifoldMesh;
    VertexPositionGeometry& origGeometry = *a.geometry;

    SignpostIntrinsicTriangulation tri(mesh, origGeometry);

    tri.flipToDelaunay();

    EdgeData<std::vector<SurfacePoint>> out = tri.traceAllIntrinsicEdgesAlongInput();
    for (Edge e : tri.mesh.edges()) {
      EXPECT_GE(out[e].size(), 2);
    }
  }
}

TEST_F(IntrinsicTriangulationSuite, IntegerTrace) {
  for (const MeshAsset& a : {getAsset("fox.ply", true)}) {
    a.printThyName();
    ManifoldSurfaceMesh& mesh = *a.manifoldMesh;
    VertexPositionGeometry& origGeometry = *a.geometry;

    IntegerCoordinatesIntrinsicTriangulation tri(mesh, origGeometry);

    tri.flipToDelaunay();

    EdgeData<std::vector<SurfacePoint>> out = tri.traceAllIntrinsicEdgesAlongInput();
    for (Edge e : tri.mesh.edges()) {
      EXPECT_EQ(out[e].size(), std::max(0, tri.normalCoordinates[e]) + 2);
    }
  }
}

TEST_F(IntrinsicTriangulationSuite, IntegerEdgeTraceAgreesWithBulk) {
  for (const MeshAsset& a : {getAsset("fox.ply", true)}) {
    a.printThyName();
    ManifoldSurfaceMesh& mesh = *a.manifoldMesh;
    VertexPositionGeometry& origGeometry = *a.geometry;

    IntegerCoordinatesIntrinsicTriangulation tri(mesh, origGeometry);

    tri.flipToDelaunay();

    // Traced individually
    EdgeData<std::vector<SurfacePoint>> out1(tri.inputMesh);
    for (Edge e : tri.inputMesh.edges()) {
      out1[e] = tri.traceInputHalfedgeAlongIntrinsic(e.halfedge());
    }

    // Traced via common subdivision
    EdgeData<std::vector<SurfacePoint>> out2 = tri.traceAllInputEdgesAlongIntrinsic();

    for (Edge e : tri.inputMesh.edges()) {
      EXPECT_EQ(out1[e].size(), out2[e].size());
      for (size_t iP = 0; iP < out1[e].size(); iP++) {
        EXPECT_EQ(out1[e][iP].type, out2[e][iP].type);
        switch (out1[e][iP].type) {
        case SurfacePointType::Vertex:
          EXPECT_EQ(out1[e][iP].vertex, out2[e][iP].vertex);
          break;
        case SurfacePointType::Edge:
          EXPECT_EQ(out1[e][iP].edge, out2[e][iP].edge);
          EXPECT_NEAR(out1[e][iP].tEdge, out2[e][iP].tEdge, 1e-5);
          break;
        case SurfacePointType::Face:
          EXPECT_EQ(out1[e][iP].face, out2[e][iP].face);
          EXPECT_NEAR((out1[e][iP].faceCoords - out2[e][iP].faceCoords).norm(), 0, 1e-5);
          break;
        }
      }
    }
  }
}


TEST_F(IntrinsicTriangulationSuite, SignpostRefine) {
  for (const MeshAsset& a : {getAsset("fox.ply", true)}) {
    a.printThyName();
    ManifoldSurfaceMesh& mesh = *a.manifoldMesh;
    VertexPositionGeometry& origGeometry = *a.geometry;

    SignpostIntrinsicTriangulation tri(mesh, origGeometry);

    tri.delaunayRefine();
    EXPECT_TRUE(tri.isDelaunay());

    // (technically on some meshes no insertions may be needed, but for this test lets choose meshes that do need it)
    EXPECT_GT(tri.mesh.nVertices(), tri.inputMesh.nVertices());

    // (technically we should check the minimum angle away from needle-like vertices)
    EXPECT_GE(tri.minAngleDegrees(), 25);
  }
}

TEST_F(IntrinsicTriangulationSuite, IntegerRefine) {
  for (const MeshAsset& a : {getAsset("fox.ply", true)}) {
    a.printThyName();
    ManifoldSurfaceMesh& mesh = *a.manifoldMesh;
    VertexPositionGeometry& origGeometry = *a.geometry;

    IntegerCoordinatesIntrinsicTriangulation tri(mesh, origGeometry);

    tri.delaunayRefine();
    EXPECT_TRUE(tri.isDelaunay());

    // (technically on some meshes no insertions may be needed, but for this test lets choose meshes that do need it)
    EXPECT_GT(tri.mesh.nVertices(), tri.inputMesh.nVertices());

    // (technically we should check the minimum angle away from needle-like vertices)
    EXPECT_GE(tri.minAngleDegrees(), 25);
  }
}

/* TODO not implemented
TEST_F(IntrinsicTriangulationSuite, SignpostCommonSubdivision) {
  for (const MeshAsset& a : {getAsset("fox.ply", true)}) {
    a.printThyName();
    ManifoldSurfaceMesh& mesh = *a.manifoldMesh;
    VertexPositionGeometry& origGeometry = *a.geometry;

    SignpostIntrinsicTriangulation tri(mesh, origGeometry);

    tri.delaunayRefine();
    CommonSubdivision& cs = tri.getCommonSubdivision();
    cs.constructMesh();

    EXPECT_GT(cs.mesh->nVertices(), tri.inputMesh.nVertices());
    EXPECT_GT(cs.mesh->nVertices(), tri.mesh.nVertices());
  }
}
*/

TEST_F(IntrinsicTriangulationSuite, IntegerCommonSubdivision) {
  for (const MeshAsset& a : {getAsset("fox.ply", true)}) {
    a.printThyName();
    ManifoldSurfaceMesh& mesh = *a.manifoldMesh;
    VertexPositionGeometry& origGeometry = *a.geometry;

    IntegerCoordinatesIntrinsicTriangulation tri(mesh, origGeometry);

    tri.delaunayRefine();
    CommonSubdivision& cs = tri.getCommonSubdivision();
    bool triangulate = false;
    cs.constructMesh(triangulate);

    EXPECT_GT(cs.mesh->nVertices(), tri.inputMesh.nVertices());
    EXPECT_GT(cs.mesh->nVertices(), tri.mesh.nVertices());

    // Check element counts against a few ways of computing them
    size_t nV, nE, nF;
    std::tie(nV, nE, nF) = cs.elementCounts();

    EXPECT_EQ(cs.mesh->nVertices(), nV);
    EXPECT_EQ(cs.mesh->nEdges(), nE);
    EXPECT_EQ(cs.mesh->nFaces(), nF);

    size_t nV_normal = tri.intrinsicMesh->nVertices();
    for (Edge e : tri.intrinsicMesh->edges()) {
      nV_normal += fmax(0, tri.normalCoordinates[e]);
    }

    EXPECT_EQ(cs.mesh->nVertices(), nV_normal);
  }
}

// TODO test signpost and integer against each other to verify they give same results

// TODO: also test with signposts?
TEST_F(IntrinsicTriangulationSuite, FunctionTransfer) {
  for (const MeshAsset& a : {getAsset("fox.ply", true)}) {
    a.printThyName();
    ManifoldSurfaceMesh& mesh = *a.manifoldMesh;
    VertexPositionGeometry& origGeometry = *a.geometry;

    IntegerCoordinatesIntrinsicTriangulation tri(mesh, origGeometry);

    tri.delaunayRefine();
    CommonSubdivision& cs = tri.getCommonSubdivision();

    AttributeTransfer transfer(cs, origGeometry);
    VertexData<double> data_B(*tri.intrinsicMesh, Vector<double>::Random(tri.intrinsicMesh->nVertices()));
    VertexData<double> data_A_Pointwise = transfer.transferBtoA(data_B, TransferMethod::Pointwise);
    VertexData<double> data_A_L2 = transfer.transferBtoA(data_B, TransferMethod::L2);
    Vector<double> truth = transfer.P_B * data_B.toVector();
    Vector<double> pointwiseA = transfer.P_A * data_A_Pointwise.toVector();
    Vector<double> L2A = transfer.P_A * data_A_L2.toVector();

    double pointwiseErr = (pointwiseA - truth).dot(transfer.M_CS_Galerkin * (pointwiseA - truth));
    double L2Err = (L2A - truth).dot(transfer.M_CS_Galerkin * (L2A - truth));

    EXPECT_LE(L2Err, pointwiseErr);

    SparseMatrix<double> lhs, rhs;
    std::tie(lhs, rhs) = transfer.constructBtoAMatrices();
    Vector<double> residual = lhs * data_A_L2.toVector() - rhs * data_B.toVector();
    EXPECT_LE(residual.norm(), 1e-6);
  }
}

TEST_F(IntrinsicTriangulationSuite, CommonSubdivisionGeometry) {
  for (const MeshAsset& a : {getAsset("fox.ply", true)}) {
    a.printThyName();
    ManifoldSurfaceMesh& mesh = *a.manifoldMesh;
    VertexPositionGeometry& origGeometry = *a.geometry;

    IntegerCoordinatesIntrinsicTriangulation tri(mesh, origGeometry);

    tri.delaunayRefine();
    CommonSubdivision& cs = tri.getCommonSubdivision();
    cs.constructMesh();

    // == Edge lengths
    // Lengths from extrinsic vertex positions
    const VertexData<Vector3>& posCS = cs.interpolateAcrossA(origGeometry.vertexPositions);
    VertexPositionGeometry csGeo(*cs.mesh, posCS);
    csGeo.requireEdgeLengths();
    EdgeData<double> lengthsFromPosA = csGeo.edgeLengths;
    csGeo.unrequireEdgeLengths();

    // Lengths from extrinsic edge lengths
    origGeometry.requireEdgeLengths();
    const EdgeData<double>& lengthsA = origGeometry.edgeLengths;
    EdgeData<double> lengthsFromLenA = cs.interpolateEdgeLengthsA(lengthsA);

    // Lengths from intrinsic edge lengths
    EdgeData<double> lengthsFromLenB = cs.interpolateEdgeLengthsB(tri.edgeLengths);

    EXPECT_NEAR((lengthsFromPosA.toVector() - lengthsFromLenA.toVector()).norm(), 0, 1e-5);
    EXPECT_NEAR((lengthsFromPosA.toVector() - lengthsFromLenB.toVector()).norm(), 0, 1e-5);
    EXPECT_NEAR((lengthsFromLenA.toVector() - lengthsFromLenB.toVector()).norm(), 0, 1e-5);
  }
}
