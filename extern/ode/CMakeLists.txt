IF(${OS} STREQUAL "wince")
	ADD_DEFINITIONS(
		-Dcosf=cos
		-Dsinf=sin
		-Dasinf=asin
		-Dacosf=acos
	)
ENDIF(${OS} STREQUAL "wince")

IF(${OS} STREQUAL "qnx")
	ADD_DEFINITIONS(-D__linux__)
ENDIF(${OS} STREQUAL "qnx")

IF(${OS} STREQUAL "linux")
	ADD_DEFINITIONS(-D__linux__)
ENDIF(${OS} STREQUAL "linux")

ADD_DEFINITIONS(
	-DODE_LIB
)

ADD_LIBRARY(ode STATIC
	array.cpp
	box.cpp
	capsule.cpp
	collision_cylinder_box.cpp
	collision_cylinder_plane.cpp
	collision_cylinder_sphere.cpp
	collision_cylinder_trimesh.cpp
	collision_kernel.cpp
	collision_quadtreespace.cpp
	collision_sapspace.cpp
	collision_space.cpp
	collision_transform.cpp
	collision_trimesh_box.cpp
	collision_trimesh_ccylinder.cpp
	collision_trimesh_disabled.cpp
	collision_trimesh_distance.cpp
	collision_trimesh_gimpact.cpp
	collision_trimesh_opcode.cpp
	collision_trimesh_plane.cpp
	collision_trimesh_ray.cpp
	collision_trimesh_sphere.cpp
	collision_trimesh_trimesh.cpp
	collision_trimesh_trimesh_new.cpp
	collision_util.cpp
	convex.cpp
	cylinder.cpp
	error.cpp
	export-dif.cpp
	fastdot.c
	fastldlt.c
	fastlsolve.c
	fastltsolve.c
	heightfield.cpp
	lcp.cpp
	mass.cpp
	mat.cpp
	matrix.cpp
	memory.cpp
	misc.cpp
	obstack.cpp
	ode.cpp
	odeinit.cpp
	odemath.cpp
	odeou.cpp
	odetls.cpp
	plane.cpp
	quickstep.cpp
	ray.cpp
	rotation.cpp
	sphere.cpp
	step.cpp
	stepfast.cpp
	testing.cpp
	timer.cpp
	util.cpp
	joints/amotor.cpp
	joints/ball.cpp
	joints/contact.cpp
	joints/fixed.cpp
	joints/hinge.cpp
	joints/hinge2.cpp
	joints/joint.cpp
	joints/lmotor.cpp
	joints/null.cpp
	joints/piston.cpp
	joints/plane2d.cpp
	joints/pr.cpp
	joints/pu.cpp
	joints/slider.cpp
	joints/universal.cpp
	OPC_AABBCollider.cpp
	OPC_AABBTree.cpp
	OPC_BaseModel.cpp
	OPC_BoxPruning.cpp
	OPC_Collider.cpp
	OPC_Common.cpp
	OPC_HybridModel.cpp
	OPC_LSSCollider.cpp
	OPC_MeshInterface.cpp
	OPC_Model.cpp
	OPC_OBBCollider.cpp
	OPC_OptimizedTree.cpp
	OPC_Picking.cpp
	OPC_PlanesCollider.cpp
	OPC_RayCollider.cpp
	OPC_SphereCollider.cpp
	OPC_SweepAndPrune.cpp
	OPC_ThreadLocalData.cpp
	OPC_TreeBuilders.cpp
	OPC_TreeCollider.cpp
	OPC_VolumeCollider.cpp
	Opcode.cpp
	Ice/IceAABB.cpp
	Ice/IceContainer.cpp
	Ice/IceHPoint.cpp
	Ice/IceIndexedTriangle.cpp
	Ice/IceMatrix3x3.cpp
	Ice/IceMatrix4x4.cpp
	Ice/IceOBB.cpp
	Ice/IcePlane.cpp
	Ice/IcePoint.cpp
	Ice/IceRandom.cpp
	Ice/IceRay.cpp
	Ice/IceRevisitedRadix.cpp
	Ice/IceSegment.cpp
	Ice/IceTriangle.cpp
	Ice/IceUtils.cpp
)

INSTALL(TARGETS ode ARCHIVE DESTINATION Engine/lib)