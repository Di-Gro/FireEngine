using System;
using System.IO;

using EngineDll;
using FireYaml;

namespace Engine {

    [GUID("c24339ac-ed91-44f9-bab5-600fe45a9806", typeof(Scene))]
    public class Scene : FireYaml.IFile, FireYaml.IAsset {

        /// FireYaml.IAsset
        [Open] public string assetId { get; private set; } = "0000000000";
        [Close] public int assetIdHash { get; private set; }
        [Close] public CppRef cppRef { get; private set; } = CppRef.NullRef;
        /// <-
        /// FireYaml.IFile ->
        [Close] public ulong assetInstance { get; set; } = 0;
        [Close] public int fileId { get; set; } = -1;
        [Close] public string prefabId { get; set; } = FireYaml.IFile.NotPrefab;
        /// <- 
        
        [Close] public bool HasInstance => cppRef.value > 0;

        public string Name { 
            get => Dll.Scene.name_get(cppRef); 
            set => Dll.Scene.name_set(cppRef, value); 
        }

        public Vector3 CameraPos {
            get => Dll.Scene.editorCameraPos_get(cppRef);
            set => Dll.Scene.editorCameraPos_set(cppRef, value);
        }

        public Quaternion CameraRot {
            get => Dll.Scene.editorCameraRot_get(cppRef);
            set => Dll.Scene.editorCameraRot_set(cppRef, value);
        }

        private Scene() {
            assetInstance = FireYaml.AssetInstance.PopId();
        }

        public Scene(CppRef sceneRef) {
            cppRef = sceneRef;
            assetInstance = cppRef.value;
            assetId = Assets.ReadCString(Dll.Scene.assetId_get(cppRef));
            assetIdHash = Dll.Scene.assetIdHash_get(cppRef);
        }

        public Scene(string assetId) : this() {
            this.assetId = assetId;
            assetIdHash = assetId.GetHashCode();
        }

        public void LoadAsset() {
            Console.WriteLine("Scene.LoadAsset()");
            assetIdHash = assetId.GetHashCode();

            if (!HasInstance)
                return;

            ReloadAsset();
        }

        public void ReloadAsset() {
            Console.WriteLine($"Scene.ReloadAsset({assetId}: {assetIdHash})");

            if (!HasInstance)
                return;

            assetInstance = cppRef.value;

            new FireYaml.FireReader(assetId).InstanciateIAssetAsFile(this);

            var assetPath = FireYaml.AssetStore.Instance.GetAssetPath(assetId);
            Name = Path.GetFileNameWithoutExtension(assetPath);
        }

        public void SaveAsset() {
            cppRef = Dll.Assets.Get(Game.gameRef, assetIdHash);
            if (cppRef.value == 0)
                throw new Exception("Asset not loaded");

            fileId = 1;
            assetInstance = cppRef.value;
            
            AssetStore.Instance.UpdateAsset(assetId, this);
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

        public override void OnDeserialize(FireYaml.FireReader deserializer, string selfPath, Type type, ref object instance) {
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