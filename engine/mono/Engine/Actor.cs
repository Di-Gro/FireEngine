using System;
using System.Reflection;
using System.Collections.Generic;
using System.Text;
using EngineMono;
using System.Runtime.InteropServices;

using EngineDll;

namespace Engine {

    sealed class Actor : CppLinked {
        //private CsRef m_transformRef;

        public Vector3 localPosition {
            get => Dll.Actor.localPosition_get(cppRef);
            set => Dll.Actor.localPosition_set(cppRef, value);
        }

        public Vector3 localRotation {
            get => Dll.Actor.localRotation_get(cppRef);
            set => Dll.Actor.localRotation_set(cppRef, value);
        }

        public Quaternion localRotationQ {
            get => Dll.Actor.localRotationQ_get(cppRef);
            set => Dll.Actor.localRotationQ_set(cppRef, value);
        }

        public Vector3 localScale {
            get => Dll.Actor.localScale_get(cppRef);
            set => Dll.Actor.localScale_set(cppRef, value);
        }

        public Vector3 localForward => Dll.Actor.localForward_get(cppRef);
        public Vector3 localUp => Dll.Actor.localUp_get(cppRef);
        public Vector3 localRight => Dll.Actor.localRight_get(cppRef);

        public Vector3 forward => Dll.Actor.forward_get(cppRef);
        public Vector3 up => Dll.Actor.up_get(cppRef);
        public Vector3 right => Dll.Actor.right_get(cppRef);

        //public Transform transform => GetObjectByRef(m_transformRef) as Transform;

        public Actor parent {
            get => (Actor)GetObjectByRef(Dll.Actor.parent_get(cppRef));
            set => Dll.Actor.parent_set(cppRef, value.cppRef);
        }

        public bool IsDestroyed => GetObjectByRef(csRef) == null;

        public int GetChildrenCount() => Dll.Actor.GetChildrenCount(cppRef);


        #region Public

        public Actor() : this("GameObject") { }

        public Actor(string name) {
            //Console.WriteLine($"#: GameObject(\"{name}\"): {csRef}, -> ");

            var info = Dll.Game.CreateGameObjectFromCS(Game.gameRef, csRef, name);
            Link(info.classRef, info.objectRef);

            //m_transformRef = info.transformRef;

            //Console.WriteLine($"#: GameObject({csRef}, {cppRef}): \"{name}\"");
        }


        public void Destroy() {
            Dll.Actor.Destroy(cppRef);
            RemoveObjectByRef(csRef);
        }

        ~Actor() {
            //Console.WriteLine($"#: ~GameObject({csRef}, {cppRef})(): {cppRef}, {csRef}");
        }

        public TComponent AddComponent<TComponent>() where TComponent : Component, new() {
            var component = new TComponent();
            component.LinkGameObject(csRef/*, transform.csRef*/);

            //Console.WriteLine($"#: GameObject({csRef}, {cppRef}).AddComponent<{typeof(TComponent).Name}>(): {component.csRef}");

            var info = component.CreateFromCS(this);
            component.Link(info.classRef, info.objectRef);
            Dll.Actor.InitComponent(cppRef, component.cppRef);

            return component;
        }

        private CsRef m_AddComponentFromCpp<TComponent>(CppObjectInfo info) where TComponent : Component, new() {
            var component = new TComponent();
            component.LinkGameObject(csRef/*, transform.csRef*/);

            //Console.WriteLine($"#: GameObject({csRef}, {cppRef}).m_AddComponentFromCpp<{typeof(TComponent).Name}>({component.csRef}, {info.objectRef})");

            component.Link(info.classRef, info.objectRef);

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

        public Actor GetChild(int index) {
            if (index < 0 || index >= GetChildrenCount())
                return null;

            var childRef = Dll.Actor.GetChild(cppRef, index);
            return GetObjectByRef(childRef) as Actor;
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

        #endregion
        #region Cpp

        private static CppRef cpp_Create() {
            //Console.WriteLine($"#: GameObject.cpp_Create()");

            var gameObject = new Actor();
            return gameObject.cppRef;
        }

  
        private static CsRef cpp_AddComponent(CsRef objRef, ulong ptr, ulong length, CppObjectInfo info) {

            string name = ReadCString(ptr, length);

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

            string name = ReadCString(ptr, length);

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
    }

}
