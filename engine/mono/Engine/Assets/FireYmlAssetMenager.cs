using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;

using FireYaml;
using EngineDll;

namespace Engine {
    public class FireYamlAsset : Asset {
        public FireYaml.YamlValues data = null;
    }

    internal class FireYmlAssetMenager : IAssetMenager {

        private Dictionary<int, FireYamlAsset> m_assetIDHash_asset = new Dictionary<int, FireYamlAsset>();

        public string m_projectPath;
        public string assetsPath => $"{m_projectPath}\\Assets";
        public string engineAssetsPath => $"{m_projectPath}\\Engine\\Assets";
        public string editorPath => $"{m_projectPath}\\Editor";

        public bool createNewAssetsOnLoad = true;

        public static readonly string[] AssetExtentions = new string[] {
            ".yml",
        };

        public static readonly string[] SourceExtentions = new string[] {
            ".obj",
            ".png",
            ".jpg",
        };

        public static readonly string[] CreateExtentions = new string[] {
            ".create_scene",
            ".create_tex",
            ".create_mat",
            ".create_prefab",
        };

        public FireYmlAssetMenager() {

        }

        public IDeserializer GetDeserializer(int assetIdHash, bool writeIDs = true, bool useCsRefs = false) {
            var asset = GetAsset(assetIdHash) as FireYamlAsset;
            if(asset == null)
                throw new Exception($"FYML.GetDeserializer: Asset not exist.");

            LoadAssetData(assetIdHash);

            return new FireYaml.FireReader(asset.data, writeIDs: writeIDs, assetId: asset.assetID);
        }

        public Asset GetAsset(int assetIdHash) {
            if(m_assetIDHash_asset.ContainsKey(assetIdHash))
                return m_assetIDHash_asset[assetIdHash];

            return null;
        }

        public void ForgetAsset(int assetIdHash) {
            if (m_assetIDHash_asset.ContainsKey(assetIdHash))
                m_assetIDHash_asset.Remove(assetIdHash);
        }

        public void Load(string projectPath) {
            m_projectPath = projectPath;

            m_UpdateAssets(editorPath, in DateTime.UnixEpoch);
            m_UpdateAssets(assetsPath, in DateTime.UnixEpoch);
        }

        public IEnumerable<Asset> EnumerateAssets() {
            foreach (var key_value in m_assetIDHash_asset)
                yield return key_value.Value;
        }

        /// <summary>
        /// <return>Возвращает AssetIDHash либо 0, если ассет не был создан.</return>
        /// </summary>
        public int CreateNewAsset(Type type, string assetPath, string sourceExt = "") {
            var assetIDGuid = Guid.NewGuid();
            var assetID = assetIDGuid.ToString();
            var assetIDHash = assetID.GetAssetIDHash();

            var sourcePath = "";

            if (sourceExt != "") {
                sourcePath = Path.ChangeExtension(assetPath, sourceExt);
                if (!File.Exists(sourcePath)) {
                    Console.WriteLine($"FYML.Assets.Create: Asset source not found: {assetPath}");
                    return 0;
                }
            }
            if (File.Exists(assetPath)) {
                Console.WriteLine($"FYML.CreateNewAsset: Asset already exist: {assetPath}");
                return 0;
            }
            if (!GUIDAttribute.HasGuid(type)) {
                Console.WriteLine($"FYML.CreateNewAsset: Type hasn't a guid: {type.Name}");
                return 0;
            }
            var assetHeader = new string[] {
                ".file0.assetId: {AssetId: "+ assetID +"}",
                ".file0.files: {Var: 0}"
            };

            File.WriteAllLines(assetPath, assetHeader);
            m_WriteNewAsset(type, assetPath, assetID, sourcePath);

            var asset = GetAsset(assetIDHash);
            var AssetTypeId = GUIDAttribute.GetGuidHash(asset.scriptType);
            var assetName = Path.GetFileNameWithoutExtension(asset.path);
            Dll.AssetStore.AddAsset(Game.gameRef, AssetTypeId, asset.assetIDHash, assetName);

            Console.WriteLine($"FYML.CreatAsset: {asset.time}: '{assetPath}'");

            return asset.assetIDHash;
        }

        /// <summary>
        /// Записывает ассет с перезаписью, если файла ассета нет, создает новый ассет.
        /// </summary>
        public void WriteAsset(int assetIDHash, object valueObj, FireWriter writer = null) {
            var asset = GetAsset(assetIDHash);
            if (asset == null)
                throw new Exception($"FYML.WriteAsset: Asset with assetIdHash: '{assetIDHash}' not exist.");

            var fullPath = Path.GetFullPath(asset.path);
            var projectFullPath = Path.GetFullPath(m_projectPath);

            if (!fullPath.Contains(projectFullPath))
                throw new Exception($"FYML.WriteAsset: Asset Path not contains project path");

            if (!File.Exists(fullPath))
                throw new Exception($"FYML.WriteAsset: Asset not exist: '{fullPath}'");

            m_WriteAssetData(asset.assetID, valueObj, fullPath, writer);

            m_UpdateAssetData(valueObj.GetType(), fullPath);
            m_ReloadAssetData(assetIDHash);
        }

        private void m_WriteNewAsset(Type assetType, string assetPath, string assetGuid, string assetSourcePath = "") {
            var assetGuidHash = assetGuid.GetAssetIDHash();
            var fullPath = Path.GetFullPath(assetPath);
            var projectFullPath = Path.GetFullPath(m_projectPath);

            if (!fullPath.Contains(projectFullPath))
                throw new Exception($"FYML.m_WriteNewAsset: Asset Path not contains project path");

            if (!File.Exists(assetPath))
                throw new FileNotFoundException($"FYML.m_WriteNewAsset: {assetPath}");

            if (assetType == typeof(Engine.Actor)) {
                m_WriteNewPrefab(assetPath, assetGuid);
            }
            else {
                var valueObj = FireReader.CreateInstance(assetType);

                if (FireWriter.IsAsset(assetType))
                    FireReader.InitIAsset(ref valueObj, assetGuid, 0);

                if (FireWriter.IsAssetWithSource(assetType)) {
                    if (assetSourcePath == "")
                        throw new Exception("FYML.m_WriteNewAsset: Asset with source need a sourcePath");

                    var ext = Path.GetExtension(assetSourcePath);

                    FireReader.InitISourceAsset(ref valueObj, ext);
                }
                m_WriteAssetData(assetGuid, valueObj, fullPath);
            }
            m_UpdateAssetData(assetType, fullPath);
            m_ReloadAssetData(assetGuidHash);
        }

        private Asset m_UpdateAssetData(Type assetType, string assetPath) {
            var assetID = m_ReadAssetGuid(assetPath);
            var assetIDHash = assetID.GetAssetIDHash();
            var scriptID = GUIDAttribute.GetGuid(assetType);
            var scriptIDHash = scriptID.GetAssetIDHash();
            var sourcePath = "";

            if (FireWriter.IsAssetWithSource(assetType))
                sourcePath = Assets.FindSourceFile(assetType, assetPath);

            var prevAsset = GetAsset(assetIDHash) as FireYamlAsset;
            var asset = new FireYamlAsset();

            asset.path = assetPath;
            asset.name = Path.GetFileNameWithoutExtension(assetPath);
            asset.sourceExt = sourcePath != "" ? Path.GetExtension(sourcePath) : "";
            asset.time = File.GetLastWriteTime(assetPath);
            asset.assetID = assetID;
            asset.assetIDHash = assetIDHash;
            asset.scriptID = scriptID;
            asset.scriptIDHash = scriptIDHash;
            asset.scriptType = assetType;
            asset.data = prevAsset != null ? prevAsset.data : null;

            m_assetIDHash_asset[asset.assetIDHash] = asset;

            return asset;
        }

        public void LoadAssetData(int assetGuidHash) {
            var asset = GetAsset(assetGuidHash) as FireYamlAsset;
            if (asset == null || asset.data != null)
                return;

            var text = File.ReadAllText(asset.path);
            var values = new YamlValues().LoadFromText(text);

            asset.data = values;
        }

        public int CreateAssetFromSourceOrRequest(string filePath) {
            if (!createNewAssetsOnLoad)
                return 0;

            var assetFile = new AssetFile();

            assetFile.path = filePath;
            assetFile.ext = Path.GetExtension(filePath);
            assetFile.writeTime = File.GetLastWriteTime(filePath);

            if (CreateExtentions.Contains(assetFile.ext)) {
                assetFile.type = AssetFileType.CreateRequest;
                return m_CreateAssetFromRequest(assetFile);
            }
            if (SourceExtentions.Contains(assetFile.ext)) {
                assetFile.type = AssetFileType.Source;
                return m_CreateAssetFromSource(assetFile);
            }
            return 0;
        }

        private void m_ReloadAssetData(int assetGuidHash) {
            var asset = GetAsset(assetGuidHash) as FireYamlAsset;
            if (asset == null)
                return;

            asset.data = null;
            LoadAssetData(assetGuidHash);
        }

        private void m_WriteAssetData(string assetGuid, object valueObj, string fullPath, FireWriter writer = null) {
            var serializer = writer;
            if (serializer == null)
                serializer = new FireWriter(ignoreExistingIds: true, startId: 1);

            serializer.Serialize(valueObj);

            if (!serializer.Result)
                throw new Exception($"FYML.Can not create asset: Serialization failed.");

            var values = serializer.Mods;

            values.AddValue(".file0.assetId", new YamlValue(YamlValue.Type.AssetId, assetGuid));
            values.AddValue(".file0.files", new YamlValue(YamlValue.Type.Var, $"{serializer.FilesCount}"));

            File.WriteAllText(fullPath, values.ToSortedText());
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
                if (values.HasValue(scpiptPath))
                    return values.GetValue(scpiptPath, "");
            }
            return "";
        }

        private void m_UpdateAssetPath(string assetPath) {
            var assetID = m_ReadAssetGuid(assetPath);
            var scriptID = m_ReadScriptGuid(assetPath);

            Guid guid;
            if (!Guid.TryParse(assetID, out guid)) {
                Console.WriteLine($"FYML.m_UpdateAssetPath: Bad AssetID: '{assetID}' in path: '{assetPath}'");
                return;
            }
            if (!Guid.TryParse(scriptID, out guid)) {
                Console.WriteLine($"FYML.m_UpdateAssetPath: Bad asset ScriptId: '{scriptID}' in path: '{assetPath}'");
                return;
            }

            var sourcePath = "";
            var assetType = GUIDAttribute.GetTypeByGuid(scriptID); ;

            if (FireWriter.IsAssetWithSource(assetType)) {
                sourcePath = Assets.FindSourceFile(assetType, assetPath);
                if (sourcePath == "")
                    Console.WriteLine($"FYML.m_UpdateAssetPath: Asset source not found: {assetPath}");
            }
            var assetIDHash = assetID.GetAssetIDHash();
            var scriptIDHash = scriptID.GetAssetIDHash();

            var prevAsset = GetAsset(assetIDHash) as FireYamlAsset;
            var asset = new FireYamlAsset();

            asset.path = assetPath;
            asset.name = Path.GetFileNameWithoutExtension(assetPath);
            asset.sourceExt = sourcePath != "" ? Path.GetExtension(sourcePath) : "";
            asset.time = File.GetLastWriteTime(assetPath);
            asset.assetID = assetID;
            asset.assetIDHash = assetIDHash;
            asset.scriptID = scriptID;
            asset.scriptIDHash = scriptIDHash;
            asset.scriptType = assetType;
            asset.data = prevAsset != null ? prevAsset.data : null;

            if (asset.scriptType == null) {
                Console.WriteLine($"FYML.UpdateAssetPath: Misssing ScriptId: '{scriptID}' in path: '{assetPath}'");
                return;
            }

            m_assetIDHash_asset[asset.assetIDHash] = asset;

            Console.WriteLine($"FYML.UpdateAssetPath: {asset.time}: '{assetPath}'");
        }

        private static void m_WriteNewPrefab(string assetPath, string assetGuid) {

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

        private DateTime m_UpdateAssets(string path, in DateTime lastTime) {
            var dirPath = Path.GetFullPath(path);
            var newTime = lastTime;

            foreach (string filePath in Directory.EnumerateFiles(dirPath)) {
                var fileTime = File.GetLastWriteTime(filePath);

                if (fileTime > lastTime) {
                    var ext = Path.GetExtension(filePath);
                    bool watched = false;

                    if (AssetExtentions.Contains(ext)) {
                        m_UpdateAssetPath(filePath);
                        watched = true;
                    }
                    else if (CreateAssetFromSourceOrRequest(filePath) != 0) {
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

                    if (dirName > newTime)
                        newTime = dirName;
                }
            }
            return newTime;
        }

        /// <summary>
        /// Принимает файл с расширением типа 'filename.create_mesh' 
        /// и создает соответствующий ассет 'filename.yml'.
        /// Создает ассет, если файл не пустой и ассета с таким именем не существукт. 
        /// </summary>
        private int m_CreateAssetFromRequest(AssetFile requestFile) {
            if (requestFile.type != AssetFileType.CreateRequest)
                return 0;

            var length = new System.IO.FileInfo(requestFile.path).Length;
            if (length != 0)
                return 0;

            var assetPath = Path.ChangeExtension(requestFile.path, ".yml");
            if (File.Exists(assetPath))
                return 0;

            var type = Assets.GetAssetTypeByExt(assetPath);
            if (type == null)
                return 0;

            var assetIdHash = CreateNewAsset(type, assetPath);
            if (assetIdHash != 0)
                File.Delete(requestFile.path);

            return assetIdHash;
        }

        private int m_CreateAssetFromSource(AssetFile sourceFile) {
            if (sourceFile.type != AssetFileType.Source)
                return 0;

            var assetPath = Path.ChangeExtension(sourceFile.path, ".yml");
            if (File.Exists(assetPath))
                return 0;

            var type = Assets.GetAssetTypeByExt(sourceFile.ext);
            if (type == null)
                return 0;

            return CreateNewAsset(type, assetPath, sourceFile.ext);
        }
    }
}
