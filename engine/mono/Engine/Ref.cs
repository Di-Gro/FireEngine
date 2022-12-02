using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

namespace Engine {

    [StructLayout(LayoutKind.Sequential)]
    public struct CppRef {
        public ulong value;

        CppRef(ulong v) => value = v;

        public static implicit operator CppRef(ulong v) => new CppRef { value = v };

        public override string ToString() => $"cpp:{value}";
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct CsRef {
        public UInt64 value;

        CsRef(UInt64 v) => value = v;

        public static implicit operator CsRef(ulong v) => new CsRef { value = v };

        public override string ToString() => $"cs:{value}";
    }

    class Ref {

        public static unsafe void* GetPointer(CppRef objectRefId) {
            void* ptr = m_GetPointer(objectRefId);

            if (ptr == null)
                throw new NullReferenceException("Object already destroyed or not exist.");

            return ptr;
        }

        public static TValue GetFieldValue<TValue>(ClassInfo classInfo, CppRef objectRefId, int fieldIndex)
            where TValue : unmanaged
        {
            unsafe {
                var ptr = (byte*)Ref.GetPointer(objectRefId);
                var fieldPtr = ptr + classInfo.offsets[fieldIndex];

                return *((TValue*)fieldPtr);
            }
        }

        public static void SetFieldValue<TValue>(ClassInfo classInfo, CppRef objectRefId, int fieldIndex, TValue value)
           where TValue : unmanaged {
            unsafe {
                var ptr = (byte*)Ref.GetPointer(objectRefId);
                var fieldPtr = ptr + classInfo.offsets[fieldIndex];

                *((TValue*)fieldPtr) = value;
            }
        }

        public static bool IsValid(CppRef cppRef) {
            unsafe {
                return Ref.GetPointer(cppRef) != null;
            }
        }


        [DllImport(EngineMono.MonoClass.ExePath, EntryPoint = "Ref_GetPointer")]
        private static extern unsafe void* m_GetPointer(CppRef objectRefId);


    }
}
