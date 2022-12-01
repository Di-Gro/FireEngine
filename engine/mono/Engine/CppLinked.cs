using System;
using System.Collections.Generic;
using System.Reflection;
using System.Text;


namespace Engine {
	interface ICppLinked {
		ClassInfo classInfo { get; }
		CppRef cppRef { get; }
		CsRef csRef { get; }

		CsRef Link(CppRef classInfoRef, CppRef objRef);
	}

	class CppLinked : ICppLinked {
		/// Static ->
		private static Dictionary<CsRef, object> s_refs = new Dictionary<CsRef, object>();
		private static CsRef s_nextRefId = 1;

		public static object GetObjectByRef(CsRef csRef) {
			if (s_refs.ContainsKey(csRef))
				return s_refs[csRef];
			Console.WriteLine($"#: CppLinked.GetObjectByRef: NULL)");
			return null;
		}
		public static void RemoveObjectByRef(CsRef csRef) {
			//Console.WriteLine($"#: CppLinked.RemoveObjectByRef({csRef})");
			s_refs.Remove(csRef);
		}
		/// <- Static


		//private ulong m_csRefId;

		public ClassInfo classInfo { get; private set; }

		public CppRef cppRef { get; private set; }
		public CsRef csRef { get; private set; }

		public CppLinked() {
			csRef = s_nextRefId.value;
			s_refs.Add(csRef, this);
			s_nextRefId.value++;
		}

		public virtual CsRef Link(CppRef classInfoRef, CppRef objRef) {
			cppRef = objRef;

			//Console.WriteLine($"#: {GetType().Name}({csRef}, {cppRef}).Link(class:{classInfoRef}, obj:{objRef})");

			classInfo = ClassInfo.GetClassInfo(classInfoRef);

			LinkProps();
			return csRef;
		}

		private void LinkProps() {
			var type = GetType();

			//Console.WriteLine($"#: {type.Name}.LinkProps() ->");

			BindingFlags bindFlags = BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance;

			//Console.WriteLine($"#: LinkProps:");
			foreach (var field in type.GetFields(bindFlags)) {
				var iprop = field.FieldType.GetInterface(nameof(IProp));
				if(iprop != null) {
					//Console.WriteLine($" {field.Name}");

					var propValue = field.GetValue(this);
					var linkMethod = field.FieldType.GetMethod(nameof(IProp.Link));
					
					linkMethod.Invoke(propValue, new object[] { this });
				}
			}
			//Console.WriteLine($"#: <-");
		}
	}
}
