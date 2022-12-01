using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

using EngineMono;


namespace Engine {

    abstract class Component : CppLinked {
        private CsRef m_objectRef;
        private CsRef m_transformRef;


        public bool IsDestroyed => GetObjectByRef(csRef) == null;

        public GameObject gameObject => GetObjectByRef(m_objectRef) as GameObject;

        public Transform transform => GetObjectByRef(m_transformRef) as Transform;

        public abstract CppObjectInfo CreateFromCS(GameObject target);

        public virtual void OnInit() { }
        public virtual void OnStart() { }
        public virtual void OnUpdate() { }
        public virtual void OnDestroy() { }


        public void Destroy() => dll_Destroy(cppRef);

        public void LinkGameObject(CsRef objectRef, CsRef transformRef) {
            //Console.WriteLine($"#: {GetType().Name}({csRef}, {cppRef}).LinkGameObject({objectRef}, {transformRef})");

            m_objectRef = objectRef;
            m_transformRef = transformRef;
        }


        [DllImport(MonoClass.ExePath, EntryPoint = "GameObject_DestroyComponent")]
        private static extern void dll_Destroy(CppRef compRef);

        [DllImport(MonoClass.ExePath, EntryPoint = "GameObject_gameObject_get")]
        private static extern CsRef dll_gameObject_get(CppRef compRef);
    }

    /// <summary>
    /// Базовый класс для C# компонента.
    /// </summary>
    abstract class CSComponent : Component {

        public override CppObjectInfo CreateFromCS(GameObject target) {
            return dll_Create(target.cppRef, csRef);
        }

        [DllImport(MonoClass.ExePath, EntryPoint = "CsComponent_Create")]
        private static extern CppObjectInfo dll_Create(CppRef cppObjRef, CsRef csCompRef);

    }


    /// <summary>
    /// C# компонент.
    /// </summary>
    class UserComponent : CSComponent {
        public override void OnInit() {
            //Console.WriteLine($"#: {GetType().Name}({csRef}, {cppRef}).OnInit()");
        }

        public override void OnStart() {
            //Console.WriteLine($"#: {GetType().Name}({csRef}, {cppRef}).OnStart()");
        }

        public override void OnUpdate() {
            //Console.WriteLine($"#: {GetType().Name}({csRef}, {cppRef}).OnUpdate()");

            //if (Input.GetButtonDown(Key.Backspace)) {
            //    Console.WriteLine($"#: YYYY____________YYYYYYYYYYYYYYYYYYYYYYYYYYYY");
            //}
            //if (Input.GetButton(Key.Backspace)) {
            //    Console.WriteLine($"#: YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY");
            //}
            //if (Input.GetButtonUp(Key.Backspace)) {
            //    Console.WriteLine($"#: YYYYYYYYYYYYYYYYYYYYYYYYYYY____________YYYYY");
            //}

        }

        public override void OnDestroy() {
            //Console.WriteLine($"#: {GetType().Name}({csRef}, {cppRef}).OnDestroy()");
        }

    }

    /// <summary>
    /// Базовый класс для C# тени C++ компонента.
    /// </summary>
    abstract class CppComponent : Component {

    }

    /// <summary>
    /// C# тень C++ компонента.
    /// </summary>
    class ConcreteCppCpmponent : CppComponent {

        public override void OnInit() {
            //Console.WriteLine($"#: {GetType().Name}({csRef}, {cppRef}).OnInit()");
        }

        public override void OnStart() {
            //Console.WriteLine($"#: {GetType().Name}({csRef}, {cppRef}).OnStart()");
        }

        public override void OnUpdate() {
            //Console.WriteLine($"#: {GetType().Name}({csRef}, {cppRef}):OnUpdate()");
        }

        public override void OnDestroy() {
            //Console.WriteLine($"#: {GetType().Name}({csRef}, {cppRef}).OnDestroy()");
        }


        public override CppObjectInfo CreateFromCS(GameObject target) {
            return dll_Create(target.cppRef, csRef);
        }

        [DllImport(MonoClass.ExePath, EntryPoint = "CppComponent_Create")]
        private static extern CppObjectInfo dll_Create(CppRef cppObjRef, CsRef csCompRef);

    }


    ///// <summary>
    ///// C# тень CPP_ONLY компонента.
    ///// </summary>
    //class CppOnlyCpmponent : CppComponent {

    //    public override CppObjectInfo CreateFromCS(GameObject target) {
    //        return dll_Create(target.cppRef, csRef);
    //    }

    //    [DllImport(MonoClass.ExePath, EntryPoint = "CppOnlyCpmponent_Create")]
    //    private static extern CppObjectInfo dll_Create(CppRef cppObjRef, CsRef csCompRef);

    //}
}
