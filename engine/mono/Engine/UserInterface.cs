using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;
using UI;

namespace Engine
{
    class UserInterface
    {
        public static UserInterface Instance { get; private set; }

        [StructLayout(LayoutKind.Sequential)]
        public struct Callbacks
        {
            public delegate void Callback(CsRef csRef);

            public Callback onDrawComponent;
        }

        private Callbacks _callbacks;

        private CppRef _cppRef;
        public static void cpp_Init(CppRef cppRef)
        {
            Instance = new UserInterface();
            Instance._cppRef = cppRef;
            Instance._callbacks = new Callbacks();
            Instance._callbacks.onDrawComponent = new Callbacks.Callback(Instance.OnDrawComponent);
            dll_SetCallbacks(cppRef, Instance._callbacks.onDrawComponent);
        }

        //public static void OnDrawComponent(ulong csref)
        public void OnDrawComponent(CsRef csRef)
        {
            //Console.WriteLine($"OnDrawComponent(): {csRef}");
            //ImGui.Begin("Hello World!");
            //var _object = CppLinked.GetObjectByRef(csref);

            //if(_object != null)
            //{
            //    ImGui.Text(csref.ToString());
            //}

            //ImGui.End();
            //ImGui.DragFloat();
        }

        [DllImport(Paths.Exe, EntryPoint = "UserInterface_SetCallbacks")]
        private static extern void dll_SetCallbacks(CppRef cppRef, Callbacks.Callback callbacks);
    }
}
