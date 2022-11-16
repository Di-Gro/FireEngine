using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

namespace EngineMono {

    class ClassInfo {

        public UInt64 classInfoRef { get; private set; }

        public int[] offsets { get; private set; }


        public static ClassInfo GetClassInfo(UInt64 classInfoRef) {
            Console.WriteLine($"CS: ClassInfo.GetClassInfo()");

            ClassInfo info = new ClassInfo();
            info.classInfoRef = classInfoRef;

            unsafe {
                var structPtr = (ClassInfoStruct*)Ref.GetPointer(classInfoRef);

                info.offsets = new int[structPtr->offsetCount];

                for (int i = 0; i < info.offsets.Length; i++)
                    info.offsets[i] = *(structPtr->offsetsPtr + i);
            }
            return info;
        }

        [StructLayout(LayoutKind.Sequential)]
        private unsafe struct ClassInfoStruct {
            public int offsetCount;
            public int* offsetsPtr;
        }

        //[DllImport(MonoClass.ExePath, EntryPoint = "CppClass_GetClassInfo")]
        //private static extern void GetClassInfo(UInt64 classInfoRefId, out ClassInfoStruct classInfo);

        //public static ClassInfo GetClassInfo(UInt64 classInfoRef) {
        //    Console.WriteLine($"CS: ClassInfo.GetClassInfo()");

        //    ClassInfo info = new ClassInfo();

        //    unsafe {
        //        var structPtr = (ClassInfoStruct*)Ref.GetPointer(classInfoRef);

        //        Console.WriteLine($"CS: classInfo ({structPtr->offsetCount}, {(ulong)structPtr->offsetsPtr})");

        //        info.offsets = new int[structPtr->offsetCount];

        //        for (int i = 0; i < info.offsets.Length; i++)
        //            info.offsets[i] = *(structPtr->offsetsPtr + i);

        //        for (int i = 0; i < info.offsets.Length; i++)
        //            Console.WriteLine($"{info.offsets[i]}");
        //    }
        //    return info;
        //}
    }

    //[StructLayout(LayoutKind.Sequential)]
    //unsafe struct ClassInfo {
    //    public int offsetCount;
    //    public int* offsetsPtr;
    //}


}
