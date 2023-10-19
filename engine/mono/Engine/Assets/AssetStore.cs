using System;
using System.IO;
using System.Reflection;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using EngineDll;
using FireYaml;


namespace Engine {

    public class AssetStore {

        public static AssetStore Instance { get; set; }

        //private static Dictionary<int, Type> m_guidHash_type = new Dictionary<int, Type>();
        //private Dictionary<int, Asset> m_assetGuidHash_assetData = new Dictionary<int, Asset>();
        //private Dictionary<int, YamlValues> m_assetGuidHash_assetValues = new Dictionary<int, YamlValues>();

        private HashSet<int> m_runtimeAssets = new HashSet<int>();

        public string ProjectPath;
        public string AssetsPath => $"{ProjectPath}\\Assets";
        public string EngineAssetsPath => $"{ProjectPath}\\Engine\\Assets";
        public string EditorPath => $"{ProjectPath}\\Editor";

        //private uint m_nextAssetId = 1;
        //private uint m_nextTmpAssetId = 1;

        private DateTime m_lastChangeTime = DateTime.UnixEpoch;

        private FireBinAssetMenager m_fbinAM = new FireBinAssetMenager();
        private FireYmlAssetMenager m_fymlAM = new FireYmlAssetMenager();

        private IAssetMenager[] m_assetMenagers;

        private bool m_useFbin = true;

        public AssetStore() {
            if (m_useFbin)
                m_assetMenagers = new IAssetMenager[] { m_fbinAM, m_fymlAM };
            else 
                m_assetMenagers = new IAssetMenager[] { m_fymlAM };
        }

        public void Init(string projectPath) {
            ProjectPath = Path.GetFullPath(projectPath); ;

            m_fymlAM.createNewAssetsOnLoad = false;

            Dll.AssetStore.projectPath_set(Game.assetStoreRef, ProjectPath);
            Dll.AssetStore.assetsPath_set(Game.assetStoreRef, AssetsPath);
            Dll.AssetStore.editorPath_set(Game.assetStoreRef, EditorPath);

            GUIDAttribute.CollectTypes();

            foreach (var assetMenager in m_assetMenagers)
                assetMenager.Load(projectPath);

            SendTypesToCpp();
            SendAssetsInCpp();

            // m_fbinAM.ConvertFromYaml(m_fymlAM, projectPath, m_fymlAM.GetAsset("25ddaa33-ea53-4749-a368-c2143d4c6a25".GetAssetIDHash()));
            // m_fbinAM.ConvertFromYaml(m_fymlAM, projectPath);
        }

        public static IDeserializer GetAssetDeserializer(int assetIdHash, bool writeIDs = true, bool useCsRefs = false) {
            foreach (var assetMenager in Instance.m_assetMenagers) {
                var asset = assetMenager.GetAsset(assetIdHash);
                if (asset != null)
                    return assetMenager.GetDeserializer(assetIdHash, writeIDs: writeIDs, useCsRefs: useCsRefs);
            }
            throw new Exception($"GetDeserializer: Asset not exist.");
        }

        public static string GetAssetGuid(int assetIdHash) {
            foreach(var assetMenager in Instance.m_assetMenagers) {
                var asset = assetMenager.GetAsset(assetIdHash);
                if (asset != null)
                    return asset.assetID;
            }
            return null;
        }

        public static Asset GetAsset(int assetIdHash) {
            foreach (var assetMenager in Instance.m_assetMenagers) {
                var asset = assetMenager.GetAsset(assetIdHash);
                if (asset != null)
                    return asset;
            }
            return null;
        }

        public static bool HasAsset(int assetIdHash) {
            foreach (var assetMenager in Instance.m_assetMenagers) {
                var asset = assetMenager.GetAsset(assetIdHash);
                if (asset != null)
                    return true;
            }
            return false;
        }

        public static Type GetAssetType(int assetIdHash) {
            foreach (var assetMenager in Instance.m_assetMenagers) {
                var asset = assetMenager.GetAsset(assetIdHash);
                if (asset != null)
                    return asset.scriptType;
            }
            return null;
        }

        public static YamlValues ThrowYamlAssetData(int assetIdHash) {
            var asset = Instance.m_fymlAM.GetAsset(assetIdHash) as FireYamlAsset;
            if (asset == null)
                throw new Exception($"Asset with assetGuidHash:{assetIdHash} not exist");

            Instance.m_fymlAM.LoadAssetData(assetIdHash);

            return asset.data;
        }

        public static FireBin.Data ThrowFbinAssetData(int assetIdHash) {
            var asset = Instance.m_fbinAM.GetAsset(assetIdHash) as FireBinAsset;
            if (asset == null)
                throw new Exception($"Asset with assetGuidHash:{assetIdHash} not exist");

            Instance.m_fbinAM.LoadAssetData(assetIdHash);

            return asset.data;
        }

        public static bool HasAssetPath(int assetIdHash) {
            foreach (var assetMenager in Instance.m_assetMenagers) {
                var asset = assetMenager.GetAsset(assetIdHash);
                if (asset != null)
                    return asset.path != "";
            }
            return false;
        }

        public static string GetAssetPath(int assetIdHash) {
            foreach (var assetMenager in Instance.m_assetMenagers) {
                var asset = assetMenager.GetAsset(assetIdHash);
                if (asset != null)
                    return asset.path;
            }
            return "";
        }

        public static int GetAssetFilesCount(int assetIdHash) {
            var yamlAsset = Instance.m_fymlAM.GetAsset(assetIdHash) as FireYamlAsset;
            if (yamlAsset != null) {
                var values = yamlAsset.data;

                var assetIdPath = ".file0.assetId";
                var filesPath = ".file0.files";

                var hasAssetId = values.HasValue(assetIdPath);
                var hasfiles = values.HasValue(filesPath);

                if (!hasAssetId || !hasfiles)
                    throw new Exception("Asset not contains AssetInfo in file0");

                var filesCount = int.Parse(values.GetValue(filesPath).value);
                return filesCount;
            }
            return 0;
        }

        /// Временные ассеты
        // Не добавляются в загруженные ассеты
        // Не сохраняются в файл при сериализации

        //public static uint cpp_CreateTmpAssetIdInt() {
        //    var id = AssetStore.Instance.m_nextTmpAssetId++;
        //    return id;
        //}

        //public static void cpp_AddTmpAssetIdHash(int tmpAssetIdHash) {
        //    Instance.m_runtimeAssets.Add(tmpAssetIdHash);
        //}

        public static bool IsRuntimeAsset(int tmpAssetIdHash) {
            return Instance.m_runtimeAssets.Contains(tmpAssetIdHash);
        }

        public static void AddRuntimeAsset(int assetIdHash) {
            Instance.m_runtimeAssets.Add(assetIdHash);
        }

        public void SendTypesToCpp() {
            Dll.AssetStore.ClearTypes(Game.gameRef);
            Dll.AssetStore.ClearAssetTypes(Game.gameRef);

            foreach (var guidHash_type in GUIDAttribute.types) {
                var typeGuidHash = guidHash_type.Key;
                var type = guidHash_type.Value;

                Dll.AssetStore.SetType(Game.gameRef, typeGuidHash, type.FullName, type.Name);

                if (FireWriter.IsUserComponent(type))
                    Dll.AssetStore.AddComponent(Game.gameRef, typeGuidHash);

                if (FireWriter.IsAsset(type))
                    Dll.AssetStore.AddAssetType(Game.gameRef, typeGuidHash);
            }
        }

        public void SendAssetsInCpp() {
            var actorId = GUIDAttribute.GetGuidHash(typeof(Engine.Actor));
            var componentId = GUIDAttribute.GetGuidHash(typeof(Engine.Component));
            var prefabGuidHash = GUIDAttribute.GetGuidHash(typeof(Engine.Prefab));
            var sceneGuidHash = GUIDAttribute.GetGuidHash(typeof(Engine.Scene));

            Dll.AssetStore.ClearAssets(Game.gameRef);
            Dll.AssetStore.actorTypeIdHash_set(Game.assetStoreRef, actorId);
            Dll.AssetStore.prefabTypeIdHash_set(Game.assetStoreRef, prefabGuidHash);
            Dll.AssetStore.componentTypeIdHash_set(Game.assetStoreRef, componentId);
            Dll.AssetStore.sceneTypeIdHash_set(Game.assetStoreRef, sceneGuidHash);

            var used = new HashSet<int>();

            if (m_useFbin) {
                foreach (var asset in m_fbinAM.EnumerateAssets()) {
                    used.Add(asset.assetIDHash);
                    Dll.AssetStore.AddAsset(Game.gameRef, asset.scriptIDHash, asset.assetIDHash, asset.name);
                }
            }
            foreach (var asset in m_fymlAM.EnumerateAssets()) {
                if (!used.Contains(asset.assetIDHash))
                    Dll.AssetStore.AddAsset(Game.gameRef, asset.scriptIDHash, asset.assetIDHash, asset.name);
            }
        }

        public static int CreateNewAsset(Type type, string assetPath, string sourceExt = "") {
            foreach (var assetMenager in Instance.m_assetMenagers)
                return assetMenager.CreateNewAsset(type, assetPath, sourceExt);
                
            return 0;
        }

        public static void WriteAsset(int assetIDHash, object valueObj, FireWriter writer = null) {
            if(Instance.m_fbinAM.GetAsset(assetIDHash) != null) {
                Instance.m_fbinAM.WriteAsset(assetIDHash, valueObj, writer);
                return;
            }
            var ymlAsset = Instance.m_fymlAM.GetAsset(assetIDHash);
            if (ymlAsset != null) {
                Instance.m_fbinAM.ConvertFromYaml(valueObj, ymlAsset);
            }
            // foreach (var assetMenager in m_assetMenagers) {
            //     if (assetMenager.GetAsset(assetIDHash) != null) {
            //         assetMenager.WriteAsset(assetIDHash, valueObj, writer);
            //         return;
            //     }
            // }
        }

        /// <summary>
        /// Принимает путь с расширением-запросом (.create_scene|.create_tex|.create_mat|.create_prefab) 
        /// <br/>
        /// или источником (.obj|.jpg|.png) и создает соответствующий ассет 'filename.yml'. 
        /// <br/>
        /// Создает ассет, если файл не пустой и ассета с таким именем не существукт. 
        /// </summary>
        /// <returns>AssetIDHash != 0 если ассет успшно создан</returns>
        public static int CreateAssetFromSourceOrRequest(string path) {
            foreach(var assetMenager in Instance.m_assetMenagers)
                return assetMenager.CreateAssetFromSourceOrRequest(path);

            return 0;
        }

        public static bool RenameAsset(int assetIDHash, string newPath) {
            if(!HasAsset(assetIDHash))
                return false;

            Asset asset = null;
            foreach (var assetMenager in Instance.m_assetMenagers) {
                asset = assetMenager.GetAsset(assetIDHash);
                if (asset != null)
                    break;
            }
            var lastPath = asset.path;

            if (File.Exists(newPath) || !File.Exists(lastPath))
                return false;

            if (asset.sourceExt != "") {
                var lastSourcePath = Path.ChangeExtension(lastPath, asset.sourceExt);
                var newSourcePath = Path.ChangeExtension(newPath, asset.sourceExt);

                if (File.Exists(newSourcePath) || !File.Exists(lastSourcePath))
                    return false;

                File.Move(lastSourcePath, newSourcePath);
            }
            File.Move(lastPath, newPath);
            asset.path = newPath;

            Dll.AssetStore.RenameAsset(Game.gameRef, asset.assetIDHash, asset.name);

            return true;
        }

        public static void RemoveAsset(int assetIDHash) {
            if (!HasAsset(assetIDHash))
                return;

            Asset asset = null;
            foreach (var assetMenager in Instance.m_assetMenagers) {
                asset = assetMenager.GetAsset(assetIDHash);
                if (asset != null)
                    break;
            }
            if (File.Exists(asset.path))
                File.Delete(asset.path);

            if (asset.sourceExt != "") {
                var sourcePath = Path.ChangeExtension(asset.path, asset.sourceExt);

                if (File.Exists(sourcePath))
                    File.Delete(sourcePath);
            }
            foreach (var assetMenager in Instance.m_assetMenagers)
                assetMenager.ForgetAsset(assetIDHash);

            Dll.AssetStore.RemoveAsset(Game.gameRef, asset.scriptIDHash, asset.assetIDHash);
        }

        /// <summary>
        /// Создает новый ассет. Принимает либо путь к asset source, либо путь-запрос. 
        /// </summary>
        /// <param name="pathPtr"></param>
        /// <returns></returns>
        public static bool cpp_CreateAsset(ulong pathPtr) {
            var path = Assets.ReadCString(pathPtr);

            var fileExists = File.Exists(path);
            if (!fileExists)
                File.WriteAllText(path, "");

            if (CreateAssetFromSourceOrRequest(path) != 0)
                return true;

            if (!fileExists)
                File.Delete(path);

            return false;
        }

        public static bool cpp_RenameAsset(int assetGuidHash, ulong newPathPtr) {
            var newPath = Assets.ReadCString(newPathPtr);
            return RenameAsset(assetGuidHash, newPath);
        }

        public static void cpp_RemoveAsset(int assetIDHash) {
            RemoveAsset(assetIDHash);
        }

        /// <param name="objRef">From object</param>
        /// <param name="typeIdHash">To type</param>
        /// <returns></returns>
        public static bool cpp_IsAssignable(CsRef objRef, int typeIdHash) {
            var obj = CppLinked.GetObjectByRef(objRef);
            if (obj == null)
                return false;

            var fullName = GUIDAttribute.GetTypeByHash(typeIdHash).FullName; //Instance.GetTypeFullName(typeIdHash);
            var type = Type.GetType(fullName);

            var result = type.IsAssignableFrom(obj.GetType());
            return result;
        }

        public static void cpp_RequestAssetGuid(int assetGuidHash) {
            var guid = GetAssetGuid(assetGuidHash);
            Dll.AssetStore.buffer_set(Game.assetStoreRef, guid);
        }


    }
}
