// #define EDITOR

using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;
using System.IO;

using Engine;
using FireYaml;


namespace Engine {

    [GUID("10ea1ff8-d1c6-41d3-b8ac-b3a56ed9db82", typeof(TestPrefab))]
    class TestPrefab : CSComponent {
        public float floatField = 5.555f;

        public Texture texture;
        public Image image;

        public override void OnStart() {
            Console.WriteLine("#: TestPrefab.OnStart()");
        }
    }

    [GUID("4f1285c1-8ccc-4a8b-b77b-848af7f55e42", typeof(TestMeshBase))]
    class TestMeshBase : CSComponent {
        public float floatField = 48.56f;
    }

    [GUID("a6b0e3a1-0a74-4901-be26-f38ce7ec7b55", typeof(TestMesh))]
    class TestMesh : TestMeshBase {

        // public Component component;

        // public Actor actorRef;

        // public Prefab userPrefab;
        // public Scene sameScene;

        [Open] private MeshComponent m_meshComp;
        [Open] private DynamicMaterial m_material;

        // public StaticMaterial nullStaticMaterial;
        // public StaticMaterial staticMaterial;

        // public List<Vector3> vecList = null;


        // [Range(0f, 10f)] public float floatField = 4.56f; // Overlap is for test
        // [Close] public float floatField2 = 5.56f;

        // [Range(1, 100)] 
        // public int intField3 = 68;

        // private float m_floatField3 = 6.56f;

        // [Open] 
        // private float m_floatField4 = 7.56f;

        // [Range(0f, 10f)] 
        // public Vector3 vector = new Vector3(4,3,2);

        [Open] private List<Mesh> m_meshes = new List<Mesh>();

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