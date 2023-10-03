using System;
using System.IO;
using System.Reflection;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Engine;
using EngineDll;

/// TODO: Переделать WriteAsset для обновления существующего ассета. 
/// TODO: В качестве AssetIDHash использовать hash от guid. 
/// TODO: Убрать замену Actor на Prefab.

namespace FireYaml {

    public class AssetStore {

        public class Asset {
            public string path;
            public DateTime time;
            public string guid;
            public int guidHash;
            public Type type;
            public string sourceExt = "";
        }

        public static AssetStore Instance { get; set; }

        //private static Dictionary<int, Type> m_guidHash_type = new Dictionary<int, Type>();
        private Dictionary<int, Asset> m_assetGuidHash_assetData = new Dictionary<int, Asset>();
        private Dictionary<int, YamlValues> m_assetGuidHash_assetValues = new Dictionary<int, YamlValues>();

        private HashSet<int> m_tmpAssetIdHashes = new HashSet<int>();

        public string ProjectPath;
        public string AssetsPath => $"{ProjectPath}/Assets";
        public string EngineAssetsPath => $"{ProjectPath}/Engine/Assets";
        public string EditorPath => $"{ProjectPath}/Editor";

        private uint m_nextAssetId = 1;
        private uint m_nextTmpAssetId = 1;

        private DateTime m_lastChangeTime = DateTime.UnixEpoch;

        public AssetStore(bool addDefaultAssets = true) {

        }

        public void Init(string projectPath) {
            ProjectPath = projectPath;

            Dll.AssetStore.projectPath_set(Game.assetStoreRef, ProjectPath);
            Dll.AssetStore.assetsPath_set(Game.assetStoreRef, AssetsPath);
            Dll.AssetStore.editorPath_set(Game.assetStoreRef, EditorPath);

            GUIDAttribute.CollectTypes();

            m_UpdateAssets(EditorPath, in DateTime.UnixEpoch);
            m_UpdateAssets(AssetsPath, in DateTime.UnixEpoch);

            SendTypesToCpp();
            SendAssetsInCpp();
        }

        public string GetAssetGuid(int assetGuidHash) {
            return m_assetGuidHash_assetData[assetGuidHash].guid;
        }

        public bool HasAssetPath(int assetIdHash) {
            return GetAssetPath(assetIdHash) != "";
        }

        public static bool HasAsset(int assetGuidHash) {
            return Instance.m_assetGuidHash_assetData.ContainsKey(assetGuidHash);
        }

        public Asset GetAsset(int assetGuidHash) {
            return Instance.m_assetGuidHash_assetData[assetGuidHash];
        }

        public Type GetAssetType(int assetGuidHash) {
            return m_assetGuidHash_assetData[assetGuidHash].type;

            // var scritpIdHash = m_assetIdHash_scriptIdHash[assetGuidHash];
            // var scriptId = m_scriptIdHash_scriptId[scritpIdHash];
            // return scriptId;
        }

        public YamlValues? GetAssetValues(int assetGuidHash) {

            //if (!GUIDAttribute.IsGuid(assetGuid))
            //    throw new Exception($"ThrowAssetValues: AssetGuid is not valid: {assetGuid}");

            //var asetGuidHash = assetGuid.GetHashCode();

            LoadAssetValues(assetGuidHash);

            if (!m_assetGuidHash_assetValues.ContainsKey(assetGuidHash))
                return null;

            return m_assetGuidHash_assetValues[assetGuidHash];
        }

        public YamlValues ThrowAssetValues(int assetGuidHash) {

            //if (!GUIDAttribute.IsGuid(assetGuid))
            //    throw new Exception($"ThrowAssetValues: AssetGuid is not valid: {assetGuid}");

            //var assetGuidHash = assetGuid.GetHashCode();
            LoadAssetValues(assetGuidHash);

            if (!m_assetGuidHash_assetValues.ContainsKey(assetGuidHash))
                throw new Exception($"Asset with assetGuidHash:{assetGuidHash} not exist");

            return m_assetGuidHash_assetValues[assetGuidHash];
        }

        public void LoadAssetValues(int assetGuidHash) {
            //var assetGuidHash = assetGuid.GetHashCode();
            if (m_assetGuidHash_assetValues.ContainsKey(assetGuidHash))
                return;

            var path = GetAssetPath(assetGuidHash);
            if (path == "")
                return;

            var text = File.ReadAllText(path);
            var values = new YamlValues().LoadFromText(text);

            m_assetGuidHash_assetValues[assetGuidHash] = values;
        }

        public void ReloadAssetValues(int assetGuidHash) {
            //var assetGuidHash = assetGuid.GetHashCode();

            if (m_assetGuidHash_assetValues.ContainsKey(assetGuidHash)) {
                m_assetGuidHash_assetValues.Remove(assetGuidHash);
                LoadAssetValues(assetGuidHash);
            }
        }

        public string GetAssetPath(int assetIdHash) {
            //var assetGuidHash = assetGuid.GetHashCode();
            if(m_assetGuidHash_assetData.ContainsKey(assetIdHash))
                return m_assetGuidHash_assetData[assetIdHash].path;

            return "";
        }

        public AssetInfo GetAssetInfo(int assetIdHash) {
            var values = GetAssetValues(assetIdHash);

            var assetIdPath = ".file0.assetId";
            var filesPath = ".file0.files";

            var hasAssetId = values.HasValue(assetIdPath);
            var hasfiles = values.HasValue(filesPath);

            if (!hasAssetId || !hasfiles)
                throw new Exception("Asset not contains AssetInfo in file0");

            var info = new AssetInfo();
            info.assetId = values.GetValue(assetIdPath).value;
            info.files = int.Parse(values.GetValue(filesPath).value);

            return info;
        }

        public int GetAssetIdHashFromFile(string assetPath) {
            var assetGuid = m_ReadAssetGuid(assetPath);
            if(assetGuid != "")
                return assetGuid.GetHashCode();

            return 0;
        }

        /// <summary>
        /// <return>Возвращает AssetIDHash либо 0, если ассет не был создан.</return>
        /// </summary>
        public int CreateNewAsset(Type type, string assetPath) {
            var assetGuid = Guid.NewGuid().ToString();
            var assetGuidHash = assetGuid.GetHashCode();
            var sourcePath = "";

            if (FireWriter.IsAssetWithSource(type)) {
                sourcePath = Assets.FindSourceFile(type, assetPath);
                if (sourcePath == "") {
                    Console.WriteLine($"Assets.Create: Asset source not found: {assetPath}");
                    return 0;
                }
            }
            var assetHeader = new string[] {
                ".file0.assetId: {AssetId: "+ assetGuid +"}",
                ".file0.files: {Var: 0}"
            };

            File.WriteAllLines(assetPath, assetHeader);
            WriteNewAsset(type, assetPath, assetGuid, sourcePath);

            var asset = GetAsset(assetGuidHash);
            var AssetTypeId = GUIDAttribute.GetGuidHash(asset.type);
            var assetName = Path.GetFileNameWithoutExtension(asset.path);
            Dll.AssetStore.AddAsset(Game.gameRef, AssetTypeId, asset.guidHash, assetName);

            Console.WriteLine($"CreatAsset: {asset.time}: '{assetPath}'");

            return asset.guidHash;
        }

        /// <summary>
        /// Записывает ассет с перезаписью, если файла ассета нет, создает новый ассет.
        /// </summary>
        /// TODO: Переделать для обновления существующего ассета. 
        public void WriteAsset(int assetIdHash, object valueObj, FireWriter writer = null) {
            if (!HasAsset(assetIdHash))
                throw new Exception($"Asset with assetIdHash: '{assetIdHash}' not exist.");

            var path = GetAssetPath(assetIdHash);
            var fullPath = Path.GetFullPath(path);
            var projectFullPath = Path.GetFullPath(ProjectPath);

            if (!fullPath.Contains(projectFullPath))
                throw new Exception($"Asset Path not contains project path");

            if (!File.Exists(fullPath))
                throw new Exception($"WriteAsset: Asset not exist: '{fullPath}'");

            var assetGuid = GetAssetGuid(assetIdHash);

            m_WriteAssetData(assetGuid, valueObj, fullPath, writer);

            UpdateAssetData(valueObj.GetType(), fullPath);
            ReloadAssetValues(assetIdHash);
        }

        private void m_WriteAssetData(string assetGuid, object valueObj, string fullPath, FireWriter writer = null) {
            var serializer = writer;
            if (serializer == null)
                serializer = new FireWriter(ignoreExistingIds: true, startId: 1);

            serializer.Serialize(valueObj);

            if (!serializer.Result)
                throw new Exception($"Can not create asset: Serialization failed.");

            var values = serializer.Mods;

            values.AddValue(".file0.assetId", new YamlValue(YamlValue.Type.AssetId, assetGuid));
            values.AddValue(".file0.files", new YamlValue(YamlValue.Type.Var, $"{serializer.FilesCount}"));

            File.WriteAllText(fullPath, values.ToSortedText());
        }

        public void WriteNewAsset(Type assetType, string assetPath, string assetGuid, string assetSourcePath = "") {
            var assetGuidHash = assetGuid.GetHashCode();
            var fullPath = Path.GetFullPath(assetPath);
            var projectFullPath = Path.GetFullPath(ProjectPath);

            if (!fullPath.Contains(projectFullPath))
                throw new Exception($"Asset Path not contains project path");

            if (!File.Exists(assetPath))
                throw new FileNotFoundException($"Assets.Create: {assetPath}");

            if (assetType == typeof(Engine.Actor)) {
                m_WriteNewPrefab(assetPath, assetGuid);
            } else {
                var valueObj = FireReader.CreateInstance(assetType);

                if (FireWriter.IsAsset(assetType))
                    FireReader.InitIAsset(ref valueObj, assetGuid, 0);

                if (FireWriter.IsAssetWithSource(assetType)) {
                    if (assetSourcePath == "")
                        throw new Exception("Assets.Create: Asset with source need a sourcePath");

                    var ext = Path.GetExtension(assetSourcePath);

                    FireReader.InitISourceAsset(ref valueObj, ext);
                }
                m_WriteAssetData(assetGuid, valueObj, fullPath);
            }
            UpdateAssetData(assetType, fullPath);
            ReloadAssetValues(assetGuidHash);
        }

        private static void m_WriteNewPrefab(string assetPath, string assetGuid)
        {

            var filesCount = 1;
            var scriptName = nameof(Engine.Actor);
            var scriptGuid = GUIDAttribute.GetGuid(typeof(Engine.Actor));

            var text = File.ReadAllText(assetPath);
            var values = new YamlValues().LoadFromText(text);

            values.SetValue(".file0.assetId", new YamlValue(YamlValue.Type.AssetId, assetGuid));
            values.SetValue(".file0.files", new YamlValue(YamlValue.Type.Var, $"{filesCount}"));

            values.AddValue(".file1!script", new YamlValue(YamlValue.Type.Var, $"{scriptName}"));
            values.AddValue(".file1!scriptId", new YamlValue(YamlValue.Type.AssetId, $"{scriptGuid}"));

            File.WriteAllText(assetPath, values.ToSortedText());
        }

        /// Временные ассеты
        // Не добавляются в загруженные ассеты
        // Не сохраняются в файл при сериализации
        // Функции обычно вызываются из С++

        public static uint CreateTmpAssetIdInt() { // Cpp
            var id = AssetStore.Instance.m_nextTmpAssetId++;
            return id;
        }

        public static void AddTmpAssetIdHash(int tmpAssetIdHash) { // Cpp
            Instance.m_tmpAssetIdHashes.Add(tmpAssetIdHash);
        }

        public static bool IsTmpAssetId(int tmpAssetIdHash) => Instance.m_tmpAssetIdHashes.Contains(tmpAssetIdHash);

        private bool m_IsAssetExt(string ext) {
            return
                ext == ".yml";
        }

        private bool m_IsNewAssetExt(string ext) {
            return
                ext == ".scene" ||
                ext == ".tex" ||
                ext == ".material" ||
                ext == ".prefab";
        }

        private bool m_IsSourceExt(string ext) {
            return
                ext == ".obj" ||
                ext == ".png" ||
                ext == ".jpg";
        }

        public void SendTypesToCpp() {
            Dll.AssetStore.ClearTypes(Game.gameRef);
            Dll.AssetStore.ClearAssetTypes(Game.gameRef);

            foreach (var guidHash_type in GUIDAttribute.types) {
                var typeGuidHash = guidHash_type.Key;
                var type = guidHash_type.Value;
                //var typeGuid = GUIDAttribute.GetGuid(type);

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

            foreach (var assetGuidHash in m_assetGuidHash_assetData.Keys) {
                var assetData = m_assetGuidHash_assetData[assetGuidHash];

                var assetName = Path.GetFileNameWithoutExtension(assetData.path);
                var scriptIdHash = GUIDAttribute.GetGuidHash(assetData.type);

                if (assetData.type == typeof(Engine.Actor))
                    scriptIdHash = prefabGuidHash;

                Dll.AssetStore.AddAsset(Game.gameRef, scriptIdHash, assetGuidHash, assetName);
            }
        }

        private DateTime m_UpdateAssets(string path, in DateTime lastTime) {
            var dirPath = Path.GetFullPath(path);
            var newTime = lastTime; 

            foreach (string filePath in Directory.EnumerateFiles(dirPath)) {
                var fileTime = File.GetLastWriteTime(filePath);

                if (fileTime > lastTime) {
                    var ext = Path.GetExtension(filePath);
                    bool watched = false;

                    if (m_IsNewAssetExt(ext) || m_IsSourceExt(ext)) {
                        if(AddAsset(filePath) != 0)
                            watched = true;
                    }
                    if (m_IsAssetExt(ext)) {
                        m_UpdateAssetPath(filePath);
                        watched = true;
                    }
                    if (watched && fileTime > newTime)
                        newTime = fileTime;
                }
            }
            foreach (string nextDirPath in Directory.EnumerateDirectories(dirPath)) {
                var name = Path.GetFileName(nextDirPath);
                if (name != "Ignore") {
                    var dirName = m_UpdateAssets(nextDirPath, lastTime);

                    if(dirName > newTime)
                        newTime = dirName;
                }
            }
            return newTime;
        }

        public static bool CreateAsset(ulong pathPtr) {
            var path = Assets.ReadCString(pathPtr);

            var fileExists = File.Exists(path);
            if(!fileExists)
                File.WriteAllText(path, "");

            if(Instance.AddAsset(path) != 0)
                return true;

            if (!fileExists)
                File.Delete(path);

            return false;
        }

        public static bool SetAssetPath(int assetGuidHash, ulong newPathPtr) {
            var newPath = Assets.ReadCString(newPathPtr);
            return Instance.SetAssetPath(assetGuidHash, newPath);
        }

        /// <summary>
        /// Создает новый пустой ассет на диске
        /// </summary>
        /// <param name="path">Путь с именем ассета и расфирением конкретного типа ассета</param>
        /// <returns>AssetIDHash != 0 если ассет был успшно создан</returns>
        public int AddAsset(string path) {
            var ext = Path.GetExtension(path);
            var isSourceFile = m_IsSourceExt(ext);

            if (!isSourceFile) {
                long length = new System.IO.FileInfo(path).Length;
                if (length != 0)
                    return 0;
            }

            var assetPath = Path.ChangeExtension(path, "yml");
            if (File.Exists(assetPath))
                return 0;

            var type = Assets.GetAssetTypeByExt(ext);
            if (type == null)
                return 0;

            if (isSourceFile)
                File.WriteAllText(assetPath, "");
            else
                File.Move(path, assetPath);

            var assetIdHash = CreateNewAsset(type, assetPath);
            return assetIdHash;
        }

        public Asset UpdateAssetData(Type type, string assetPath) {
            var assetGuid = m_ReadAssetGuid(assetPath);
            var assetGuidHash = assetGuid.GetHashCode();
            var sourcePath = "";

            if (FireWriter.IsAssetWithSource(type))
                sourcePath = Assets.FindSourceFile(type, assetPath);

            var data = new Asset();

            data.path = assetPath;
            data.time = File.GetLastWriteTime(assetPath);
            data.guid = assetGuid;
            data.guidHash = assetGuidHash;
            data.type = type != typeof(Actor) ? type : typeof(Prefab);
            data.sourceExt = sourcePath != "" ? Path.GetExtension(sourcePath) : "";

            m_assetGuidHash_assetData[data.guidHash] = data;

            return data;
        }

        private void m_UpdateAssetPath(string assetPath) {
            var assetGuid = m_ReadAssetGuid(assetPath);
            var scriptGuid = m_ReadScriptGuid(assetPath);

            Guid guid;
            if (!Guid.TryParse(assetGuid, out guid)) {
                Console.WriteLine($"Bad AssetID: '{assetGuid}' in path: '{assetPath}'");
                return;
            }
            if (!Guid.TryParse(scriptGuid, out guid)) {
                Console.WriteLine($"Bad asset ScriptId: '{scriptGuid}' in path: '{assetPath}'");
                return;
            }

            var sourcePath = "";
            var assetType = GUIDAttribute.GetTypeByGuid(scriptGuid);;

            if (FireWriter.IsAssetWithSource(assetType)) {
                sourcePath = Assets.FindSourceFile(assetType, assetPath);
                if (sourcePath == "")
                    Console.WriteLine($"UpdateAsset: Asset source not found: {assetPath}");
            }

            var data = new Asset();

            data.path = assetPath;
            data.time = File.GetLastWriteTime(assetPath);
            data.guid = assetGuid;
            data.guidHash = data.guid.GetHashCode();
            data.type = GUIDAttribute.GetTypeByGuid(scriptGuid);
            data.sourceExt = sourcePath != "" ? Path.GetExtension(sourcePath) : "";

            if(data.type == null) {
                Console.WriteLine($"UpdateAssetPath: Misssing ScriptId: '{scriptGuid}' in path: '{assetPath}'");
                return;
            }

            m_assetGuidHash_assetData[data.guidHash] = data;

            Console.WriteLine($"UpdateAssetPath: {data.time}: '{assetPath}'");
        }

        public bool SetAssetPath(int assetGuidHash, string newPath) {
            if(!m_assetGuidHash_assetData.ContainsKey(assetGuidHash))
                return false;

            var assetData = m_assetGuidHash_assetData[assetGuidHash];
            var lastPath = assetData.path;

            if (File.Exists(newPath) || !File.Exists(lastPath))
                return false;

            if (assetData.sourceExt != "") {
                var lastSourcePath = Path.ChangeExtension(lastPath, assetData.sourceExt);
                var newSourcePath = Path.ChangeExtension(newPath, assetData.sourceExt);

                if (File.Exists(newSourcePath) || !File.Exists(lastSourcePath))
                    return false;

                File.Move(lastSourcePath, newSourcePath);
            }
            File.Move(lastPath, newPath);
            assetData.path = newPath;

            var name = Path.GetFileNameWithoutExtension(newPath);
            Dll.AssetStore.RenameAsset(Game.gameRef, assetData.guidHash, name);

            return true;
        }

        public static void RemoveAsset(int assetGuidHash) {
            if (!Instance.m_assetGuidHash_assetData.ContainsKey(assetGuidHash))
                return;

            var asset = Instance.m_assetGuidHash_assetData[assetGuidHash];

            if(File.Exists(asset.path))
                File.Delete(asset.path);

            if(asset.sourceExt != "") {
                var sourcePath = Path.ChangeExtension(asset.path, asset.sourceExt);

                if (File.Exists(sourcePath))
                    File.Delete(sourcePath);
            }

            Instance.m_assetGuidHash_assetData.Remove(asset.guidHash);

            var typeGuidHash = GUIDAttribute.GetGuidHash(asset.type);
            Dll.AssetStore.RemoveAsset(Game.gameRef, typeGuidHash, asset.guidHash);
        }

        private string m_ReadAssetGuid(string assetPath) {
            foreach (var line in File.ReadLines(assetPath)) {
                var values = new YamlValues().LoadFromText(line);
                return values.GetValue(".file0.assetId", "");
            }
            return "";
        }

        private string m_ReadScriptGuid(string assetPath) {
            var scpiptPath = ".file1!scriptId";

            foreach (var line in File.ReadLines(assetPath)) {
                var values = new YamlValues().LoadFromText(line);
                if(values.HasValue(scpiptPath))
                    return values.GetValue(scpiptPath, "");
            }
            return "";
        }

        /// <param name="objRef">From object</param>
        /// <param name="typeIdHash">To type</param>
        /// <returns></returns>
        public static bool IsAssignable(CsRef objRef, int typeIdHash) {
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
