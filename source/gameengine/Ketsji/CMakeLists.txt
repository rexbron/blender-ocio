# $Id$
# ***** BEGIN GPL LICENSE BLOCK *****
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#
# The Original Code is Copyright (C) 2006, Blender Foundation
# All rights reserved.
#
# The Original Code is: all of this file.
#
# Contributor(s): Jacques Beaurain.
#
# ***** END GPL LICENSE BLOCK *****

set(INC
	.
	KXNetwork
	../BlenderRoutines
	../Converter
	../Expressions
	../GameLogic
	../Network
	../Network/LoopBackNetwork
	../Physics/common
	../Rasterizer
	../Rasterizer/RAS_OpenGLRasterizer
	../SceneGraph
	../../blender
	../../blender/blenfont
	../../blender/blenkernel
	../../blender/blenlib
	../../blender/blenloader
	../../blender/gpu
	../../blender/imbuf
	../../blender/makesdna
	../../blender/python
	../../blender/python/generic
	../../blender/python/mathutils
	../../../intern/container
	../../../intern/guardedalloc
	../../../intern/moto/include
	../../../intern/string
)

set(INC_SYS
	${GLEW_INCLUDE_PATH}
)

set(SRC
	BL_BlenderShader.cpp
	BL_Material.cpp
	BL_Shader.cpp
	BL_Texture.cpp
	KX_ArmatureSensor.cpp
	KX_BlenderMaterial.cpp
	KX_BulletPhysicsController.cpp
	KX_Camera.cpp
	KX_CameraActuator.cpp
	KX_CameraIpoSGController.cpp
	KX_ConstraintActuator.cpp
	KX_ConstraintWrapper.cpp
	KX_ConvertPhysicsObjects.cpp
	KX_Dome.cpp
	KX_EmptyObject.cpp
	KX_FontObject.cpp
	KX_GameActuator.cpp
	KX_GameObject.cpp
	KX_IPO_SGController.cpp
	KX_IPhysicsController.cpp
	KX_IpoActuator.cpp
	KX_KetsjiEngine.cpp
	KX_Light.cpp
	KX_LightIpoSGController.cpp
	KX_MaterialIpoController.cpp
	KX_MeshProxy.cpp
	KX_MotionState.cpp
	KX_MouseFocusSensor.cpp
	KX_NearSensor.cpp
	KX_ObColorIpoSGController.cpp
	KX_ObjectActuator.cpp
	KX_OrientationInterpolator.cpp
	KX_ParentActuator.cpp
	KX_PhysicsObjectWrapper.cpp
	KX_PolyProxy.cpp
	KX_PolygonMaterial.cpp
	KX_PositionInterpolator.cpp
	KX_PyConstraintBinding.cpp
	KX_PyMath.cpp
	KX_PythonInit.cpp
	KX_PythonInitTypes.cpp
	KX_PythonSeq.cpp
	KX_RadarSensor.cpp
	KX_RayCast.cpp
	KX_RayEventManager.cpp
	KX_RaySensor.cpp
	KX_SCA_AddObjectActuator.cpp
	KX_SCA_DynamicActuator.cpp
	KX_SCA_EndObjectActuator.cpp
	KX_SCA_ReplaceMeshActuator.cpp
	KX_SG_BoneParentNodeRelationship.cpp
	KX_SG_NodeRelationships.cpp
	KX_ScalarInterpolator.cpp
	KX_ScalingInterpolator.cpp
	KX_Scene.cpp
	KX_SceneActuator.cpp
	KX_SoundActuator.cpp
	KX_StateActuator.cpp
	KX_TimeCategoryLogger.cpp
	KX_TimeLogger.cpp
	KX_TouchEventManager.cpp
	KX_TouchSensor.cpp
	KX_TrackToActuator.cpp
	KX_VehicleWrapper.cpp
	KX_VertexProxy.cpp
	KX_VisibilityActuator.cpp
	KX_WorldInfo.cpp
	KX_WorldIpoController.cpp

	BL_BlenderShader.h
	BL_Material.h
	BL_Shader.h
	BL_Texture.h
	KX_ArmatureSensor.h
	KX_BlenderMaterial.h
	KX_BulletPhysicsController.h
	KX_Camera.h
	KX_CameraActuator.h
	KX_CameraIpoSGController.h
	KX_ClientObjectInfo.h
	KX_ConstraintActuator.h
	KX_ConstraintWrapper.h
	KX_ConvertPhysicsObject.h
	KX_Dome.h
	KX_EmptyObject.h
	KX_FontObject.h
	KX_GameActuator.h
	KX_GameObject.h
	KX_IInterpolator.h
	KX_IPOTransform.h
	KX_IPO_SGController.h
	KX_IPhysicsController.h
	KX_IScalarInterpolator.h
	KX_ISceneConverter.h
	KX_ISystem.h
	KX_IpoActuator.h
	KX_KetsjiEngine.h
	KX_Light.h
	KX_LightIpoSGController.h
	KX_MaterialIpoController.h
	KX_MeshProxy.h
	KX_MotionState.h
	KX_MouseFocusSensor.h
	KX_NearSensor.h
	KX_ObColorIpoSGController.h
	KX_ObjectActuator.h
	KX_OrientationInterpolator.h
	KX_ParentActuator.h
	KX_PhysicsEngineEnums.h
	KX_PhysicsObjectWrapper.h
	KX_PhysicsPropertiesobsolete.h
	KX_PolyProxy.h
	KX_PolygonMaterial.h
	KX_PositionInterpolator.h
	KX_PyConstraintBinding.h
	KX_PyMath.h
	KX_PythonInit.h
	KX_PythonInitTypes.h
	KX_PythonSeq.h
	KX_RadarSensor.h
	KX_RayCast.h
	KX_RayEventManager.h
	KX_RaySensor.h
	KX_SCA_AddObjectActuator.h
	KX_SCA_DynamicActuator.h
	KX_SCA_EndObjectActuator.h
	KX_SCA_ReplaceMeshActuator.h
	KX_SG_BoneParentNodeRelationship.h
	KX_SG_NodeRelationships.h
	KX_ScalarInterpolator.h
	KX_ScalingInterpolator.h
	KX_Scene.h
	KX_SceneActuator.h
	KX_SoundActuator.h
	KX_StateActuator.h
	KX_TimeCategoryLogger.h
	KX_TimeLogger.h
	KX_TouchEventManager.h
	KX_TouchSensor.h
	KX_TrackToActuator.h
	KX_VehicleWrapper.h
	KX_VertexProxy.h
	KX_VisibilityActuator.h
	KX_WorldInfo.h
	KX_WorldIpoController.h
)

add_definitions(-DGLEW_STATIC)

if(WITH_SDL)
	list(APPEND INC_SYS
		${SDL_INCLUDE_DIR}
	)
else()
	add_definitions(-DDISABLE_SDL)
endif()

if(WITH_CODEC_FFMPEG)
	add_definitions(-DWITH_FFMPEG)
endif()

if(WITH_AUDASPACE)
	list(APPEND INC
		../../../intern/audaspace/intern
	)
	add_definitions(-DWITH_AUDASPACE)
endif()

if(WITH_BULLET)
	list(APPEND INC
		../../../extern/bullet2/src
		../Physics/Bullet
	)
	add_definitions(-DUSE_BULLET)
endif()

blender_add_lib(ge_logic_ketsji "${SRC}" "${INC}" "${INC_SYS}")
