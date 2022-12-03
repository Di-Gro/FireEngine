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

			m_meshComp = actor.AddComponent<MeshComponent>();
            m_meshComp.mesh = new StaticMesh(m_meshNames[m_index]);
            m_material = new DynamicMaterial(m_meshComp.GetMaterial(0));

            m_meshComp.SetMaterial(0, m_material);
        }

        public override void OnUpdate() {

			actor.localRotationQ = actor.localRotationQ * Quaternion.CreateXRotation(Game.DeltaTime * 1f);

			//Console.WriteLine($"# DeltaTime: color={m_material.DiffuseColor}");

			if (Input.GetButtonDown(Key.Enter)) {
                m_meshComp.mesh = new StaticMesh(m_meshNames[m_index]);
                m_index = (m_index + 1) % m_meshNames.Length;

                m_meshComp.SetMaterial(0, m_material);

                var pos = actor.localPosition;
				pos.X -= 100;
                actor.localPosition = pos;
            }

            if (Input.WheelDelta != 0 && Input.GetButton(Key.M)) {
                m_material.DiffuseColor = m_material.DiffuseColor + Vector3.One * 0.05f * Input.WheelDelta;

                Console.WriteLine($"# OhMyMesh: color={m_material.DiffuseColor}");
            }

            //if (Input.GetButtonDown(Key.Tab)) {
            //    var actors = Game.GetRootActors();

            //    foreach(var actor in actors) {
            //        Console.WriteLine($"# root-> {actor.Name}");
            //    }
            //}

        }

    }
}

namespace EngineMono {

	class CSClass : CppLinked {

		/// Связывание с C++ объектом
		public override CsRef Link(CppRef classInfoRef, CppRef objRef) {
			CsRef csRef = base.Link(classInfoRef, objRef);

			var gameObject = new Actor();
			gameObject.AddComponent<OhMyMesh>();
            gameObject.AddComponent<UI.TestImGui>();

            /// Передача vector2, vector3 и quaternion ->
            //var gameObject2 = new GameObject();

            //Console.WriteLine($"# pos:{gameObject2.transform.localScale}");

            //var pos = gameObject2.transform.localScale;
            ////pos = pos.SetX(48);
            //pos.X = 48;
            //gameObject2.transform.localScale = pos;

            //Console.WriteLine($"# pos:{gameObject2.transform.localScale}");

            //Console.WriteLine($"# quat:{gameObject2.transform.localRotationQ}");

            //var quat = gameObject2.transform.localRotationQ;
            ////quat = quat.SetX(48);
            //quat.X = 48;
            //gameObject2.transform.localRotationQ = quat;

            //Console.WriteLine($"# quat:{gameObject2.transform.localRotationQ}");

            //var mpos = Input.MousePosition;
            //Console.WriteLine($"# mpos:{mpos}");
            /// <-


            return csRef;
		}

	};
}
