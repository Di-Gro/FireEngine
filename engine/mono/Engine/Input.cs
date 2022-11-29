using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

using EngineMono;


namespace Engine {
    static class Input {

        public static CppRef s_hotkeysRef;

        public static Vector2 MousePosition => dll_MousePosition(s_hotkeysRef);
        public static int WheelDelta => dll_wheelDelta_get(s_hotkeysRef);

        public static bool GetButtonDown(Key key) => dll_GetButtonDown(s_hotkeysRef, (int)key);
        public static bool GetButtonUp(Key key) => dll_GetButtonUp(s_hotkeysRef, (int)key);
        public static bool GetButton(Key key) => dll_GetButton(s_hotkeysRef, (int)key);


        private static void cpp_OnInit(CppRef objRef) {
            Console.WriteLine($"#: Input.cpp_OnInit(): {objRef}");

            s_hotkeysRef = objRef;

            StartListeningAllKeys();
        }

        private static void cpp_OnDestroy() {
            StopListeningAllKeys();
        }

        private static void StartListeningAllKeys() {
            foreach(Key key in Enum.GetValues(typeof(Key))) {
                //Console.WriteLine($"#: Input.StartListening: {key}");
                dll_RegisterHotkey(s_hotkeysRef, (int)key);
            }
        }

        private static void StopListeningAllKeys() {
            foreach (Key key in Enum.GetValues(typeof(Key))) {
                //Console.WriteLine($"#: Input.StartListening: {key}");
                dll_UnregisterHotkey(s_hotkeysRef, (int)key);
            }
        }


        [DllImport(MonoClass.ExePath, EntryPoint = "HotKeys_GetButtonDown")]
        private static extern bool dll_GetButtonDown(CppRef objRef, int keyCode);

        [DllImport(MonoClass.ExePath, EntryPoint = "HotKeys_GetButtonUp")]
        private static extern bool dll_GetButtonUp(CppRef objRef, int keyCode);

        [DllImport(MonoClass.ExePath, EntryPoint = "HotKeys_GetButton")]
        private static extern bool dll_GetButton(CppRef objRef, int keyCode);


        [DllImport(MonoClass.ExePath, EntryPoint = "HotKeys_RegisterHotkey")]
        private static extern void dll_RegisterHotkey(CppRef objRef, int keyCode);

        [DllImport(MonoClass.ExePath, EntryPoint = "HotKeys_UnregisterHotkey")]
        private static extern void dll_UnregisterHotkey(CppRef objRef, int keyCode);


        [DllImport(MonoClass.ExePath, EntryPoint = "HotKeys" + "_" + "MousePosition")]
        private static extern Vector2 dll_MousePosition(CppRef objRef);

        [DllImport(MonoClass.ExePath, EntryPoint = "HotKeys" + "_" + "wheelDelta_get")]
        private static extern int dll_wheelDelta_get(CppRef objRef);

    }
}
