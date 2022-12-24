using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;
using System.IO;

using Engine;


namespace Engine {
    class TestMesh : CSComponent {

        private MeshComponent m_meshComp;
        private DynamicMaterial m_material;

        public float floatField = 4.56f;
        [Close] public float floatField2 = 5.56f;

        private float m_floatField3 = 6.56f;
        [Open] private float m_floatField4 = 7.56f;

        public Vector3 vector = new Vector3(4,3,2);


        private string[] m_meshNames = new string[] {
            "../../data/assets/levels/farm/meshes/House_Red.obj",
            "../../data/assets/levels/farm/meshes/House_Purple.obj",
            "../../data/assets/levels/farm/meshes/House_Blue.obj",
		};

        private int m_index = 0;

        public override void OnInit() {
			m_meshComp = actor.AddComponent<MeshComponent>();
            m_meshComp.mesh = new StaticMesh(m_meshNames[m_index]);
            m_material = new DynamicMaterial(m_meshComp.GetMaterial(0));
            
            m_meshComp.SetMaterial(0, m_material);
        }

        public override void OnDestroy() {
            m_meshComp.RemoveMaterial(0);
            m_material.Delete();
        }

        public override void OnUpdate() {

			actor.localRotationQ = actor.localRotationQ * Quaternion.CreateXRotation(Game.DeltaTime * 1f);

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

        }

    }
}

namespace EngineMono {

	class CSClass : CppLinked {
        static string pathOut = @"C:\Users\Dmitry\Desktop\Пример\out.yml";

        /// Связывание с C++ объектом
        public override CsRef Link(CppRef classInfoRef, CppRef objRef) {
			CsRef csRef = base.Link(classInfoRef, objRef);

            var assetStore = new FireYaml.AssetStore();
            FireYaml.AssetStore.Instance = assetStore;

            var gameObject = new Actor();
			gameObject.AddComponent<TestMesh>();
            gameObject.AddComponent<UI.TestImGui>();

            //var actor = new Actor();
            //var serializer = new FireYaml.Serializer(ignoreExistingIds: false, writeNewIds: true, startId: 1);
            //serializer.Serialize(gameObject);

            //File.WriteAllText(pathOut, serializer.Values.ToSortedText());

            return csRef;
		}

	};
}
