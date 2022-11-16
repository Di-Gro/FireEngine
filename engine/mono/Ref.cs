using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

namespace EngineMono {

    class Ref {

        public static unsafe void* GetPointer(UInt64 objectRefId) {
            void* ptr = m_GetPointer(objectRefId);

            if (ptr == null)
                throw new NullReferenceException("Object already destroyed or not exist.");

            return ptr;
        }

        public static TValue GetFieldValue<TValue>(ClassInfo classInfo, UInt64 objectRefId, int fieldIndex)
            where TValue : unmanaged
        {
            unsafe {
                var ptr = (byte*)Ref.GetPointer(objectRefId);
                var fieldPtr = ptr + classInfo.offsets[fieldIndex];

                return *((TValue*)fieldPtr);
            }
        }

        public static void SetFieldValue<TValue>(ClassInfo classInfo, UInt64 objectRefId, int fieldIndex, TValue value)
           where TValue : unmanaged {
            unsafe {
                var ptr = (byte*)Ref.GetPointer(objectRefId);
                var fieldPtr = ptr + classInfo.offsets[fieldIndex];

                *((TValue*)fieldPtr) = value;
            }
        }


        [DllImport(MonoClass.ExePath, EntryPoint = "Ref_GetPointer")]
        private static extern unsafe void* m_GetPointer(UInt64 objectRefId);


    }
}
