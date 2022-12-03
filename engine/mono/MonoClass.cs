using System;
using System.Runtime.InteropServices;

namespace EngineMono {

    public class MonoClass {
        
        private UInt64 m_refId;

        public void Func() {
            Console.WriteLine("From C# with fire!");
        }

        public void SetRef(UInt64 refId) {
            m_refId = refId; 
        }

        public void CallCppFunc(float value) {
            Console.WriteLine($"#: CallCppFunc({value})");
            try {
                CppFunc(m_refId, value);
            }
            catch (Exception ex) {
                Console.WriteLine($"#: {ex.GetType().Name}='{ex.Message}'");
            }
        }

        public UInt64 CreateInstance(UInt64 refId, string name) {
            var type = Type.GetType(name);
            var obj = Activator.CreateInstance(type);


            return 0;
        }


        [DllImport(Engine.Paths.Exe, EntryPoint = "setValue")]
        public static extern void CppFunc(UInt64 refId, float value);

        //[DllImport(ExePath, EntryPoint = "setValue")]
        //public static extern void CppFunc();
    }
}
