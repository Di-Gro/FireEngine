using System;
using System.Collections.Generic;
using System.Reflection;

using EngineDll;
using FireYaml;

namespace Engine {

    [Serializable]
    public class Scene : FireYaml.IFile {

        /// FireYaml.IFile ->
        [Close] public ulong assetInstance { get; set; } = FireYaml.AssetInstance.PopId();

        [Close] public int fileId { get; set; } = -1;

        [Close] public string prefabId { get; set; } = FireYaml.IFile.NotPrefab;
        /// <- 

        public CppRef cppRef { get; private set; } = 0;

        public string Name = "";

        private Scene() { }

        public Scene(CppRef sceneRef) {
            this.cppRef = sceneRef;
        }

    }

    public class SceneSerializer : SerializerBase {

        public override void OnSerialize(FireYaml.Serializer serializer, string selfPath, Type type, object instance) {
            base.OnSerialize(serializer, selfPath, type, instance);

            var scene = instance as Engine.Scene;
            if (scene == null)
                return;

            Game.PushScene(scene);
            var roots = Game.GetRootActors(scene.cppRef);
            Game.PopScene();

            serializer.AddField($"{selfPath}.m_roots", roots.GetType(), roots);

            foreach(var rootActor in roots)
                serializer.CreateDocument(rootActor.GetType(), rootActor);
        }

        public override void OnDeserialize(FireYaml.Deserializer deserializer, string selfPath, Type type, ref object instance) {
            base.OnDeserialize(deserializer, selfPath, type, ref instance);

            var scene = instance as Engine.Scene;
            if (scene == null)
                return;

            Game.PushScene(scene);
            m_LoadActors(deserializer, $"{selfPath}.m_roots");
            Game.PopScene();
        }

        private void m_LoadActors(FireYaml.Deserializer deserializer, string rootsPath) {
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