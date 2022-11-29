using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

using Engine;


namespace Engine {
    class OhMyMesh : CSComponent {

        private MeshComponent m_meshComp;
        private DynamicMaterial m_material;


        private string[] m_meshNames = new string[] {
            "../../data/assets/levels/farm/meshes/House_Red.obj",
            "../../data/assets/levels/farm/meshes/House_Purple.obj",
            "../../data/assets/levels/farm/meshes/House_Blue.obj",
			//"runtime:/form/Box",
			//"runtime:/form/BoxLined",
			//"runtime:/form/Sphere",
			//"runtime:/form/SphereLined",
		};

        private int m_index = 0;

        public override void OnInit() {
			//transform.localScale = new Vector3(100, 100, 100);

			m_meshComp = gameObject.AddComponent<MeshComponent>();
			m_meshComp.mesh = new StaticMesh(m_meshNames[m_index]);

            m_material = new DynamicMaterial(m_meshComp.GetMaterial(0));

            m_meshComp.SetMaterial(0, m_material);
        }

        public override void OnUpdate() {

            if (Input.GetButtonDown(Key.Enter)) {
                m_meshComp.mesh = new StaticMesh(m_meshNames[m_index]);
                m_index = (m_index + 1) % m_meshNames.Length;

                m_meshComp.SetMaterial(0, m_material);

                var pos = transform.localPosition;
                pos.x += 150;
                transform.localPosition = pos;
            }

			if (Input.WheelDelta != 0 && Input.GetButton(Key.LeftShift)) {
				var color = m_material.DiffuseColor;
				color.x += 0.05f * Input.WheelDelta;
				color.y += 0.05f * Input.WheelDelta;
				color.z += 0.05f * Input.WheelDelta;
				m_material.DiffuseColor = color;

				Console.WriteLine($"# OhMyMesh: color={m_material.DiffuseColor}");
			}

        }
    }
}

namespace EngineMono {

	class CSClass : CppLinked {

		public Transform transform { get; private set; }

		/// напрямую в C# через указатели
		public float floatValue { 
			get => prop_floatValue.value; 
			set => prop_floatValue.value = value; 
		}

		public int intValue { 
			get => prop_intValue.value; 
			set => prop_intValue.value = value; 
		}

		public Vector3 vector3 {
			get => prop_vector3.value;
			set => prop_vector3.value = value;
        }

		public Quaternion quaternion {
			get => prop_quat.value;
			set => prop_quat.value = value;
		}

		private Prop<float> prop_floatValue = new Prop<float>(0);
		private Prop<int> prop_intValue = new Prop<int>(1);

		private Prop<Vector3> prop_vector3 = new Prop<Vector3>(2);
		private Prop<Quaternion> prop_quat = new Prop<Quaternion>(3);

		/// через геттеры и сеттеры в C++
		public long longValue { 
			get => m_longValue_get(cppRef); 
			set => m_longValue_set(cppRef, value); 
		}

		public Vector3 vector3m {
			get => m_vector3m_get(cppRef);
			set => m_vector3m_set(cppRef, value);
		}

		/// функция из C++
		public float SomeFunc() => m_SomeFunc(cppRef);

		/// Связывание с C++ объектом
		public override CsRef Link(CppRef classInfoRef, CppRef objRef) {
			CsRef csRef = base.Link(classInfoRef, objRef);

			var gameObject = new GameObject().AddComponent<OhMyMesh>();


            //Console.WriteLine($"# {gameObject.transform.localScale}");

            //var pos = gameObject.transform.localScale;
			//pos.SetX(0);


			//gameObject.transform.localScale = new Vector3(1,2,3);

            //Console.WriteLine($"# {gameObject.transform.localScale}");

            //var mpos = Input.MousePosition;

            //Console.WriteLine($"# mpos: {mpos}");

            return csRef;
		}

        /// Связывание с DLL

        [DllImport(MonoClass.ExePath, EntryPoint = "CppClass_longValue_get")]
		private static extern long m_longValue_get(CppRef objRef);

		[DllImport(MonoClass.ExePath, EntryPoint = "CppClass_longValue_set")]
		private static extern void m_longValue_set(CppRef objRef, long value);


		[DllImport(MonoClass.ExePath, EntryPoint = "CppClass_vector3m_get")]
		private static extern Vector3 m_vector3m_get(CppRef objRef);

		[DllImport(MonoClass.ExePath, EntryPoint = "CppClass_vector3m_set")]
		private static extern void m_vector3m_set(CppRef objRef, Vector3 value);


		[DllImport(MonoClass.ExePath, EntryPoint = "CppClass_SomeFunc")]
		private static extern float m_SomeFunc(CppRef objRef);

	};
}
