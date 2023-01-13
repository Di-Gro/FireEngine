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
        public static string M_Default = "M_Default";

        private Dictionary<int, string> m_scriptIdHash_scriptName = new Dictionary<int, string>();
        private Dictionary<string, string> m_scriptName_scriptId = new Dictionary<string, string>();
        private Dictionary<int, string> m_scriptIdHash_scriptId = new Dictionary<int, string>();
        private Dictionary<string, int> m_scriptName_scriptIdHash = new Dictionary<string, int>();


        private Dictionary<string, YamlValues> tmp_assetValues = new Dictionary<string, YamlValues>();

        private Dictionary<int, string> m_assetIdHash_assetId = new Dictionary<int, string>();
        private Dictionary<int, string> m_assetIdHash_assetPath = new Dictionary<int, string>();
        private Dictionary<int, int> m_assetIdHash_scriptIdHash = new Dictionary<int, int>();

        private HashSet<int> m_tmpAssetIdHashes = new HashSet<int>();

        private HashSet<int> m_missingAssetAIdHash = new HashSet<int>();

        public string ProjectPath;
        public string AssetsPath => $"{ProjectPath}/Assets";
        public string EngineAssetsPath => $"{ProjectPath}/Engine/Assets";
        public string EditorPath => $"{ProjectPath}/Editor";

        private uint m_nextAssetId = 1;
        private uint m_nextTmpAssetId = 1;

        public AssetStore(bool addDefaultAssets = true) {

            if (addDefaultAssets) {
                var project = ProjectPath;

                // AddAssetId("MESH000001", $"{AssetsPath}/Meshes/Farm/House_Red.yml");
                // AddAssetId("MESH000002", $"{AssetsPath}/Meshes/Farm/House_Purple.yml");
                // AddAssetId("MESH000003", $"{AssetsPath}/Meshes/Farm/House_Blue.yml");
                // AddAssetId("TestMesh1", $"{AssetsPath}/Meshes/TestMesh1.yml");
                // AddAssetId("test_navmesh", $"{AssetsPath}/Meshes/test_navmesh.yml");

                // AddAssetId("IMG0000004", $"{AssetsPath}/Images/Gradients.yml");
                // AddAssetId("TestImage1", $"{AssetsPath}/Images/TestImage1.yml");

                // AddAssetId("TEX0000005", $"{AssetsPath}/Textures/Texture.yml");
                // AddAssetId("TestTexture1", $"{AssetsPath}/Textures/TestTexture1.yml");

                // AddAssetId("MAT0000006", $"{AssetsPath}/Materials/Material.yml");
                // AddAssetId("TestMaterial1", $"{AssetsPath}/Materials/TestMaterial1.yml");

                // AddAssetId("TestMesh", $"{AssetsPath}/Prefabs/TestMesh.yml");
                // AddAssetId("TestMeshPrefab", $"{AssetsPath}/Prefabs/TestMeshPrefab.yml");
                // AddAssetId("TestPrefab", $"{AssetsPath}/Prefabs/TestPrefab.yml");
                // AddAssetId("TestPrefab2", $"{AssetsPath}/Prefabs/TestPrefab2.yml");
                
                // AddAssetId("scene_1", $"{AssetsPath}/Scenes/scene_1.yml");
            }
        }

        public string GetTypeFullName(string scriptId) => m_scriptIdHash_scriptName[scriptId.GetHashCode()];
        public string GetTypeFullName(int scriptIdHash) => m_scriptIdHash_scriptName[scriptIdHash];
        public bool HasTypeName(string typeName) => m_scriptName_scriptId.ContainsKey(typeName);
        public string GetScriptIdByTypeName(string typeName) => m_scriptName_scriptId[typeName];
        
        public void AddScriptId(string scriptId, string scriptName) {
            var scriptIdHash = scriptId.GetHashCode();

            m_scriptIdHash_scriptId[scriptIdHash] = scriptId;
            m_scriptIdHash_scriptName[scriptIdHash] = scriptName;
            m_scriptName_scriptId[scriptName] = scriptId;
            m_scriptName_scriptIdHash[scriptName] = scriptIdHash;
        }

        public void AddAssetId(string assetId, string path) {
            var assetIdHash = assetId.GetHashCode();
            m_assetIdHash_assetId[assetIdHash] = assetId;
            m_assetIdHash_assetPath[assetIdHash] = path;
        }

        public string GetAssetId(int assetId) {
            return m_assetIdHash_assetId[assetId];
        }

        public bool HasAssetPath(string assetId) {
            return GetAssetPath(assetId) != "";
        }

        public static bool HasAsset(int assetIdHash) {
            return Instance.m_assetIdHash_assetPath.ContainsKey(assetIdHash);
        }

        public bool TryGetAssetIdByType(string typeName, out string assetId) {
            assetId = "";
            if (m_scriptName_scriptId.ContainsKey(typeName)) {
                assetId = m_scriptName_scriptId[typeName];
                return true;
            }
            return false;
        }

        public bool TryGetAssetIdByType(string typeName, out int assetId) {
            assetId = 0;
            if (m_scriptName_scriptId.ContainsKey(typeName)) {
                assetId = m_scriptName_scriptIdHash[typeName];
                return true;
            }
            return false;
        }

        public string GetAssetScriptId(int assetIdHash) {
            var scritpIdHash = m_assetIdHash_scriptIdHash[assetIdHash];
            var scriptId = m_scriptIdHash_scriptId[scritpIdHash];
            return scriptId;

            // var assetId = GetAssetId(assetIdHash);
            // var values = GetAssetValues(assetId);

            // var scriptIdPath = ".file1!scriptId";
            // var hasScriptId = values.HasValue(scriptIdPath);
            // if (!hasScriptId )
            //     throw new Exception("Asset not contains .file1!scriptId");

            // var typeId = values.GetValue(scriptIdPath).value;

            // return typeId;
        }

        public YamlValues? GetAssetValues(string assetId) {
            LoadAsset(assetId);

            if (!tmp_assetValues.ContainsKey(assetId))
                return null;

            return tmp_assetValues[assetId];
        }

        public YamlValues ThrowAssetValues(string assetId) {
            LoadAsset(assetId);

            if (!tmp_assetValues.ContainsKey(assetId))
                throw new Exception($"Asset with assetId:{assetId} not exist");

            return tmp_assetValues[assetId];
        }

        public void LoadAsset(string assetId) {
            if (tmp_assetValues.ContainsKey(assetId))
                return;

            var path = GetAssetPath(assetId);
            if (path == "")
                return;

            var text = File.ReadAllText(path);
            var values = new YamlValues().LoadFromText(text);
            
            var scriptIdPath = ".file1!scriptId";
            var scriptId = values.GetValue(scriptIdPath).value;
            var scriptIdHash = scriptId.GetHashCode();
            var assetIdHash = assetId.GetHashCode();

            tmp_assetValues[assetId] = values;
            m_assetIdHash_scriptIdHash[assetIdHash] = scriptIdHash;
        }

        public void ReloadAsset(string prefabId) {
            if (tmp_assetValues.ContainsKey(prefabId)) {
                tmp_assetValues.Remove(prefabId);
                LoadAsset(prefabId);
            }
        }

        public string GetAssetPath(string assetId) {
            var assetIdInt = assetId.GetHashCode();
            if (m_assetIdHash_assetPath.ContainsKey(assetIdInt))
                return m_assetIdHash_assetPath[assetIdInt];
            
            return "";
        }

        public void ResetAssetPath(int assetIdHash, string path) {
            if (m_assetIdHash_assetPath.ContainsKey(assetIdHash))
                m_assetIdHash_assetPath[assetIdHash] = path;
        }

        public AssetInfo GetAssetInfo(string assetId) {
            var values = GetAssetValues(assetId);

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

        public AssetInfo ReadAssetInfo(string assetId) {
            var values = GetAssetValues(assetId);

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

        public void CreateAsset(string path, object data, string tmp_id) {
            if(!path.Contains(ProjectPath))
                throw new Exception($"Asset Path not contains project path");

            var serializer = new FireWriter(ignoreExistingIds: true, startId: 1);
            serializer.Serialize(data);

            if (!serializer.Result)
                throw new Exception($"Can not create asset: Serialization failed.");

            var values = serializer.Values;
            var assetPath = path;

            values.AddValue(".file0.assetId", new YamlValue(YamlValue.Type.AssetId, tmp_id));
            values.AddValue(".file0.files", new YamlValue(YamlValue.Type.Var, $"{serializer.FilesCount}"));


            File.WriteAllText(assetPath, values.ToSortedText());

            AddAssetId(tmp_id, assetPath);
            ReloadAsset(tmp_id);
        }

        public void UpdateAsset(string assetId, object data) {
            if (!HasAssetPath(assetId))
                throw new Exception($"Asset with assetId:{assetId} not exist");

            var assetInfo = GetAssetInfo(assetId);
            var assetPath = GetAssetPath(assetId);

            var serializer = new FireWriter(ignoreExistingIds: false, writeNewIds: true, startId: assetInfo.files + 1);
            serializer.Serialize(data);

            if (!serializer.Result)
                throw new Exception($"Can not update asset: Serialization failed.");

            var values = serializer.Values;

            values.AddValue(".file0.assetId", new YamlValue(YamlValue.Type.AssetId, assetInfo.assetId));
            values.AddValue(".file0.files", new YamlValue(YamlValue.Type.Var, $"{serializer.FilesCount}"));

            File.WriteAllText(assetPath, values.ToSortedText());

            tmp_assetValues.Remove(assetId);
            LoadAsset(assetId);
        }

        public void UpdateAssetValues(string assetId) {
            if (!HasAssetPath(assetId))
                throw new Exception($"Asset with assetId:{assetId} not exist");

            var assetPath = GetAssetPath(assetId);
            var values = GetAssetValues(assetId);

            File.WriteAllText(assetPath, values.ToSortedText());

            tmp_assetValues.Remove(assetId);
            LoadAsset(assetId);
        }

        public static string CreateAssetId() {
            var id = CreateAssetIdInt();
            var idStr = id.ToString();
            var nullCount = 10 - idStr.Length;
            var assetId = new string('0', nullCount) + idStr;
            return assetId;
        }

        public static string CreateTmpAssetId() {
            var id = CreateTmpAssetIdInt();
            var idStr = id.ToString();
            var nullCount = 10 - idStr.Length;
            var assetId = new string('0', nullCount) + idStr;
            assetId = $"tmp_{id.ToString()}";

            AddTmpAssetIdHash(assetId.GetHashCode());

            return assetId;
        }

        public static uint CreateAssetIdInt() {
            var id = AssetStore.Instance.m_nextAssetId++;
            return id;
        }

        public static uint CreateTmpAssetIdInt() {
            var id = AssetStore.Instance.m_nextTmpAssetId++;
            return id;
        }

        public static void AddTmpAssetIdHash(int tmpAssetIdHash) {
            Instance.m_tmpAssetIdHashes.Add(tmpAssetIdHash);
        }

        public static bool IsTmpAssetId(int tmpAssetIdHash) => Instance.m_tmpAssetIdHashes.Contains(tmpAssetIdHash);

        public static void UpdateTypesInCpp() {
            Dll.AssetStore.projectPath_set(Game.assetStoreRef, Instance.ProjectPath);
            Dll.AssetStore.assetsPath_set(Game.assetStoreRef, Instance.AssetsPath);
            Dll.AssetStore.editorPath_set(Game.assetStoreRef, Instance.EditorPath);

            var typeNames = new List<string>();
            var assembly = Assembly.GetAssembly(typeof(AssetStore));

            var types = assembly.GetTypes();
            foreach(var type in types) {
                if (FireWriter.IsComponent(type) || FireWriter.IsAsset(type) || FireWriter.IsFile(type)) { 
                    typeNames.Add(type.FullName);
                    if(!Instance.HasTypeName(type.FullName))
                        Instance.AddScriptId(CreateAssetId(), type.FullName);
                }
            }
            typeNames.Sort();

            Dll.AssetStore.ClearTypes(Game.gameRef);
            Dll.AssetStore.ClearAssetTypes(Game.gameRef);

            foreach (var typeName in typeNames) {
                var type = Type.GetType(typeName);
                var typeId = Instance.GetScriptIdByTypeName(type.FullName);
                var typeIdHash = typeId.GetHashCode();

                Dll.AssetStore.SetType(Game.gameRef, typeIdHash, type.FullName, type.Name);

                if(FireWriter.IsUserComponent(type))
                    Dll.AssetStore.AddComponent(Game.gameRef, typeIdHash);
                
                if(FireWriter.IsAsset(type))
                    Dll.AssetStore.AddAssetType(Game.gameRef, typeIdHash);
            }
        }

        public static void UpdateAssetsInCpp() {
            var store = Instance;

            var actorId = Instance.GetScriptIdByTypeName(typeof(Engine.Actor).FullName);
            var componentId = Instance.GetScriptIdByTypeName(typeof(Engine.Component).FullName);
            var prefabId = Instance.GetScriptIdByTypeName(typeof(Engine.Prefab).FullName);

            Dll.AssetStore.ClearAssets(Game.gameRef);
            Dll.AssetStore.actorTypeIdHash_set(Game.assetStoreRef, actorId.GetHashCode());
            Dll.AssetStore.componentTypeIdHash_set(Game.assetStoreRef, componentId.GetHashCode());

            foreach(var assetIdHash in Instance.m_assetIdHash_assetId.Keys) {
                var assetId = Instance.GetAssetId(assetIdHash);
                var scriptId = m_ReadScriptId(assetIdHash);
                if (scriptId == actorId)
                    scriptId = prefabId;
                
                var scriptIdHash = scriptId.GetHashCode();
                var typeFullName = Instance.m_scriptIdHash_scriptName[scriptIdHash];
                var typeName = Type.GetType(typeFullName).Name;
                var assetName = m_ReadAssetName(assetIdHash);

                Instance.m_assetIdHash_scriptIdHash[assetIdHash] =  scriptIdHash;
                Dll.AssetStore.AddAsset(Game.gameRef, scriptIdHash, assetIdHash, assetName);
            }
        }

        private static string m_ReadScriptId(int assetIdHash) {
            var inst = Instance;
            var path = Instance.m_assetIdHash_assetPath[assetIdHash];

            var lines = File.ReadLines(path);
            var scriptIdPath = ".file1!scriptId";

            foreach(var line in lines) {
                if(line.StartsWith(scriptIdPath)) {
                    var scriptId = new YamlValues().LoadFromText(line).GetValue(scriptIdPath).value;
                    return scriptId;
                }
            }
            return "";
        }

        private static string m_ReadAssetName(int assetIdHash) {
            var path = Instance.m_assetIdHash_assetPath[assetIdHash];
            var fileName = Path.GetFileNameWithoutExtension(path);
            return fileName;
        }

        
        public void Init(string projectPath) {
            ProjectPath = projectPath;

            var typeTablePath = $"{EditorPath}/TypeTable.yml";
            var assetTablePath = $"{EditorPath}/AssetTable.yml";

            if (!File.Exists(typeTablePath))
                m_InitTypeTable(typeTablePath);

            if (!File.Exists(assetTablePath))
                m_InitAssetTable(assetTablePath);

            m_ReadTypes(typeTablePath);
            m_ReadAssets(assetTablePath);

            m_AddAssemblyTypes();
            m_UpdateAssets(AssetsPath);

            // TODO: Продолжить
            m_UpdateMissingAssets();

            throw new NotImplementedException();

            UpdateTypesInCpp();
            UpdateAssetsInCpp();
        }

        private void m_InitTypeTable(string path) {
            var list = new List<string>();
            var types = m_GetDefaultTypes();

            foreach (var name_typeId in types)
                list.Add($"{name_typeId.Key}: {name_typeId.Value}");

            File.WriteAllLines(path, list);
        }

        public void m_InitAssetTable(string path) {
            var list = new List<string>();
            var assets = m_GetDefaultAssets();

            foreach (var assetId_path in assets) {
                var assetPath = Path.GetRelativePath(ProjectPath, assetId_path.Value);
                list.Add($"{assetId_path.Key}: {assetPath}");
            }
            File.WriteAllLines(path, list);
        }

        public void m_ReadTypes(string path) {
            var lines = File.ReadAllLines(path);

            foreach(var line in lines) {
                string id, value;
                if(YamlTable.ReadLine(line, out id, out value))
                    AddScriptId(id, value);
            }
        }

        public void m_ReadAssets(string path) {
            var lines = File.ReadAllLines(path);

            foreach (var line in lines) {
                string id, value;
                if (YamlTable.ReadLine(line, out id, out value))
                    AddAssetId(id, value);
            }
        }

        public void m_AddAssemblyTypes() {
            var assembly = Assembly.GetAssembly(typeof(AssetStore));
            var types = assembly.GetTypes();

            foreach (var type in types) {
                if (FireWriter.IsComponent(type) || FireWriter.IsAsset(type) || FireWriter.IsFile(type)) {
                    if (!HasTypeName(type.FullName))
                        AddScriptId(CreateAssetId(), type.FullName); //TODO: записать в файл
                }
            }
        }

        public void m_UpdateMissingAssets() {
            foreach(var pair in m_assetIdHash_assetPath) {
                var assetIdHash = pair.Key;
                var assetPath = pair.Value;

                var fullPath = Path.Combine(ProjectPath, assetPath);

                if(!File.Exists(fullPath)) {
                    Console.WriteLine($"Missing asset: {fullPath}");

                    m_missingAssetAIdHash.Add(assetIdHash);
                }
            };
        } 

        public void m_UpdateAssets(string path) {
            var fullPath = Path.GetFullPath(path);

            // Console.WriteLine(fullPath);

            foreach (string fileName in Directory.EnumerateFiles(fullPath, "*.yml")) {
                //Console.WriteLine(fileName);

                var name = Path.GetFileNameWithoutExtension(fileName);

                foreach (var line in File.ReadLines(fileName)) {
                    var values = new YamlValues().LoadFromText(line);

                    var assetId = values.GetValue(".file0.assetId", "");
                    if (assetId != "") {
                        var assetIdHash = assetId.GetHashCode();
                        var filePath = Path.GetRelativePath(ProjectPath, fileName);

                        if (!HasAsset(assetIdHash)) {
                            AddAssetId(assetId, filePath);
                            //TODO: записать в файл  
                            break;
                        }
                        var assetPath = GetAssetPath(assetId);
                        if (assetPath != filePath && !m_AssetExist(filePath, assetId)) {
                            ResetAssetPath(assetIdHash, filePath); 
                            //TODO: записать в файл  
                        }
                    }
                    break;
                }
            }

            foreach (string fileName in Directory.EnumerateDirectories(fullPath))
                m_UpdateAssets(fileName);
        }

        private bool m_AssetExist(string path, string expectedAssetId) {
            var fullPath = Path.GetFullPath(path);

            if(!File.Exists(fullPath))
                return false;

            foreach (var line in File.ReadLines(fullPath)) {
                var values = new YamlValues().LoadFromText(line);

                var assetId = values.GetValue(".file0.assetId", "");
                if (assetId != "")
                    return assetId == expectedAssetId;
                
                break;
            }
            return false;
        }

        private List<KeyValuePair<string, string>> m_GetDefaultTypes() {
            var list = new List<KeyValuePair<string, string>>();

            Func<string, string, KeyValuePair<string, string>> MakePair =
            (key, value) => new KeyValuePair<string, string>(key, value);

            list.Add(MakePair("D00000010000", typeof(Engine.Actor).FullName));
            list.Add(MakePair("D00000010003", typeof(Engine.MeshComponent).FullName));
            list.Add(MakePair("D00000010004", typeof(Engine.CameraComponent).FullName));
            list.Add(MakePair("D00000010005", typeof(Engine.TestMesh).FullName));
            list.Add(MakePair("D00000010006", typeof(UI.TestImGui).FullName));
            list.Add(MakePair("D00000010007", typeof(Engine.CSComponent).FullName));
            list.Add(MakePair("D00000010008", typeof(Engine.TestMeshBase).FullName));
            list.Add(MakePair("D00000010009", typeof(Engine.TestPrefab).FullName));
            list.Add(MakePair("D00000010010", typeof(Engine.Texture).FullName));
            list.Add(MakePair("D00000010011", typeof(Engine.Image).FullName));
            list.Add(MakePair("D00000010012", typeof(Engine.StaticMaterial).FullName));
            list.Add(MakePair("D00000010013", typeof(Engine.StaticMesh).FullName));
            list.Add(MakePair("D00000010014", typeof(Engine.Scene).FullName));
            list.Add(MakePair("D00000010015", typeof(Engine.Prefab).FullName));
            list.Add(MakePair("D00000010016", typeof(Engine.Component).FullName));
            list.Add(MakePair("D00000010017", typeof(Engine.EditorSettings).FullName));
            list.Add(MakePair("D00000010018", typeof(Engine.AmbientLight).FullName));
            list.Add(MakePair("D00000010019", typeof(Engine.DirectionalLight).FullName));
            list.Add(MakePair("D00000010020", typeof(Engine.PointLight).FullName));
            list.Add(MakePair("D00000010021", typeof(Engine.SpotLight).FullName));

            return list;
        }

        private List<KeyValuePair<string, string>> m_GetDefaultAssets() {
            var list = new List<KeyValuePair<string, string>>();

            Func<string, string, KeyValuePair<string, string>> MakePair =
            (key, value) => new KeyValuePair<string, string>(key, value);

            list.Add(MakePair("editor_settings", $"{EditorPath}/editor_settings.yml"));

            list.Add(MakePair("M_Default", $"{AssetsPath}/Materials/M_Default.yml"));
            list.Add(MakePair("M_WorldGride", $"{AssetsPath}/Materials/M_WorldGride.yml"));

            return list;
        }


        /// <param name="objRef">From object</param>
        /// <param name="typeIdHash">To type</param>
        /// <returns></returns>
        public static bool IsAssignable(CsRef objRef, int typeIdHash) {
            var obj = CppLinked.GetObjectByRef(objRef);
            if (obj == null)
                return false;

            var fullName = Instance.GetTypeFullName(typeIdHash);
            var type = Type.GetType(fullName);

            var result = type.IsAssignableFrom(obj.GetType());
            return result;
        }



    }
}
