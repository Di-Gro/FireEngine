using YamlDotNet.Serialization;
using YamlDotNet.Serialization.NamingConventions;
using YamlDotNet.RepresentationModel;
using System.Numerics;
using System.Collections;
using System.Collections.Generic;
using System.Reflection;
using System.Globalization;
using System.Diagnostics;
using System;
using FireYaml;

namespace Engine {

    [GUID("eec5f7b8-5d7b-4d78-b1a8-d69f21459e73", typeof(Prefab))]
    public sealed class Prefab : FireYaml.IAsset {

        /// FireYaml.IAsset
        [Open] public string assetId { get; private set; } = "0000000000";
        public int assetIdHash { get; private set; } = 0;
        [Close] public CppRef cppRef { get; private set; } = CppRef.NullRef;
        /// <-

        public Prefab() { }

        public Prefab(string assetId) { 
            this.assetId = assetId; 
            assetIdHash = assetId.GetHashCode(); 
        }

        public Actor Instanciate() {
            return new FireYaml.FireReader(assetId).Instanciate<Actor>();
        }

        public void LoadAsset() {
            assetIdHash = assetId.GetHashCode();
        }

        public void ReloadAsset() { }

        public void SaveAsset() { }


        public static int CreatePrefab(CsRef actorRef, ulong pathPtr) {
            var path = Assets.ReadCString(pathPtr);

            try {
                object actorObj = CppLinked.GetObjectByRef(actorRef);
                var actor = actorObj as Actor;
                var lastPrefabId = actor.prefabId;
                actor.prefabId = "";

                var writer = new FireWriter(ignoreExistingIds: true, writeNewIds: false, startId: 1);
                var assetGuidHash = FireYaml.AssetStore.Instance.WriteAsset(path, actorObj, writer: writer);

                actor.prefabId = lastPrefabId;
                return assetGuidHash;

            } catch (Exception e) {

                Console.WriteLine("Exception on SaveScene:");

                if (e.InnerException != null) {
                    Console.WriteLine(e.InnerException.Message);
                    Console.WriteLine(e.InnerException.StackTrace);
                }
                Console.WriteLine(e.Message);
                Console.WriteLine(e.StackTrace);
                return 0;
            }
        }

        public static bool LoadPrefab(int assetGuidHash, CsRef actorRef) {
            try {
                var assetGuid = AssetStore.Instance.GetAssetGuid(assetGuidHash);
                object actorObj = CppLinked.GetObjectByRef(actorRef);

                new FireYaml.FireReader(assetGuid, writeIDs: false).InstanciateTo(ref actorObj);

                var actor = actorObj as Actor;
                actor.prefabId = assetGuid;
                return true;

            } catch (Exception e) {

                Console.WriteLine("Exception on SaveScene:");

                if (e.InnerException != null) {
                    Console.WriteLine(e.InnerException.Message);
                    Console.WriteLine(e.InnerException.StackTrace);
                }
                Console.WriteLine(e.Message);
                Console.WriteLine(e.StackTrace);

                return false;
            }
        }       

        public static bool UpdatePrefab(CsRef actorRef, int assetGuidHash) {
            try {
                object actor = CppLinked.GetObjectByRef(actorRef);
                var assetGuid = AssetStore.Instance.GetAssetGuid(assetGuidHash);

                var writer = new FireWriter(ignoreExistingIds: true, writeNewIds: false, startId: 1);
                FireYaml.AssetStore.Instance.UpdateAsset(assetGuid, actor, writer: writer);

                return true;

            } catch (Exception e) {

                Console.WriteLine("Exception on SaveScene:");

                if (e.InnerException != null) {
                    Console.WriteLine(e.InnerException.Message);
                    Console.WriteLine(e.InnerException.StackTrace);
                }
                Console.WriteLine(e.Message);
                Console.WriteLine(e.StackTrace);
                return false;
            }
        }

    }
}