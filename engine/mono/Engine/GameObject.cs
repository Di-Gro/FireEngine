using System;
using System.Reflection;
using System.Collections.Generic;
using System.Text;
using EngineMono;
using System.Runtime.InteropServices;


namespace Engine {

    sealed class GameObject : CppLinked {
        private CsRef m_transformRef;


        public Transform transform => GetObjectByRef(m_transformRef) as Transform;

        public GameObject parent {
            get => (GameObject)GetObjectByRef(dll_parent_get(cppRef));
            set => dll_parent_set(cppRef, value.cppRef);
        }

        public bool IsDestroyed => GetObjectByRef(csRef) == null;

        public int GetChildrenCount() => dll_GetChildrenCount(cppRef);


        #region Public

        public GameObject() : this("GameObject") { }

        public GameObject(string name) {
            //Console.WriteLine($"#: GameObject(\"{name}\"): {csRef}, -> ");

            var info = dll_CreateGameObjectFromCS(Game.gameRef, csRef, name);
            Link(info.classRef, info.objectRef);

            m_transformRef = info.transformRef;

            Console.WriteLine($"#: GameObject({csRef}, {cppRef}): \"{name}\"");
        }


        public void Destroy() {
            dll_Destroy(cppRef);
            RemoveObjectByRef(csRef);
        }

        ~GameObject() {
            //Console.WriteLine($"#: ~GameObject({csRef}, {cppRef})(): {cppRef}, {csRef}");
        }

        public TComponent AddComponent<TComponent>() where TComponent : Component, new() {
            var component = new TComponent();
            component.LinkGameObject(csRef, transform.csRef);

            Console.WriteLine($"#: GameObject({csRef}, {cppRef}).AddComponent<{typeof(TComponent).Name}>(): {component.csRef}");

            var info = component.CreateFromCS(this);
            component.Link(info.classRef, info.objectRef);
            dll_InitComponent(cppRef, component.cppRef);

            return component;
        }

        private CsRef m_AddComponentFromCpp<TComponent>(CppObjectInfo info) where TComponent : Component, new() {
            var component = new TComponent();
            component.LinkGameObject(csRef, transform.csRef);

            //Console.WriteLine($"#: GameObject({csRef}, {cppRef}).m_AddComponentFromCpp<{typeof(TComponent).Name}>({component.csRef}, {info.objectRef})");

            component.Link(info.classRef, info.objectRef);

            return component.csRef;
        }

        public TComponent GetComponent<TComponent>() where TComponent : Component {
            var refs = m_GetComponentRefs();

            foreach (var compRef in refs) {
                var component = GetObjectByRef(compRef);
                if (component is TComponent)
                    return component as TComponent;
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
                if(child != null) {
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

        public GameObject GetChild(int index) {
            if (index < 0 || index >= GetChildrenCount())
                return null;

            var childRef = dll_GetChild(cppRef, index);
            return GetObjectByRef(childRef) as GameObject;
        }

        #endregion
        #region Private

        private void m_GetComponents<TComponent>(ref List<TComponent> list) where TComponent : Component {
            var refs = m_GetComponentRefs();

            foreach (var compRef in refs) {
                var component = GetObjectByRef(compRef);
                if (component is TComponent)
                    list.Add(component as TComponent);
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

        private CsRef[] m_GetComponentRefs() {
            int length = dll_GetComponentsCount(cppRef);
            CsRef[] refs = new CsRef[length];

            unsafe {
                IntPtr ptr = Marshal.AllocHGlobal(length * Marshal.SizeOf(typeof(ulong)));
                var refPtr = (ulong*)ptr.ToPointer();

                dll_WriteComponentsRefs(cppRef, (ulong)refPtr);

                for (int i = 0; i < length; i++, refPtr++) {
                    ulong value = *refPtr;
                    refs[i] = new CsRef();
                    refs[i].value = value;
                }
                Marshal.FreeHGlobal(ptr);
            }

            //Console.Write($"#: GameObject.m_GetComponentsRefs(): refs({refs.Length}): ");
            //foreach (var compRef in refs) {
            //    Console.Write($"{compRef}, ");
            //}
            //Console.WriteLine();

            return refs;
        }

        #endregion
        #region Cpp

        private static CppRef cpp_Create() {
            //Console.WriteLine($"#: GameObject.cpp_Create()");

            var gameObject = new GameObject();
            return gameObject.cppRef;
        }

  
        private static CsRef cpp_AddComponent(CsRef objRef, ulong ptr, ulong length, CppObjectInfo info) {

            string name = ReadCString(ptr, length);

            //Console.WriteLine($"#: GameObject({objRef}).cpp_AddComponent(\"{name}\") -> ");

            var componentType = Type.GetType(name);
            var addComponent = typeof(GameObject).GetMethod(nameof(GameObject.m_AddComponentFromCpp), BindingFlags.NonPublic | BindingFlags.Instance);

            addComponent = addComponent.MakeGenericMethod(componentType);

            var gameObject = CppLinked.GetObjectByRef(objRef) as GameObject;
            var result = addComponent.Invoke(gameObject, new object[] { info });

            return (CsRef)result;
        }

        private static CppRef cpp_AddCsComponent(CsRef objRef, ulong ptr, ulong length) {
            //Console.WriteLine($"#: GameObject.cpp_AddCsComponent(): obj: {objRef} -> ");

            string name = ReadCString(ptr, length);

            var componentType = Type.GetType(name);
            var componentCppRefProp = componentType.GetProperty(nameof(CppLinked.cppRef));
            //var componentCsRefProp = componentType.GetProperty(nameof(CppLinked.csRef));

            var addComponent = typeof(GameObject).GetMethod(nameof(GameObject.AddComponent));
            addComponent = addComponent.MakeGenericMethod(componentType);

            var gameObject = CppLinked.GetObjectByRef(objRef) as GameObject;
            var component = addComponent.Invoke(gameObject, null);

            var cppRef = (CppRef)componentCppRefProp.GetValue(component);
            //var csRef = (CsRef)componentCsRefProp.GetValue(component);

            //Console.WriteLine($"#: -> {csRef}, {cppRef} ");

            return cppRef;
        }

        private static string ReadCString(ulong ptr, ulong length) {
            string str = "";
            unsafe {
                byte[] bytes = new byte[length];
                byte* bptr = (byte*)ptr;
                for (ulong i = 0; i < length; i++, bptr++) {
                    bytes[i] = *bptr;
                }
                str = Encoding.UTF8.GetString(bytes);
            }
            return str;
        }

        private static void cpp_InitComponent(CsRef compRef) {
            var component = GetObjectByRef(compRef) as Component;

            component.OnInit();
        }

        private static void cpp_StartComponent(CsRef compRef) {
            var component = GetObjectByRef(compRef) as Component;

            component.OnStart();
        }

        private static void cpp_UpdateComponent(CsRef compRef) {
            var component = GetObjectByRef(compRef) as Component;

            component.OnUpdate();
        }

        private static void cpp_DestroyComponent(CsRef compRef) {
            var component = GetObjectByRef(compRef) as Component;
            
            component.OnDestroy();

            RemoveObjectByRef(compRef);
        }

        #endregion
        #region Dll


        [DllImport(MonoClass.ExePath, EntryPoint = "Game_CreateGameObjectFromCS", CharSet = CharSet.Ansi)]
        private static extern GameObjectInfo dll_CreateGameObjectFromCS(CppRef gameRef, CsRef objRef, string name);

        [DllImport(MonoClass.ExePath, EntryPoint = "GameObject_InitComponent")]
        private static extern void dll_InitComponent(CppRef objRef, CppRef compRef);

        [DllImport(MonoClass.ExePath, EntryPoint = "GameObject_Destroy")]
        private static extern void dll_Destroy(CppRef objRef);

        [DllImport(MonoClass.ExePath, EntryPoint = "GameObject_GetComponentsCount")]
        private static extern int dll_GetComponentsCount(CppRef objRef);

        [DllImport(MonoClass.ExePath, EntryPoint = "GameObject_WriteComponentsRefs")]
        private static extern void dll_WriteComponentsRefs(CppRef objRef, ulong listPtr);

        [DllImport(MonoClass.ExePath, EntryPoint = "GameObject_GetChildrenCount")]
        private static extern int dll_GetChildrenCount(CppRef objRef);

        [DllImport(MonoClass.ExePath, EntryPoint = "GameObject_GetChild")]
        private static extern CsRef dll_GetChild(CppRef objRef, int index);

        [DllImport(MonoClass.ExePath, EntryPoint = "GameObject_parent_get")]
        private static extern CsRef dll_parent_get(CppRef objRef);

        [DllImport(MonoClass.ExePath, EntryPoint = "GameObject_parent_set")]
        private static extern void dll_parent_set(CppRef objRef, CppRef parent);

        #endregion
    }

}
