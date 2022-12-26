﻿using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

using EngineDll;

namespace Engine {

    [StructLayout(LayoutKind.Sequential)]
    public struct ComponentCallbacks {
        public delegate void ComponentCallback();

        public ComponentCallback onInit;
        public ComponentCallback onStart;
        public ComponentCallback onUpdate;
        public ComponentCallback onDestroy;

    }

    abstract class Component : CppLinked {
        private CsRef m_objectRef;

        private ComponentCallbacks m_callbacks;


        public bool IsDestroyed => GetObjectByRef(csRef) == null;

        public Actor actor => GetObjectByRef(m_objectRef) as Actor;

        //public Transform transform => GetObjectByRef(m_transformRef) as Transform;

        public abstract CppObjectInfo CppConstructor();

        public virtual void OnInit() { }
        public virtual void OnStart() { }
        public virtual void OnUpdate() { }
        public virtual void OnDestroy() { }


        public void Destroy() => Dll.Actor.Destroy(cppRef);

        public void CsBindComponent(CsRef objectRef, CppObjectInfo componentInfo) {
            m_objectRef = objectRef;
            Link(componentInfo.classRef, componentInfo.objectRef);
        }

        public override CsRef Link(CppRef classInfoRef, CppRef objRef) {
            var res = base.Link(classInfoRef, objRef);

            m_callbacks = new ComponentCallbacks();
            m_callbacks.onInit = new ComponentCallbacks.ComponentCallback(OnInit);
            m_callbacks.onStart = new ComponentCallbacks.ComponentCallback(OnStart);
            m_callbacks.onUpdate = new ComponentCallbacks.ComponentCallback(OnUpdate);
            m_callbacks.onDestroy = new ComponentCallbacks.ComponentCallback(OnDestroy);

            dll_SetComponentCallbacks(cppRef, m_callbacks);

            return res;
        }

        [DllImport(Paths.Exe, EntryPoint = "Actor_SetComponentCallbacks")]
        private static extern void dll_SetComponentCallbacks(CppRef componentRef, ComponentCallbacks callbacks);
    }

    /// <summary>
    /// Базовый класс для C# компонента.
    /// </summary>
    [Serializable]
    abstract class CSComponent : Component, FireYaml.IFile {
        /// FireYaml.IFile ->
        public ulong assetInstance { get; set; } = FireYaml.AssetInstance.PopId();

        public int fileId { get; set; } = -1;

        public string prefabId { get; set; } = FireYaml.IFile.NotPrefab;
        /// <- 

        public override CppObjectInfo CppConstructor(/*Actor target*/) {
            return Dll.CsComponent.Create(/*target.cppRef, */csRef);
        }

    }

    /// <summary>
    /// Базовый класс для C# тени C++ компонента.
    /// </summary>
    [Serializable]
    abstract class CppComponent : Component, FireYaml.IFile {
        /// FireYaml.IFile ->
        public ulong assetInstance { get; set; } = FireYaml.AssetInstance.PopId();

        public int fileId { get; set; } = -1;

        public string prefabId { get; set; } = FireYaml.IFile.NotPrefab;
        /// <- 
    }

}
