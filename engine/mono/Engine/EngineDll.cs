﻿
// AUTOGENERATED FILE: Do not edit directly, instead edit EngineDll.tt

using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

using Engine;

using size_t = System.UInt64;

namespace EngineDll {

	public class Dll {


		public static class CameraComponent {

            [DllImport(Paths.Exe, EntryPoint = "CameraComponent_Create")]
		    public static extern CppObjectInfo Create(/*CppRef cppObjRef, */CsRef csCompRef);

            [DllImport(Paths.Exe, EntryPoint = "CameraComponent_IsAttached_get")]
		    public static extern bool IsAttached_get(CppRef objRef);

            [DllImport(Paths.Exe, EntryPoint = "CameraComponent_orthographic_get")]
		    public static extern bool orthographic_get(CppRef objRef);

            [DllImport(Paths.Exe, EntryPoint = "CameraComponent_orthographic_set")]
		    public static extern void orthographic_set(CppRef objRef, bool value);

            [DllImport(Paths.Exe, EntryPoint = "CameraComponent_Attach")]
		    public static extern void Attach(CppRef compRef);

            [DllImport(Paths.Exe, EntryPoint = "CameraComponent_UpdateProjMatrix")]
		    public static extern void UpdateProjMatrix(CppRef compRef);

		}

		public static class Actor {
		
            [DllImport(Paths.Exe, EntryPoint = "Actor_BindComponent")]
		    public static extern void BindComponent(CppRef objRef, CppRef compRef);

            [DllImport(Paths.Exe, EntryPoint = "Actor_InitComponent")]
		    public static extern void InitComponent(CppRef objRef, CppRef compRef);

            [DllImport(Paths.Exe, EntryPoint = "Actor_gameObject_get")]
		    public static extern CsRef gameObject_get(CppRef objBaseRef);

            [DllImport(Paths.Exe, EntryPoint = "Actor_parent_get")]
		    public static extern CsRef parent_get(CppRef objRef);

            [DllImport(Paths.Exe, EntryPoint = "Actor_parent_set")]
		    public static extern void parent_set(CppRef objRef, CppRef newObjRef);

            [DllImport(Paths.Exe, EntryPoint = "Actor_DestroyComponent")]
		    public static extern void DestroyComponent(CppRef compRef);

            [DllImport(Paths.Exe, EntryPoint = "Actor_Destroy")]
		    public static extern void Destroy(CppRef objRef);

            [DllImport(Paths.Exe, EntryPoint = "Actor_GetComponentsCount")]
		    public static extern int GetComponentsCount(CppRef objRef);

            [DllImport(Paths.Exe, EntryPoint = "Actor_WriteComponentsRefs")]
		    public static extern void WriteComponentsRefs(CppRef objRef, size_t listPtr);

            [DllImport(Paths.Exe, EntryPoint = "Actor_GetChildrenCount")]
		    public static extern int GetChildrenCount(CppRef objRef);

            [DllImport(Paths.Exe, EntryPoint = "Actor_GetChild")]
		    public static extern CsRef GetChild(CppRef objRef, int index);

            [DllImport(Paths.Exe, EntryPoint = "Actor_localPosition_get")]
		    public static extern Vector3 localPosition_get(CppRef objRef);

            [DllImport(Paths.Exe, EntryPoint = "Actor_localPosition_set")]
		    public static extern void localPosition_set(CppRef objRef, Vector3 value);

            [DllImport(Paths.Exe, EntryPoint = "Actor_localRotation_get")]
		    public static extern Vector3 localRotation_get(CppRef objRef);

            [DllImport(Paths.Exe, EntryPoint = "Actor_localRotation_set")]
		    public static extern void localRotation_set(CppRef objRef, Vector3 value);

            [DllImport(Paths.Exe, EntryPoint = "Actor_localRotationQ_get")]
		    public static extern Quaternion localRotationQ_get(CppRef objRef);

            [DllImport(Paths.Exe, EntryPoint = "Actor_localRotationQ_set")]
		    public static extern void localRotationQ_set(CppRef objRef, Quaternion value);

            [DllImport(Paths.Exe, EntryPoint = "Actor_localScale_get")]
		    public static extern Vector3 localScale_get(CppRef objRef);

            [DllImport(Paths.Exe, EntryPoint = "Actor_localScale_set")]
		    public static extern void localScale_set(CppRef objRef, Vector3 value);

            [DllImport(Paths.Exe, EntryPoint = "Actor_worldPosition_get")]
		    public static extern Vector3 worldPosition_get(CppRef objRef);

            [DllImport(Paths.Exe, EntryPoint = "Actor_worldPosition_set")]
		    public static extern void worldPosition_set(CppRef objRef, Vector3 value);

            [DllImport(Paths.Exe, EntryPoint = "Actor_worldRotationQ_get")]
		    public static extern Quaternion worldRotationQ_get(CppRef objRef);

            [DllImport(Paths.Exe, EntryPoint = "Actor_worldRotationQ_set")]
		    public static extern void worldRotationQ_set(CppRef objRef, Quaternion value);

            [DllImport(Paths.Exe, EntryPoint = "Actor_worldScale_get")]
		    public static extern Vector3 worldScale_get(CppRef objRef);

            [DllImport(Paths.Exe, EntryPoint = "Actor_worldScale_set")]
		    public static extern void worldScale_set(CppRef objRef, Vector3 value);

            [DllImport(Paths.Exe, EntryPoint = "Actor_localForward_get")]
		    public static extern Vector3 localForward_get(CppRef objRef);

            [DllImport(Paths.Exe, EntryPoint = "Actor_localUp_get")]
		    public static extern Vector3 localUp_get(CppRef objRef);

            [DllImport(Paths.Exe, EntryPoint = "Actor_localRight_get")]
		    public static extern Vector3 localRight_get(CppRef objRef);

            [DllImport(Paths.Exe, EntryPoint = "Actor_forward_get")]
		    public static extern Vector3 forward_get(CppRef objRef);

            [DllImport(Paths.Exe, EntryPoint = "Actor_up_get")]
		    public static extern Vector3 up_get(CppRef objRef);

            [DllImport(Paths.Exe, EntryPoint = "Actor_right_get")]
		    public static extern Vector3 right_get(CppRef objRef);


		}

		public static class Game {
		
            [DllImport(Paths.Exe, EntryPoint = "Game_CreateGameObjectFromCS", CharSet = CharSet.Ansi)]
		    public static extern GameObjectInfo CreateGameObjectFromCS(CppRef gameRef, CsRef csRef, CppRef parentRef);

            [DllImport(Paths.Exe, EntryPoint = "Game_GetRootActorsCount")]
		    public static extern int GetRootActorsCount(CppRef gameRef);

            [DllImport(Paths.Exe, EntryPoint = "Game_WriteRootActorsRefs")]
		    public static extern void WriteRootActorsRefs(CppRef gameRef, CsRef[] refs);


		}

		public static class CsComponent {

            [DllImport(Paths.Exe, EntryPoint = "CsComponent_Create")]
		    public static extern CppObjectInfo Create(CsRef csCompRef);


		}

		public static class Input {
		
            [DllImport(Paths.Exe, EntryPoint = "HotKeys_GetButtonDown")]
		    public static extern bool GetButtonDown(CppRef objRef, int keyCode);

            [DllImport(Paths.Exe, EntryPoint = "HotKeys_GetButtonUp")]
		    public static extern bool GetButtonUp(CppRef objRef, int keyCode);

            [DllImport(Paths.Exe, EntryPoint = "HotKeys_GetButton")]
		    public static extern bool GetButton(CppRef objRef, int keyCode);

            [DllImport(Paths.Exe, EntryPoint = "HotKeys_RegisterHotkey")]
		    public static extern void RegisterHotkey(CppRef objRef, int keyCode);

            [DllImport(Paths.Exe, EntryPoint = "HotKeys_UnregisterHotkey")]
		    public static extern void UnregisterHotkey(CppRef objRef, int keyCode);

            [DllImport(Paths.Exe, EntryPoint = "HotKeys_MousePosition")]
		    public static extern Vector2 MousePosition(CppRef objRef);

            [DllImport(Paths.Exe, EntryPoint = "HotKeys_wheelDelta_get")]
		    public static extern int wheelDelta_get(CppRef objRef);

		}

		public static class Material {

            [DllImport(Paths.Exe, EntryPoint = "Material_diffuseColor_get")]
		    public static extern Vector3 diffuseColor_get(CppRef objRef);

            [DllImport(Paths.Exe, EntryPoint = "Material_diffuseColor_set")]
		    public static extern void diffuseColor_set(CppRef objRef, Vector3 value);

            [DllImport(Paths.Exe, EntryPoint = "Material_diffuse_get")]
		    public static extern float diffuse_get(CppRef objRef);

            [DllImport(Paths.Exe, EntryPoint = "Material_diffuse_set")]
		    public static extern void diffuse_set(CppRef objRef, float value);

            [DllImport(Paths.Exe, EntryPoint = "Material_ambient_get")]
		    public static extern float ambient_get(CppRef objRef);

            [DllImport(Paths.Exe, EntryPoint = "Material_ambient_set")]
		    public static extern void ambient_set(CppRef objRef, float value);

            [DllImport(Paths.Exe, EntryPoint = "Material_specular_get")]
		    public static extern float specular_get(CppRef objRef);

            [DllImport(Paths.Exe, EntryPoint = "Material_specular_set")]
		    public static extern void specular_set(CppRef objRef, float value);

            [DllImport(Paths.Exe, EntryPoint = "Material_shininess_get")]
		    public static extern float shininess_get(CppRef objRef);

            [DllImport(Paths.Exe, EntryPoint = "Material_shininess_set")]
		    public static extern void shininess_set(CppRef objRef, float value);

            [DllImport(Paths.Exe, EntryPoint = "Material_cullMode_get")]
		    public static extern int cullMode_get(CppRef objRef);

            [DllImport(Paths.Exe, EntryPoint = "Material_cullMode_set")]
		    public static extern void cullMode_set(CppRef objRef, int value);

            [DllImport(Paths.Exe, EntryPoint = "Material_fillMode_get")]
		    public static extern int fillMode_get(CppRef objRef);

            [DllImport(Paths.Exe, EntryPoint = "Material_fillMode_set")]
		    public static extern void fillMode_set(CppRef objRef, int value);

            [DllImport(Paths.Exe, EntryPoint = "Material_priority_get")]
		    public static extern size_t priority_get(CppRef objRef);

            [DllImport(Paths.Exe, EntryPoint = "Material_priority_set")]
		    public static extern void priority_set(CppRef objRef, size_t value);

            [DllImport(Paths.Exe, EntryPoint = "Material_name_get")]
		    public static extern int name_get(CppRef matRef, byte[] buf, int bufLehgth);

            [DllImport(Paths.Exe, EntryPoint = "Material_isDynamic_get")]
		    public static extern bool isDynamic_get(CppRef matRef);

            [DllImport(Paths.Exe, EntryPoint = "Material_Create")]
		    public static extern CppRef Create(CppRef gameRef, size_t assetHash);

		}

		public static class Assets {
		
            [DllImport(Paths.Exe, EntryPoint = "Assets_Reload")]
		    public static extern void Reload(CppRef gameRef, int assetId);

            [DllImport(Paths.Exe, EntryPoint = "Assets_Get")]
		    public static extern CppRef Get(CppRef gameRef, int assetId);

		}

		public static class ImageAsset {
		
            [DllImport(Paths.Exe, EntryPoint = "ImageAsset_Load", CharSet = CharSet.Ansi)]
		    public static extern CppRef Load(CppRef gameRef, string path, ref int width, ref int height);

		}

		public static class Texture {
		
            [DllImport(Paths.Exe, EntryPoint = "Texture_assetId_get")]
		    public static extern int assetId_get(CppRef objRef);

            [DllImport(Paths.Exe, EntryPoint = "Texture_assetId_set")]
		    public static extern void assetId_set(CppRef objRef, int value);

            [DllImport(Paths.Exe, EntryPoint = "Texture_PushAsset")]
		    public static extern CppRef PushAsset(CppRef gameRef, int assetId);

            [DllImport(Paths.Exe, EntryPoint = "Texture_Init")]
		    public static extern void Init(CppRef gameRef, CppRef texRef, uint width, uint height);

            [DllImport(Paths.Exe, EntryPoint = "Texture_InitFromImage")]
		    public static extern void InitFromImage(CppRef gameRef, CppRef texRef, CppRef imageRef);

		}

		public static class MeshAsset {

            [DllImport(Paths.Exe, EntryPoint = "MeshAsset_CreateHash", CharSet = CharSet.Ansi)]
		    public static extern size_t CreateHash(CppRef meshAssetRef, string fileName);

            [DllImport(Paths.Exe, EntryPoint = "MeshAsset_Load")]
		    public static extern void Load(CppRef meshAssetRef, size_t assetHash);

            [DllImport(Paths.Exe, EntryPoint = "MeshAsset_GetMesh")]
		    public static extern CppRef GetMesh(CppRef meshAssetRef, size_t assetHash);

            [DllImport(Paths.Exe, EntryPoint = "MeshAsset_CreateDynamicMaterial")]
		    public static extern CppRef CreateDynamicMaterial(CppRef meshAssetRef, CppRef otherMaterialRef);

            [DllImport(Paths.Exe, EntryPoint = "MeshAsset_DeleteDynamicMaterial")]
		    public static extern void DeleteDynamicMaterial(CppRef meshAssetRef, CppRef otherMaterialRef);

		}

		public static class Mesh4 {
		
            [DllImport(Paths.Exe, EntryPoint = "Mesh4_ShapeCount")]
		    public static extern int ShapeCount(CppRef mesh4Ref);

            [DllImport(Paths.Exe, EntryPoint = "Mesh4_MaterialMaxIndex")]
		    public static extern int MaterialMaxIndex(CppRef mesh4Ref);

            [DllImport(Paths.Exe, EntryPoint = "Mesh4_assetHash_get")]
		    public static extern size_t assetHash_get(CppRef objRef);

		}

		public static class MeshComponent {

            [DllImport(Paths.Exe, EntryPoint = "MeshComponent_Create")]
		    public static extern CppObjectInfo Create(/*CppRef cppObjRef, */CsRef csCompRef);

            [DllImport(Paths.Exe, EntryPoint = "MeshComponent_IsDynamic_get")]
		    public static extern bool IsDynamic_get(CppRef objRef);

            [DllImport(Paths.Exe, EntryPoint = "MeshComponent_IsStatic_get")]
		    public static extern bool IsStatic_get(CppRef objRef);

            [DllImport(Paths.Exe, EntryPoint = "MeshComponent_MaterialCount_get")]
		    public static extern int MaterialCount_get(CppRef objRef);

            [DllImport(Paths.Exe, EntryPoint = "MeshComponent_SetFromCs")]
		    public static extern void SetFromCs(CppRef compRef, CppRef meshRef);

            [DllImport(Paths.Exe, EntryPoint = "MeshComponent_AddShape", CallingConvention = CallingConvention.Cdecl)]
		    public static extern void AddShape(CppRef compRef, Vertex[] verteces, int vength, int[] indeces, int ilength, int matIndex);

            [DllImport(Paths.Exe, EntryPoint = "MeshComponent_RemoveMaterials")]
		    public static extern void RemoveMaterials(CppRef compRef);

            [DllImport(Paths.Exe, EntryPoint = "MeshComponent_RemoveMaterial")]
		    public static extern void RemoveMaterial(CppRef compRef, int index);

            [DllImport(Paths.Exe, EntryPoint = "MeshComponent_ClearMesh")]
		    public static extern void ClearMesh(CppRef compRef);

            [DllImport(Paths.Exe, EntryPoint = "MeshComponent_SetMaterial")]
		    public static extern void SetMaterial(CppRef compRef, size_t index, CppRef materialRef);

            [DllImport(Paths.Exe, EntryPoint = "MeshComponent_GetMaterial")]
		    public static extern CppRef GetMaterial(CppRef compRef, size_t index);

		}
	}
}