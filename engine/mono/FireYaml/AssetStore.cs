using System;
using System.IO;
using System.Reflection;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Engine;
using EngineDll;


namespace FireYaml {

    public class AssetStore {

        public static AssetStore Instance { get; set; }

        //private static Dictionary<int, Type> m_guidHash_type = new Dictionary<int, Type>();
        //private Dictionary<int, Asset> m_assetGuidHash_assetData = new Dictionary<int, Asset>();
        //private Dictionary<int, YamlValues> m_assetGuidHash_assetValues = new Dictionary<int, YamlValues>();

        private HashSet<int> m_tmpAssetIdHashes = new HashSet<int>();

        public string ProjectPath;
        public string AssetsPath => $"{ProjectPath}/Assets";
        public string EngineAssetsPath => $"{ProjectPath}/Engine/Assets";
        public string EditorPath => $"{ProjectPath}/Editor";

        private uint m_nextAssetId = 1;
        private uint m_nextTmpAssetId = 1;

        private DateTime m_lastChangeTime = DateTime.UnixEpoch;

        private FireBinAssetMenager m_fbinAM;
        private FireYmlAssetMenager m_fymlAM;

        public AssetStore(bool addDefaultAssets = true) {

        }

        public void Init(string projectPath) {
            ProjectPath = projectPath;

            m_fbinAM = new FireBinAssetMenager(projectPath);
            m_fymlAM = new FireYmlAssetMenager(projectPath);

            Dll.AssetStore.projectPath_set(Game.assetStoreRef, ProjectPath);
            Dll.AssetStore.assetsPath_set(Game.assetStoreRef, AssetsPath);
            Dll.AssetStore.editorPath_set(Game.assetStoreRef, EditorPath);

            GUIDAttribute.CollectTypes();

            //m_fbinAM.Load();
            m_fymlAM.Load();

            //m_UpdateAssets(EditorPath, in DateTime.UnixEpoch);
            //m_UpdateAssets(AssetsPath, in DateTime.UnixEpoch);

            SendTypesToCpp();
            SendAssetsInCpp();
        }

        public string GetAssetGuid(int assetIdHash) {
            return m_fymlAM.GetAsset(assetIdHash).assetID;
        }

        public static bool HasAsset(int assetIdHash) {
            return Instance.m_fymlAM.GetAsset(assetIdHash) != null;
        }

        public Type GetAssetType(int assetIdHash) {
            return m_fymlAM.GetAsset(assetIdHash).scriptType;
        }

        public YamlValues ThrowAssetValues(int assetIdHash) {
            var asset = m_fymlAM.GetAsset(assetIdHash);
            if (asset == null)
                throw new Exception($"Asset with assetGuidHash:{assetIdHash} not exist");

            m_fymlAM.LoadAssetData(assetIdHash);

            return asset.data;
        }

        public bool HasAssetPath(int assetIdHash) {
            var asset = m_fymlAM.GetAsset(assetIdHash);
            if (asset != null)
                return asset.path != "";

            return false;
        }

        public string GetAssetPath(int assetIdHash) {
            var asset = m_fymlAM.GetAsset(assetIdHash);
            if(asset != null)
                return asset.path;

            return "";
        }

        public int GetAssetFilesCount(int assetIdHash) {
            var yamlAsset = m_fymlAM.GetAsset(assetIdHash);
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

        public static uint cpp_CreateTmpAssetIdInt() {
            var id = AssetStore.Instance.m_nextTmpAssetId++;
            return id;
        }

        public static void cpp_AddTmpAssetIdHash(int tmpAssetIdHash) {
            Instance.m_tmpAssetIdHashes.Add(tmpAssetIdHash);
        }

        public static bool IsTmpAssetId(int tmpAssetIdHash) => Instance.m_tmpAssetIdHashes.Contains(tmpAssetIdHash);

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

            foreach(var asset in m_fymlAM.EnumerateAssets())
                Dll.AssetStore.AddAsset(Game.gameRef, asset.scriptIDHash, asset.assetIDHash, asset.name);
        }

        public int CreateNewAsset(Type type, string assetPath, string sourceExt = "") {
            return m_fymlAM.CreateNewAsset(type, assetPath, sourceExt);
        }

        public void WriteAsset(int assetIDHash, object valueObj, FireWriter writer = null) {
            m_fymlAM.WriteAsset(assetIDHash, valueObj, writer);
        }

        /// <summary>
        /// Принимает путь с расширением-запросом (.create_scene|.create_tex|.create_mat|.create_prefab) 
        /// <br/>
        /// или источником (.obj|.jpg|.png) и создает соответствующий ассет 'filename.yml'. 
        /// <br/>
        /// Создает ассет, если файл не пустой и ассета с таким именем не существукт. 
        /// </summary>
        /// <returns>AssetIDHash != 0 если ассет успшно создан</returns>
        public int CreateAssetFromSourceOrRequest(string path) {
            return m_fymlAM.CreateAssetFromSourceOrRequest(path);
        }

        public bool RenameAsset(int assetIDHash, string newPath) {
            if(!HasAsset(assetIDHash))
                return false;

            var asset = m_fymlAM.GetAsset(assetIDHash);
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

        public void RemoveAsset(int assetIDHash) {
            if (!HasAsset(assetIDHash))
                return;

            var asset = m_fymlAM.GetAsset(assetIDHash);

            if (File.Exists(asset.path))
                File.Delete(asset.path);

            if (asset.sourceExt != "") {
                var sourcePath = Path.ChangeExtension(asset.path, asset.sourceExt);

                if (File.Exists(sourcePath))
                    File.Delete(sourcePath);
            }

            m_fymlAM.ForgetAsset(assetIDHash);

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

            if (Instance.CreateAssetFromSourceOrRequest(path) != 0)
                return true;

            if (!fileExists)
                File.Delete(path);

            return false;
        }

        public static bool cpp_RenameAsset(int assetGuidHash, ulong newPathPtr) {
            var newPath = Assets.ReadCString(newPathPtr);
            return Instance.RenameAsset(assetGuidHash, newPath);
        }

        public static void cpp_RemoveAsset(int assetIDHash) {
            Instance.RemoveAsset(assetIDHash);
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
            var guid = Instance.GetAssetGuid(assetGuidHash);
            Dll.AssetStore.buffer_set(Game.assetStoreRef, guid);
        }


    }
}
