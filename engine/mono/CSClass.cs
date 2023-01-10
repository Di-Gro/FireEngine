// #define EDITOR

using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;
using System.IO;

using Engine;
using FireYaml;


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
        public Scene sameScene = new Scene("scene_1");

        [Open] private MeshComponent m_meshComp;
        [Open] private DynamicMaterial m_material;

        public StaticMaterial nullStaticMaterial;
        public StaticMaterial staticMaterial;

        public List<Vector3> vecList = null;


        [Range(0f, 10f)] public float floatField = 4.56f;
        [Close] public float floatField2 = 5.56f;

        [Range(1, 100)] 
        public int intField3 = 68;

        private float m_floatField3 = 6.56f;

        [Open] 
        private float m_floatField4 = 7.56f;

        [Range(0f, 10f)] 
        public Vector3 vector = new Vector3(4,3,2);

        [Open] private List<StaticMesh> m_meshes = new List<StaticMesh>();

        private int m_index = 0;


        public override void OnInit() {
            if (m_meshComp == null)
                throw new Exception($"#: {GetType().Name}: {nameof(m_meshComp)} == null");
        }

        public override void OnDestroy() {
            if (!m_meshComp.IsDestroyed)
                m_meshComp.RemoveMaterial(0);

            m_material.Delete();
        } 

        public override void OnStart() {
            m_material = new DynamicMaterial(m_meshComp.GetMaterial(0));
            m_meshComp.SetMaterial(0, m_material);
        }

        public override void OnUpdate() {

            actor.localRotationQ = actor.localRotationQ * Quaternion.CreateXRotation(Game.DeltaTime * 45);

            if (Input.GetButtonDown(Key.Enter)) {
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

            var assetStore = AssetStore.Instance;

            // var actor = new Actor();
            // actor.AddComponent<TestMesh>();
            // actor.AddComponent<UI.TestImGui>();
            // assetStore.CreateAsset("TestMesh.yml", actor, "TestMesh");
            var actor = new Prefab("TestMeshPrefab").Instanciate();
            // var textMesh = actor.GetComponent<TestMesh>();
            // var meshComp = actor.GetComponent<MeshComponent>();

            // var actor = new Actor();
            // actor.AddComponent<TestPrefab>();
            // assetStore.CreateAsset("TestPrefab.yml", actor, "TestPrefab");
            //var actor2 = new Prefab("TestPrefab").Instanciate();

            // var texture = new Texture();
            // texture.width = 128;
            // texture.height = 128;
            // assetStore.CreateAsset("TestTexture1.yml", texture, "TestTexture1");
            // var texture = new FireYaml.Deserializer("TestTexture1").Instanciate();

            // var image = new Image();
            // image.ext = "png";
            // assetStore.CreateAsset("TestImage1.yml", image, "TestImage1");
            // var image = new FireYaml.Deserializer("TestImage1").Instanciate();

            // var meshComp = new Actor().AddComponent<MeshComponent>();
            // meshComp.mesh = new StaticMesh("../../data/assets/levels/farm/meshes/House_Red.obj");
            // var material = meshComp.GetMaterial(0);
            // assetStore.CreateAsset("TestMaterial1.yml", material, "TestMaterial1");
            // var material = new FireYaml.Deserializer("TestMaterial1").Instanciate();

            // var meshComp = new Actor().AddComponent<MeshComponent>();
            // meshComp.mesh = new StaticMesh("../../data/assets/levels/farm/meshes/House_Red.obj");

            // var mesh = new FireYaml.Deserializer("TestMesh1").Instanciate();

            // var actor = new Actor();
            // var meshComp = actor.AddComponent<MeshComponent>();
            // meshComp.mesh = new StaticMesh().LoadFromAsset("TestMesh1");
            // meshComp.SetMaterial(0, new StaticMaterial().LoadFromAsset(AssetStore.M_Default));
            // assetStore.CreateAsset("TestPrefab2.yml", actor, "TestPrefab2");

            // var actor = new Prefab("TestPrefab2").Instanciate();
            // var meshComp = actor.GetComponent<MeshComponent>();

            // var asset = mesh as FireYaml.IAsset;
            // EngineDll.Dll.Assets.Reload(Game.gameRef, asset.assetIdHash);

            // var dirLight = new Actor("CS DirLight").AddComponent<DirectionalLight>();
            // var dirLight = new Actor("CS AmbientLight").AddComponent<AmbientLight>();
            // var pointLight = new Actor("CS Point Light").AddComponent<PointLight>();
            // pointLight.Radius = 300;
            // var spotLight = new Actor("CS Spot Light").AddComponent<SpotLight>();


            // object scene = new Scene(0);
            // new FireYaml.Deserializer("TestMesh1").InstanciateTo(ref scene);

            // var scene = Game.CreateScene();


            return csRef;
		}

	};
}
