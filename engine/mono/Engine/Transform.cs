using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;


namespace Engine {
    sealed class Transform : CppLinked {

		public Vector3 localPosition {
			get => dll_localPosition_get(cppRef);
			set => dll_localPosition_set(cppRef, value);
		}

		public Vector3 localRotation {
			get => dll_localRotation_get(cppRef);
			set => dll_localRotation_set(cppRef, value);
		}

		public Quaternion localRotationQ {
			get => dll_localRotationQ_get(cppRef);
			set => dll_localRotationQ_set(cppRef, value);
		}

		public Vector3 localScale {
			get => dll_localScale_get(cppRef);
			set => dll_localScale_set(cppRef, value);
		}


		public Vector3 localForward => dll_localForward_get(cppRef);
		public Vector3 localUp => dll_localUp_get(cppRef);
		public Vector3 localRight => dll_localRight_get(cppRef);

		public Vector3 forward => dll_forward_get(cppRef);
		public Vector3 up => dll_up_get(cppRef);
		public Vector3 right => dll_right_get(cppRef);


		private Transform() { }

		private static CsRef cpp_Create(CppRef classRef, CppRef objRef) {
			//Console.WriteLine($"#: Transform.cpp_Create(class={classRef}, obj={objRef})");

			var transform = new Transform();
			transform.Link(classRef, objRef);

			return transform.csRef;
		}

		private static void cpp_Remove(CsRef objRef) {
			RemoveObjectByRef(objRef);
		}


		[DllImport(EngineMono.MonoClass.ExePath, EntryPoint = "Transform_localPosition_get")]
		private static extern Vector3 dll_localPosition_get(CppRef objRef);

		[DllImport(EngineMono.MonoClass.ExePath, EntryPoint = "Transform_localPosition_set")]
		private static extern void dll_localPosition_set(CppRef objRef, Vector3 value);


		[DllImport(EngineMono.MonoClass.ExePath, EntryPoint = "Transform_localRotation_get")]
		private static extern Vector3 dll_localRotation_get(CppRef objRef);

		[DllImport(EngineMono.MonoClass.ExePath, EntryPoint = "Transform_localRotation_set")]
		private static extern void dll_localRotation_set(CppRef objRef, Vector3 value);


		[DllImport(EngineMono.MonoClass.ExePath, EntryPoint = "Transform_localRotationQ_get")]
		private static extern Quaternion dll_localRotationQ_get(CppRef objRef);

		[DllImport(EngineMono.MonoClass.ExePath, EntryPoint = "Transform_localRotationQ_set")]
		private static extern void dll_localRotationQ_set(CppRef objRef, Quaternion value);


		[DllImport(EngineMono.MonoClass.ExePath, EntryPoint = "Transform_localScale_get")]
		private static extern Vector3 dll_localScale_get(CppRef objRef);

		[DllImport(EngineMono.MonoClass.ExePath, EntryPoint = "Transform_localScale_set")]
		private static extern void dll_localScale_set(CppRef objRef, Vector3 value);


		[DllImport(EngineMono.MonoClass.ExePath, EntryPoint = "Transform_localForward_get")]
		private static extern Vector3 dll_localForward_get(CppRef objRef);

		[DllImport(EngineMono.MonoClass.ExePath, EntryPoint = "Transform_localUp_get")]
		private static extern Vector3 dll_localUp_get(CppRef objRef);

		[DllImport(EngineMono.MonoClass.ExePath, EntryPoint = "Transform_localRight_get")]
		private static extern Vector3 dll_localRight_get(CppRef objRef);


		[DllImport(EngineMono.MonoClass.ExePath, EntryPoint = "Transform_forward_get")]
		private static extern Vector3 dll_forward_get(CppRef objRef);

		[DllImport(EngineMono.MonoClass.ExePath, EntryPoint = "Transform_up_get")]
		private static extern Vector3 dll_up_get(CppRef objRef);

		[DllImport(EngineMono.MonoClass.ExePath, EntryPoint = "Transform_right_get")]
		private static extern Vector3 dll_right_get(CppRef objRef);

	}
}
