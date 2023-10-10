using System;
using System.Reflection;
using System.Collections.Generic;
using System.Text;
using EngineMono;
using System.Runtime.InteropServices;

using EngineDll;

namespace Engine {

    [GUID("b0730a53-16c4-4943-bd7d-666f402617d5", typeof(Actor))]

#if DETACHED
    public sealed class Actor : CppLinked, FireYaml.IFile {

        /// FireYaml.IFile ->
        [Close] public ulong assetInstance { get; set; } = FireYaml.AssetInstance.PopId();

        [Close] public int fileId { get; set; } = -1;

        [Close] public Flag Flags { get; set; }
        public string prefabId { get; set; }
        public string Name { get; set; }
        public bool IsActive { get; private set; }
        public bool ActiveSelf { get; set; }
        public Vector3 localPosition { get; set; }
        [Close] public Vector3 localRotation { get; set; }
        public Quaternion localRotationQ { get; set; }
        public Vector3 localScale { get; set; }
        [Close] public Vector3 worldPosition { get; set; }
        [Close] public Quaternion worldRotationQ { get; set; }
        [Close] public Vector3 worldScale { get; set; }

        public Vector3 localForward { get; private set; }
        public Vector3 localUp { get; private set; }
        public Vector3 localRight { get; private set; }

        public Vector3 forward { get; private set; }
        public Vector3 up { get; private set; }
        public Vector3 right { get; private set; }

        public bool HasParent => parent != null;

        public Scene scene { get; set; }

        [Close]
        public Actor parent { get; set; }

        public bool IsDestroyed => GetObjectByRef(csRef) == null;

        /// <summary> Detached ready </summary>
        public int GetChildrenCount() => detached_children.Count;


        [Close] public List<Actor> detached_children = new List<Actor>();
        [Close] public List<Component> detached_components = new List<Component>();


        #region Public

        /// <summary> Detached ready </summary>
        public Actor() : this("Actor", null) { }

        /// <summary> Detached ready </summary>
        public Actor(string name) : this(name, null) { }

        /// <summary> Detached ready </summary>
        public Actor(Actor targetParent) : this("Actor", targetParent) { }

        /// <summary> Detached ready </summary>
        public Actor(string name, Actor targetParent) {
            if (targetParent != null)
                targetParent.detached_children.Add(this);

            parent = targetParent;
            Name = name;

            scene = Game.GetScene();
        }

        /// <summary> Detached ready </summary>
        public void Destroy() {
            if (parent != null) {
                parent.detached_children.Remove(this);
                parent = null;
            }
            RemoveCsRef(csRef);
            csRef = 0;
        }

        ~Actor() { }

        /// <summary> Detached ready </summary>
        public TComponent AddComponent<TComponent>() where TComponent : Component, new() {
            /// Create
            var component = new TComponent();
            //var info = component.CppConstructor();
            /// Bind
            //component.CsBindComponent(csRef, info);
            //Dll.Actor.BindComponent(cppRef, component.cppRef);
            /// PostBind
            /// Init
            //Dll.Actor.InitComponent(cppRef, component.cppRef);

            detached_components.Add(component);

            return component;
        }

        /// <summary> Detached ready </summary>
        public TComponent GetComponent<TComponent>() where TComponent : Component {
            foreach (var component in detached_components) {
                if (component is TComponent)
                    return component as TComponent;
            }
            return null;
        }

        /// <summary> Detached ready </summary>
        public List<TComponent> GetComponents<TComponent>() where TComponent : Component {
            var list = new List<TComponent>();
            m_GetComponents(ref list);
            return list;
        }

        /// <summary> Detached ready </summary>
        public TComponent GetComponentInChild<TComponent>() where TComponent : Component {
            var component = GetComponent<TComponent>();
            if (component != null)
                return component;

            int count = GetChildrenCount();
            for (int i = 0; i < count; i++) {
                var child = GetChild(i);
                if (child != null) {
                    component = child.GetComponentInChild<TComponent>();
                    if (component != null)
                        return component;
                }
            }
            return null;
        }

        /// <summary> Detached ready </summary>
        public List<TComponent> GetComponentsInChild<TComponent>() where TComponent : Component {
            var list = new List<TComponent>();
            m_GetComponentsInChild(ref list);
            return list;
        }

        /// <summary> Detached ready </summary>
        public Actor GetChild(int index) {
            if (index < 0 || index >= GetChildrenCount())
                return null;

            return detached_children[index];
        }

        /// <summary> Detached ready </summary>
        public List<Actor> GetChildren() {
            var list = new List<Actor>();
            list.AddRange(detached_children);

            return list;
        }

        /// <summary> Detached ready </summary>
        public List<Component> GetComponentsList() {
            var list = new List<Component>();
            list.AddRange(detached_components);

            return list;
        }

        /// <summary> Detached ready </summary>
        public bool Has(Flag flags) {
            return (Flags & flags) == flags;
        }

        #endregion
        #region Private

        /// <summary> Detached ready </summary>
        private void m_GetComponents<TComponent>(ref List<TComponent> list) where TComponent : Component {
            foreach (var component in detached_components) {
                if (component is TComponent)
                    list.Add(component as TComponent);
            }
        }

        /// <summary> Detached ready </summary>
        private void m_GetComponentsInChild<TComponent>(ref List<TComponent> list) where TComponent : Component {
            m_GetComponents(ref list);

            int count = GetChildrenCount();
            for (int i = 0; i < count; i++) {
                var child = GetChild(i);
                if (child != null)
                    m_GetComponentsInChild(ref list);
            }
        }

        /// <summary> Detached ready </summary>
        public static void SetPrefabId(CsRef actorRef, int prefabGuidHash) {
            var actor = CppLinked.GetObjectByRef(actorRef) as Actor;

            var prefabGuid = "";
            if (prefabGuidHash != 0)
                prefabGuid = FireYaml.AssetStore.Instance.GetAssetGuid(prefabGuidHash);

            actor.prefabId = prefabGuid;
        }

        #endregion
        #region Cpp
        #endregion

    }

#else

    public sealed class Actor : CppLinked, FireYaml.IFile {

        /// FireYaml.IFile ->
        [Close] public ulong assetInstance { get; set; } = FireYaml.AssetInstance.PopId();

        [Close] public int fileId { get; set; } = -1;

        [Close]
        public Flag Flags {
            get => (Flag)Dll.Actor.flags_get(cppRef);
            set => Dll.Actor.flags_set(cppRef, (ulong)value);
        }

        public string prefabId {
            get => Dll.Actor.prefabId_get(cppRef);
            set => Dll.Actor.prefabId_set(cppRef, value);
        }

        public string Name { get => Dll.Actor.name_get(cppRef); set => Dll.Actor.name_set(cppRef, value); }

        public bool IsActive {
            get => Dll.Actor.isActive_get(cppRef);
        }

        public bool ActiveSelf {
            get => Dll.Actor.activeSelf_get(cppRef);
            set => Dll.Actor.activeSelf_set(cppRef, value);
        }

        public Vector3 localPosition {
            get => Dll.Actor.localPosition_get(cppRef);
            set => Dll.Actor.localPosition_set(cppRef, value);
        }

        [Close]
        public Vector3 localRotation {
            get => Dll.Actor.localRotation_get(cppRef).deg();
            set => Dll.Actor.localRotation_set(cppRef, value.rad());
        }

        public Quaternion localRotationQ {
            get => Dll.Actor.localRotationQ_get(cppRef);
            set => Dll.Actor.localRotationQ_set(cppRef, value);
        }

        public Vector3 localScale {
            get => Dll.Actor.localScale_get(cppRef);
            set => Dll.Actor.localScale_set(cppRef, value);
        }

        [Close]
        public Vector3 worldPosition {
            get => Dll.Actor.worldPosition_get(cppRef);
            set => Dll.Actor.worldPosition_set(cppRef, value);
        }

        [Close]
        public Quaternion worldRotationQ {
            get => Dll.Actor.worldRotationQ_get(cppRef);
            set => Dll.Actor.worldRotationQ_set(cppRef, value);
        }

        [Close]
        public Vector3 worldScale {
            get => Dll.Actor.worldScale_get(cppRef);
            set => Dll.Actor.worldScale_set(cppRef, value);
        }

        public Vector3 localForward => Dll.Actor.localForward_get(cppRef);
        public Vector3 localUp => Dll.Actor.localUp_get(cppRef);
        public Vector3 localRight => Dll.Actor.localRight_get(cppRef);

        public Vector3 forward => Dll.Actor.forward_get(cppRef);
        public Vector3 up => Dll.Actor.up_get(cppRef);
        public Vector3 right => Dll.Actor.right_get(cppRef);

        public bool HasParent => parent != null;

        public Scene scene => new Scene(Dll.Actor.scene_get(cppRef));

        [Close]
        public Actor parent {
            get => (Actor)GetObjectByRef(Dll.Actor.parent_get(cppRef));
            set => Dll.Actor.parent_set(cppRef, value == null ? 0 : value.cppRef);
        }

        public bool IsDestroyed => GetObjectByRef(csRef) == null;

        public int GetChildrenCount() => Dll.Actor.GetChildrenCount(cppRef);


    #region Public

        public Actor() : this("Actor", null) { }
        public Actor(string name) : this(name, null) { }
        public Actor(Actor targetParent) : this("Actor", targetParent) { }

        public Actor(string name, Actor targetParent) {
            CppRef parentRef = targetParent != null ? targetParent.cppRef : 0;

            var info = Dll.Game.CreateGameObjectFromCS(Game.sceneRef, csRef, parentRef);
            Link(info.classRef, info.objectRef);

            Name = name;
        }


        public void Destroy() {
            Dll.Actor.Destroy(cppRef);
        }

        ~Actor() {
            //Console.WriteLine($"#: ~GameObject({csRef}, {cppRef})(): {cppRef}, {csRef}");
        }

        public TComponent AddComponent<TComponent>() where TComponent : Component, new() {
            /// Create
            var component = new TComponent();
            var info = component.CppConstructor();
            /// Bind
            component.CsBindComponent(csRef, info);
            Dll.Actor.BindComponent(cppRef, component.cppRef);
            /// PostBind
            /// Init
            Dll.Actor.InitComponent(cppRef, component.cppRef);

            return component;
        }

        private CsRef m_AddComponentFromCpp<TComponent>(CppObjectInfo info) where TComponent : Component, new() {
            /// Create
            var component = new TComponent();
            /// Bind
            component.CsBindComponent(csRef, info);
            /// PostBind
            /// Init
            return component.csRef;
        }

        public TComponent GetComponent<TComponent>() where TComponent : Component {
            var refs = m_GetComponentRefs();

            foreach (var compRef in refs) {
                if (compRef.value != 0) {
                    var component = GetObjectByRef(compRef);
                    if (component is TComponent)
                        return component as TComponent;
                }
            }
            return null;
        }

        public List<TComponent> GetComponents<TComponent>() where TComponent : Component {
            var list = new List<TComponent>();
            m_GetComponents(ref list);
            return list;
        }

        public TComponent GetComponentInChild<TComponent>() where TComponent : Component {
            var component = GetComponent<TComponent>();
            if (component != null)
                return component;

            int count = GetChildrenCount();
            for (int i = 0; i < count; i++) {
                var child = GetChild(i);
                if (child != null) {
                    component = child.GetComponentInChild<TComponent>();
                    if (component != null)
                        return component;
                }
            }
            return null;
        }

        public List<TComponent> GetComponentsInChild<TComponent>() where TComponent : Component {
            var list = new List<TComponent>();
            m_GetComponentsInChild(ref list);
            return list;
        }

        public Actor GetChild(int index) {
            if (index < 0 || index >= GetChildrenCount())
                return null;

            var childRef = Dll.Actor.GetChild(cppRef, index);
            return GetObjectByRef(childRef) as Actor;
        }

        public List<Actor> GetChildren() {
            var count = GetChildrenCount();
            var list = new List<Actor>();

            for (int i = 0; i < count; i++) {
                var childRef = Dll.Actor.GetChild(cppRef, i);
                list.Add(GetObjectByRef(childRef) as Actor);
            }
            return list;
        }

        public List<Component> GetComponentsList() {
            var refs = m_GetComponentRefs();
            var list = new List<Component>();

            foreach (var compRef in refs) {
                if (compRef.value != 0) {
                    var component = GetObjectByRef(compRef) as Component;
                    if (component.GetType() != typeof(CSComponent))
                        list.Add(component);
                }
            }
            return list;
        }

        public bool Has(Flag flags) {
            return (Flags & flags) == flags;
        }

    #endregion
    #region Private

        private void m_GetComponents<TComponent>(ref List<TComponent> list) where TComponent : Component {
            var refs = m_GetComponentRefs();

            foreach (var compRef in refs) {
                if (compRef.value != 0) {
                    var component = GetObjectByRef(compRef);
                    if (component is TComponent)
                        list.Add(component as TComponent);
                }
            }
        }

        private void m_GetComponentsInChild<TComponent>(ref List<TComponent> list) where TComponent : Component {
            m_GetComponents(ref list);

            int count = GetChildrenCount();
            for (int i = 0; i < count; i++) {
                var child = GetChild(i);
                if (child != null)
                    m_GetComponentsInChild(ref list);
            }
        }

        /// <summary>
        /// Возвращает список ссылок на прикрепленные к объекту компоненты.
        /// </summary>
        /// <returns>C# ссылка на компонент. Если компонента нет в C#, ссылка будет невалидна. </returns>
        private CsRef[] m_GetComponentRefs() {
            int length = Dll.Actor.GetComponentsCount(cppRef);
            CsRef[] refs = new CsRef[length];

            unsafe {
                IntPtr ptr = Marshal.AllocHGlobal(length * Marshal.SizeOf(typeof(ulong)));
                var refPtr = (ulong*)ptr.ToPointer();

                Dll.Actor.WriteComponentsRefs(cppRef, (ulong)refPtr);

                for (int i = 0; i < length; i++, refPtr++) {
                    ulong value = *refPtr;
                    refs[i] = new CsRef();
                    refs[i].value = value;
                }
                Marshal.FreeHGlobal(ptr);
            }

            return refs;
        }

        public static void SetPrefabId(CsRef actorRef, int prefabGuidHash) {
            var actor = CppLinked.GetObjectByRef(actorRef) as Actor;

            var prefabGuid = "";
            if (prefabGuidHash != 0)
                prefabGuid = AssetStore.GetAssetGuid(prefabGuidHash);

            actor.prefabId = prefabGuid;
        }

    #endregion
    #region Cpp

        private static CppRef cpp_Create() {
            //Console.WriteLine($"#: GameObject.cpp_Create()");

            var gameObject = new Actor();
            return gameObject.cppRef;
        }


        private static CsRef cpp_AddComponent(CsRef objRef, ulong ptr, ulong length, CppObjectInfo info) {

            string name = Assets.ReadCString(ptr, length);

            //Console.WriteLine($"#: GameObject({objRef}).cpp_AddComponent(\"{name}\") -> ");

            var componentType = Type.GetType(name);
            var addComponent = typeof(Actor).GetMethod(nameof(Actor.m_AddComponentFromCpp), BindingFlags.NonPublic | BindingFlags.Instance);

            addComponent = addComponent.MakeGenericMethod(componentType);

            var gameObject = CppLinked.GetObjectByRef(objRef) as Actor;
            var result = addComponent.Invoke(gameObject, new object[] { info });

            return (CsRef)result;
        }

        private static CppRef cpp_AddCsComponent(CsRef objRef, ulong ptr, ulong length) {
            //Console.WriteLine($"#: GameObject.cpp_AddCsComponent(): obj: {objRef} -> ");

            string name = Assets.ReadCString(ptr, length);

            var componentType = Type.GetType(name);
            var componentCppRefProp = componentType.GetProperty(nameof(CppLinked.cppRef));
            //var componentCsRefProp = componentType.GetProperty(nameof(CppLinked.csRef));

            var addComponent = typeof(Actor).GetMethod(nameof(Actor.AddComponent));
            addComponent = addComponent.MakeGenericMethod(componentType);

            var gameObject = CppLinked.GetObjectByRef(objRef) as Actor;
            var component = addComponent.Invoke(gameObject, null);

            var cppRef = (CppRef)componentCppRefProp.GetValue(component);
            //var csRef = (CsRef)componentCsRefProp.GetValue(component);

            //Console.WriteLine($"#: -> {csRef}, {cppRef} ");

            return cppRef;
        }

        //private static void cpp_SetName(CsRef objRef, ulong ptr, ulong length) {
        //    var actor = CppLinked.GetObjectByRef(objRef) as Actor;
        //    actor.Name = ReadCString(ptr, length);

        //    //Console.WriteLine($"#: Actor.cpp_SetName('{actor.Name}')");
        //}

        //private static string ReadCString(ulong ptr, ulong length) {
        //    string str = "";
        //    unsafe {
        //        byte[] bytes = new byte[length];
        //        byte* bptr = (byte*)ptr;
        //        for (ulong i = 0; i < length; i++, bptr++) {
        //            bytes[i] = *bptr;
        //        }
        //        str = Encoding.UTF8.GetString(bytes);
        //    }
        //    return str;
        //}

        // private static void cpp_InitComponent(CsRef compRef) {
        //     var component = GetObjectByRef(compRef) as Component;

        //     component.OnInit();
        // }

        // private static void cpp_StartComponent(CsRef compRef) {
        //     var component = GetObjectByRef(compRef) as Component;

        //     component.OnStart();
        // }

        // private static void cpp_UpdateComponent(CsRef compRef) {
        //     var component = GetObjectByRef(compRef) as Component;

        //     component.OnUpdate();
        // }

        // private static void cpp_DestroyComponent(CsRef compRef) {
        //     var component = GetObjectByRef(compRef) as Component;

        //     component.OnDestroy();

        //     RemoveCsRef(compRef);
        // }

    #endregion
    }

#endif



}
