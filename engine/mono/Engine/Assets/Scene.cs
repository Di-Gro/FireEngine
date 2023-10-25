using System;
using System.IO;
using System.Collections.Generic;
using System.Diagnostics;

using EngineDll;
using FireYaml;

namespace Engine {

    public class SceneData : AssetDataBase {
        public string Name {
            get { return cppRef.value == 0 ? "" : Dll.Scene.name_get(cppRef); }
            set { if (cppRef.value != 0) Dll.Scene.name_set(cppRef, value); }
        }

        public Vector3 CameraPos {
            get { return cppRef.value == 0 ? default : Dll.Scene.editorCameraPos_get(cppRef); }
            set { if (cppRef.value != 0) Dll.Scene.editorCameraPos_set(cppRef, value); }
        }

        public Quaternion CameraRot {
            get { return cppRef.value == 0 ? default : Dll.Scene.editorCameraRot_get(cppRef); }
            set { if (cppRef.value != 0) Dll.Scene.editorCameraRot_set(cppRef, value); }
        }

        public float CameraSpeed {
            get { return cppRef.value == 0 ? default : Dll.Scene.editorCameraSpeed_get(cppRef); }
            set { if (cppRef.value != 0) Dll.Scene.editorCameraSpeed_set(cppRef, value); }
        }
    }

    [GUID("c24339ac-ed91-44f9-bab5-600fe45a9806", typeof(Scene))]
    public class Scene : AssetBase<Scene, SceneData>, IAsset {

        public string Name { get => m_data.Name; set => m_data.Name = value; }
        public Vector3 CameraPos { get => m_data.CameraPos; set => m_data.CameraPos = value; }
        public Quaternion CameraRot { get => m_data.CameraRot; set => m_data.CameraRot = value; }
        public float CameraSpeed { get => m_data.CameraSpeed; set => m_data.CameraSpeed = value; }

        [Close] public bool HasInstance => cppRef.value > 0;

        public Scene() { }

        public Scene(CppRef sceneRef) {
            cppRef = sceneRef;
            assetId = Dll.Scene.assetId_get(cppRef);
        }

        public Scene(string assetId) : this() {
            this.assetId = assetId;
        }

        public override void Init(string assetId, CppRef cppRef) {
            this.assetId = assetId;
            this.cppRef = cppRef;
        }

        public override void LoadAsset() {
            LogLoad();

            if (!HasInstance)
                return;

            ReloadAsset();
        }

        public override void ReloadAsset() {
            LogReload();

            if (!HasInstance)
                return;

            cppRef = Dll.Assets.Get(Game.gameRef, assetIdHash);
            if (cppRef.value == 0)
                throw new Exception("Asset not loaded");

            var stopwatch = new Stopwatch();

            stopwatch.Start();
            AssetStore.GetAssetDeserializer(assetIdHash).InstanciateToWithoutLoad(this);
            stopwatch.Stop();

            TimeSpan elapsedTime = stopwatch.Elapsed;
            Console.WriteLine($"Time: {elapsedTime.TotalMilliseconds} ms");

            var assetPath = AssetStore.GetAssetPath(assetIdHash);
            Name = Path.GetFileNameWithoutExtension(assetPath);
        }

        public static string ToSceneAssetId(CppRef sceneRef, string assetId) {
            return $"scene_{sceneRef}_{assetId}";
        }

        public static int cpp_CreateSceneAsset(ulong cpath) {
            var path = Assets.ReadCString(cpath);

            var assetIdHash = AssetStore.CreateNewAsset(typeof(Scene), path);

            return assetIdHash;
        }

        public static bool cpp_RenameSceneAsset(int assetIdHash, ulong cpath) {
            return AssetStore.cpp_RenameAsset(assetIdHash, cpath);
        }
    }

    public class SceneSerializer : SerializerBase {

        public override void OnSerialize(FireYaml.FireWriter serializer, string selfPath, Type type, object instance) {
            base.OnSerialize(serializer, selfPath, type, instance);

            var scene = instance as Engine.Scene;
            if (scene == null || !scene.HasInstance)
                return;

            Game.PushScene(scene);
            var roots = Game.GetRootActors(scene.cppRef);
            Game.PopScene();

            serializer.AddField($"{selfPath}.m_roots", roots.GetType(), roots);

            foreach(var rootActor in roots)
                serializer.CreateDocument(rootActor.GetType(), rootActor);
        }

        private static readonly List<string> s_extraFields = new List<string>() {
            "m_roots",
        };

        public override List<string> GetNamesOfExtraFields() => s_extraFields;

        public override List<FireBin.Pointer?> WriteExtraFields(FireBin.Serializer writer, Type type, object instance) {
            var res = new List<FireBin.Pointer?>();

            var scene = instance as Scene;
            if (scene.cppRef.value == 0)
                throw new Exception("scene.cppRef.value == 0");

            Game.PushScene(scene);
            var roots = Game.GetRootActors(scene.cppRef);
            Game.PopScene();

            res.Add(writer.AddAsList(roots.GetType(), roots));

            foreach (var rootActor in roots)
                writer.AddAsNamedList(rootActor.GetType(), rootActor);

            return res;
        }
        
        public override void ReadExtraFields(FireBin.Deserializer des, object instance, FireBin.PtrList list) {
            var scene = instance as Scene;

            var rootsPtr = list[0].Value;
            var roots = des.Reader.ReadList(rootsPtr);

            Game.PushScene(scene);
            for (int i = 0; i < roots.Count; i++) {
                var actorRef = des.Reader.ReadReference(roots[i].Value);
                des.LoadAsNamedList(typeof(Actor), actorRef.to);
            }
            Game.PopScene();
        }

        public override void OnDeserialize(FireReader deserializer, string selfPath, Type type, ref object instance) {
            base.OnDeserialize(deserializer, selfPath, type, ref instance);

            var scene = instance as Engine.Scene;
            if (scene == null || !scene.HasInstance)
                return;

            Game.PushScene(scene);
            m_LoadActors(deserializer, $"{selfPath}.m_roots");
            Game.PopScene();
        }

        private void m_LoadActors(FireYaml.FireReader deserializer, string rootsPath) {
            var roots = deserializer.GetField(rootsPath);

            var count = roots.GetItemsCount(rootsPath);
            for (int i = 0; i < count; i++) {
               var yamlChild = roots.GetValue($"{rootsPath}.{i}");
               var fullPath = $".{yamlChild.value}";

               if (!deserializer.HasFile(fullPath))
                   throw new Exception("Missing actor");

               if (yamlChild.type == YamlValue.Type.Null)
                   throw new Exception("Actor in scene can not be Null");

               var rootActor = deserializer.LoadDocument(fullPath) as Engine.Actor;
            }
        }
    }

}