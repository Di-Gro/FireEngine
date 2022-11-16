using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

namespace EngineMono {

	class CSClass : CppLinked {

		/// напрямую в C# через указатели
		public float floatValue { 
			get => prop_floatValue.value; 
			set => prop_floatValue.value = value; 
		}

		public int intValue { 
			get => prop_intValue.value; 
			set => prop_intValue.value = value; 
		}

		private Prop<float> prop_floatValue = new Prop<float>();
		private Prop<int> prop_intValue = new Prop<int>();

		/// через геттеры и сеттеры в C++
		public long longValue { 
			get => m_longValue_get(objectRef); 
			set => m_longValue_set(objectRef, value); 
		}

		/// функция из C++
		public float SomeFunc() => m_SomeFunc(objectRef);

		/// Связывание с C++ объектом
		public override void Link(UInt64 classInfoRef, UInt64 objRef) {
            base.Link(classInfoRef, objRef);

            prop_floatValue.Link(this, 0);
            prop_intValue.Link(this, 1);
        }

        /// Связывание с DLL

        [DllImport(MonoClass.ExePath, EntryPoint = "CppClass_longValue_get")]
		private static extern long m_longValue_get(UInt64 objRef);

		[DllImport(MonoClass.ExePath, EntryPoint = "CppClass_longValue_set")]
		private static extern void m_longValue_set(UInt64 objRef, long value);

		[DllImport(MonoClass.ExePath, EntryPoint = "CppClass_SomeFunc")]
		private static extern float m_SomeFunc(UInt64 objRef);

		//public override void Link(UInt64 classInfoRef, UInt64 objRef) {
			//base.Link(classInfoRef, objRef);

			//Console.WriteLine($"CS: CSClass.Link()");

			//prop_floatValue.Link(this, 0);
			//prop_intValue.Link(this, 1);

			//Console.WriteLine($"CS: floatValue = {floatValue}");
			//Console.WriteLine($"CS: intValue = {intValue}");
			//Console.WriteLine($"CS: longValue = {longValue}");

			//floatValue = 7.84f;
			//intValue = 555;
			//longValue = 1611;

			//Console.WriteLine($"CS: floatValue = {floatValue}");
			//Console.WriteLine($"CS: intValue = {intValue}");
			//Console.WriteLine($"CS: longValue = {longValue}");
		//}

	};
}
