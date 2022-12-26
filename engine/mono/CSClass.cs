// #define EDITOR

using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;
using System.IO;

using Engine;


namespace Engine {

    class TestPrefab : CSComponent {
        public float floatField = 5.555f;

        public Texture texture;
        public Image image;

        public override void OnStart() {
            Console.WriteLine("#: TestPrefab.OnStart()");
        }
    }

    class TestMeshBase : CSComponent {
        public float floatField = 48.56f;
    } 

    class TestMesh : TestMeshBase {

        public Component component;

        public Actor actorRef;

        public Prefab userPrefab = new Prefab("TestPrefab");

        [Open] private MeshComponent m_meshComp;
        [Open] private DynamicMaterial m_material;

        public StaticMaterial nullStaticMaterial;
        public StaticMaterial staticMaterial;

        public List<Vector3> vecList = null;


        public float floatField = 4.56f;
        [Close] public float floatField2 = 5.56f;

        private float m_floatField3 = 6.56f;
        [Open] private float m_floatField4 = 7.56f;

        public Vector3 vector = new Vector3(4,3,2);


  //      private string[] m_meshNames = new string[] {
  //          "../../data/assets/levels/farm/meshes/House_Red.obj",
  //          "../../data/assets/levels/farm/meshes/House_Purple.obj",
  //          "../../data/assets/levels/farm/meshes/House_Blue.obj",
		//};
        [Open] private List<StaticMesh> m_meshes = new List<StaticMesh>();

        private int m_index = 0;


        public override void OnInit() {
            // TODO: Убрать
            // m_meshComp = actor.AddComponent<MeshComponent>();
            // m_meshComp.mesh = new StaticMesh(m_meshNames[m_index]);
            // m_material = new DynamicMaterial(m_meshComp.GetMaterial(0));

            // m_meshComp.SetMaterial(0, m_material);

            // staticMaterial = m_meshComp.GetMaterial(0);

            // component = m_meshComp;
            // actorRef = actor;

            // vecList = new List<Vector3>();
            // vecList.Add(new Vector3(1,2,3));
            // vecList.Add(new Vector3(4,5,6));

            if (m_meshComp == null)
                throw new Exception($"#: {GetType().Name}: {nameof(m_meshComp)} == null");

            //var testPrefab = userPrefab.Instanciate().GetComponent<TestPrefab>();
            //testPrefab.floatField = 6554;
        }

        public override void OnStart() {
            //m_meshComp.mesh = new StaticMesh(m_meshNames[m_index]); // TODO: Убрать
            m_meshComp.mesh = m_meshes[m_index];
            m_material = new DynamicMaterial(m_meshComp.GetMaterial(0));
            
            m_meshComp.SetMaterial(0, m_material);
        }

        public override void OnDestroy() {
            if(!m_meshComp.IsDestroyed)
                m_meshComp.RemoveMaterial(0);
                
            m_material.Delete();
        }

        public override void OnUpdate() {

			actor.localRotationQ = actor.localRotationQ * Quaternion.CreateXRotation(Game.DeltaTime * 1f);

			if (Input.GetButtonDown(Key.Enter)) {
                // TODO: Убрать
                //m_meshComp.mesh = new StaticMesh(m_meshNames[m_index]); 
                //m_index = (m_index + 1) % m_meshNames.Length; // TODO: Убрать
                m_meshComp.mesh = m_meshes[m_index];
                m_index = (m_index + 1) % m_meshes.Count;
                
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

            // var actor = new Actor();
            // actor.AddComponent<TestMesh>();
            // actor.AddComponent<UI.TestImGui>();
            // assetStore.CreateAsset("TestMesh.yml", actor, "TestMesh");
            // var actor = new Prefab("TestMesh").Instanciate();

            // var actor = new Actor();
            // actor.AddComponent<TestPrefab>();
            // assetStore.CreateAsset("TestPrefab.yml", actor, "TestPrefab");
            //var actor2 = new Prefab("TestPrefab").Instanciate();

            // var texture = new Texture();
            // texture.width = 128;
            // texture.height = 128;
            // assetStore.CreateAsset("TestTexture1.yml", texture, "TestTexture1");
            var texture = new FireYaml.Deserializer("TestTexture1").Instanciate();

            // var image = new Image();
            // image.ext = "png";
            // assetStore.CreateAsset("TestImage1.yml", image, "TestImage1");
            // var image = new FireYaml.Deserializer("TestImage1").Instanciate();


            // var meshComp = new Actor().AddComponent<MeshComponent>();
            // meshComp.mesh = new StaticMesh("../../data/assets/levels/farm/meshes/House_Red.obj");
            // var material = meshComp.GetMaterial(0);
            // assetStore.CreateAsset("TestMaterial1.yml", material, "TestMaterial1");
            // var material = new FireYaml.Deserializer("TestMaterial1").Instanciate();

            var tex = texture as Texture;
            EngineDll.Dll.Assets.Reload(Game.gameRef, tex.assetIdHash);

            
            return csRef;
		}

	};
}
