using System;
using System.Runtime.InteropServices;

namespace EngineMono {

    public class MonoClass {
        public const string ExePath = "../Core.exe";

        private UInt64 m_refId;

        public void Func() {
            Console.WriteLine("From C# with fire!");
        }

        public void SetRef(UInt64 refId) {
            m_refId = refId; 
        }

        public void CallCppFunc(float value) {
            Console.WriteLine($"CS: CallCppFunc({value})");
            try {
                CppFunc(m_refId, value);
            }
            catch (Exception ex) {
                Console.WriteLine($"CS: {ex.GetType().Name}='{ex.Message}'");
            }
        }

        [DllImport(ExePath, EntryPoint = "setValue")]
        public static extern void CppFunc(UInt64 refId, float value);

        //[DllImport(ExePath, EntryPoint = "setValue")]
        //public static extern void CppFunc();
    }
}
