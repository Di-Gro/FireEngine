using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

using EngineDll;


namespace Engine {
    static class Input {

        public static Action OnMouseMove;
        public static Action OnWheelMove;


        public static CppRef s_hotkeysRef;

        public static Vector2 MousePosition => Dll.Input.MousePosition(s_hotkeysRef);

        public static Vector2 MouseDelta => Dll.Input.mouseDelta_get(s_hotkeysRef);
        public static int WheelDelta => Dll.Input.wheelDelta_get(s_hotkeysRef);

        public static bool GetButtonDown(Key key) => Dll.Input.GetButtonDown(s_hotkeysRef, (int)key);
        public static bool GetButtonUp(Key key) => Dll.Input.GetButtonUp(s_hotkeysRef, (int)key);
        public static bool GetButton(Key key) => Dll.Input.GetButton(s_hotkeysRef, (int)key);


        public static void OnUpdate() {
            if(MouseDelta.X != 0 || MouseDelta.Y != 0)
                OnMouseMove?.Invoke();

            if (WheelDelta != 0)
                OnWheelMove?.Invoke();
        }

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
                Dll.Input.RegisterHotkey(s_hotkeysRef, (int)key);
            }
        }

        private static void StopListeningAllKeys() {
            foreach (Key key in Enum.GetValues(typeof(Key))) {
                Dll.Input.UnregisterHotkey(s_hotkeysRef, (int)key);
            }
        }

    }
}
