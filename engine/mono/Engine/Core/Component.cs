using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

using EngineDll;

namespace Engine {

    [StructLayout(LayoutKind.Sequential)]
    public struct ComponentCallbacks {
        public delegate void ComponentCallback();
        public delegate void ContactEnter(Actor otherActor, in Contact contact);
        public delegate void ContactExit(Actor otherActor);

        public ComponentCallback onInit;
        public ComponentCallback onStart;
        public ComponentCallback onUpdate;
        public ComponentCallback onFixedUpdate;
        public ComponentCallback onDestroy;

        public ContactEnter onCollisionEnter;
        public ContactExit onCollisionExit;
        public ContactEnter onTriggerEnter;
        public ContactExit onTriggerExit;

        public ComponentCallback onActivate;
        public ComponentCallback onDeactivate;

    }

    [GUID("ce174a6a-575b-4e00-b603-088fa2a337f9", typeof(Component))]
    public abstract class Component : CppLinked {

        public bool runtimeOnly {
            get => Dll.CsComponent.runtimeOnly_get(cppRef);
            set => Dll.CsComponent.runtimeOnly_set(cppRef, value);
        }

        public bool IsCrashed {
            get => Dll.CsComponent.f_isCrashed_get(cppRef);
            private set => Dll.CsComponent.f_isCrashed_set(cppRef, value);
        }

        [Close] public bool IsActivated {
            get => Dll.CsComponent.IsActivated_get(cppRef);
        }

        private CsRef m_objectRef;

        private ComponentCallbacks m_callbacks;


        public bool IsDestroyed => GetObjectByRef(csRef) == null;

        public Actor actor => GetObjectByRef(m_objectRef) as Actor;

        //public Transform transform => GetObjectByRef(m_transformRef) as Transform;

        public abstract CppObjectInfo CppConstructor();

        public virtual void OnInit() { }
        public virtual void OnStart() { }
        public virtual void OnUpdate() { }
        public virtual void OnFixedUpdate() { }
        public virtual void OnDestroy() { }

        public virtual void OnCollisionEnter(Actor otherActor, in Contact contact) { }
        public virtual void OnCollisionExit(Actor otherActor) { }
        public virtual void OnTriggerEnter(Actor otherActor, in Contact contact) { }
        public virtual void OnTriggerExit(Actor otherActor) { }

        public virtual void OnActivate() { }
        public virtual void OnDeactivate() { }


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
            m_callbacks.onFixedUpdate = new ComponentCallbacks.ComponentCallback(OnFixedUpdate);
            m_callbacks.onDestroy = new ComponentCallbacks.ComponentCallback(OnDestroy);

            m_callbacks.onCollisionEnter = new ComponentCallbacks.ContactEnter(OnCollisionEnter);
            m_callbacks.onCollisionExit = new ComponentCallbacks.ContactExit(OnCollisionExit);
            m_callbacks.onTriggerEnter = new ComponentCallbacks.ContactEnter(OnTriggerEnter);
            m_callbacks.onTriggerExit = new ComponentCallbacks.ContactExit(OnTriggerExit);

            m_callbacks.onActivate = new ComponentCallbacks.ComponentCallback(OnActivate);
            m_callbacks.onDeactivate = new ComponentCallbacks.ComponentCallback(OnDeactivate);

            dll_SetComponentCallbacks(cppRef, m_callbacks);

            return res;
        }

        public static bool RunOrCrush(CsRef componentRef, ComponentCallbacks.ComponentCallback method) {
            try {
                method.Invoke();
                return false;
            }
            catch(Exception ex) {
                CrashComponent(componentRef, ex);
                return true;
            }
        }

        public static bool RunOrCrushContactEnter(CsRef componentRef, ComponentCallbacks.ContactEnter method, CsRef otherActorRef, in Contact contact) {
            try {
                var otherActor = CppLinked.GetObjectByRef(otherActorRef) as Actor;
                method.Invoke(otherActor, in contact);
                return false;
            } catch (Exception ex) {
                CrashComponent(componentRef, ex);
                return true;
            }
        }

        public static bool RunOrCrushContactExit(CsRef componentRef, ComponentCallbacks.ContactExit method, CsRef otherActorRef) {
            try {
                var otherActor = CppLinked.GetObjectByRef(otherActorRef) as Actor;
                method.Invoke(otherActor);
                return false;
            } catch (Exception ex) {
                CrashComponent(componentRef, ex);
                return true;
            }
        }

        private static void CrashComponent(CsRef componentRef, Exception ex) {
            var component = CppLinked.GetObjectByRef(componentRef) as Component;

            Console.WriteLine("ComponentCrash: Component was disabled.");
            if (component == null) {
                Console.WriteLine($"Component: null {componentRef}");
            } else {
                Console.WriteLine($"Component: {component.GetType().FullName}, ");
                Console.WriteLine($"Actor name: ({component.actor.Name})");
            }
            Console.WriteLine("Message: ");
            Console.WriteLine(ex.Message);
            Console.WriteLine("StackTrace: ");
            Console.WriteLine(ex.StackTrace);
        }

        // private void cpp_OnCollisionEnter(CsRef otherActorRef) {
        //     var otherActor = CppLinked.GetObjectByRef(otherActorRef) as Actor;

        //     OnCollisionEnter(otherActor);
        // }

        // private void cpp_OnCollisionExit(CsRef otherActorRef) {
        //     var otherActor = CppLinked.GetObjectByRef(otherActorRef) as Actor;

        //     OnCollisionExit(otherActor);
        // }

        // private void cpp_OnTriggerEnter(CsRef otherActorRef) {
        //     var otherActor = CppLinked.GetObjectByRef(otherActorRef) as Actor;

        //     OnTriggerEnter(otherActor);
        // }

        // private void cpp_OnTriggerExit(CsRef otherActorRef) {
        //     var otherActor = CppLinked.GetObjectByRef(otherActorRef) as Actor;

        //     OnTriggerExit(otherActor);
        // }

        [DllImport(Paths.Exe, EntryPoint = "Actor_SetComponentCallbacks")]
        private static extern void dll_SetComponentCallbacks(CppRef componentRef, ComponentCallbacks callbacks);
    }

    /// <summary>
    /// Базовый класс для C# компонента.
    /// </summary>
    [GUID("0ee2de42-9126-42d6-90cf-e4b65ac7c607", typeof(CSComponent))]
    public abstract class CSComponent : Component, FireYaml.IFile {
        /// FireYaml.IFile ->
        public ulong assetInstance { get; set; } = FireYaml.AssetInstance.PopId();

        public int fileId { get; set; } = -1;

        public string prefabId { get; set; } = FireYaml.IFile.NotPrefab;
        /// <- 

        public override CppObjectInfo CppConstructor() {
            return Dll.CsComponent.Create(csRef);
        }

    }

    /// <summary>
    /// Базовый класс для C# тени C++ компонента.
    /// </summary>
    [Serializable]
    public abstract class CppComponent : Component, FireYaml.IFile {
        /// FireYaml.IFile ->
        public ulong assetInstance { get; set; } = FireYaml.AssetInstance.PopId();

        public int fileId { get; set; } = -1;

        public string prefabId { get; set; } = FireYaml.IFile.NotPrefab;
        /// <- 
    }

}
