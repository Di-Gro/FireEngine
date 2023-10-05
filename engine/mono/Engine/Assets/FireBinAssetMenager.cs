using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;

using EngineDll;

namespace Engine {

    public class Asset {
        public string path;
        public string name;
        public string sourceExt = "";

        public DateTime time;

        public string assetID;
        public int assetIDHash;

        public string scriptID;
        public int scriptIDHash;
        public Type scriptType;
    }

    public class FireBinAsset : Asset {
        public FireBin.Data data = null;
    }

    

    public enum AssetFileType {
        CreateRequest,
        Source,
        Asset,
    }

    public struct AssetFile {
        public string ext;
        public string path;
        public DateTime writeTime;
        public AssetFileType type;
    }

    //public interface IAssetManager {
    //    void Load();
    //    Asset GetAsset(int assetIdHash);
    //    int CreateNewAsset(Type type, string assetPath, string sourceExt = "");
    //    void WriteAsset(int assetIDHash, object valueObj);
    //}

    public class FireBinAssetMenager {
        private Dictionary<int, FireBinAsset> m_asets = new Dictionary<int, FireBinAsset>();

        public string m_projectPath;
        public string assetsPath => $"{m_projectPath}/Assets";
        public string engineAssetsPath => $"{m_projectPath}/Engine/Assets";
        public string editorPath => $"{m_projectPath}/Editor";

        public FireBinAssetMenager(string projectPath) {
            m_projectPath = projectPath;
        }

        public void Load() {
            m_UpdateAssets(editorPath, in DateTime.UnixEpoch);
            m_UpdateAssets(assetsPath, in DateTime.UnixEpoch);
        }

        public FireBinAsset GetAsset(int assetIdHash) {
            if (m_asets.ContainsKey(assetIdHash))
                return m_asets[assetIdHash];

            return null;
        }
        
        public static readonly string[] AssetExtentions = new string[] {
            ".scene",
            ".tex",
            ".mat",
            ".prefab",
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

        public string GetAssetExtByRequestExt(string requestExt) {
            switch (requestExt) {
                case ".create_scene": return ".scene";
                case ".create_tex": return ".tex";
                case ".create_mat": return ".mat";
                case ".create_prefab": return ".prefab";
            }
            throw new Exception($"Unknown create request extention: '{requestExt}'.");
        }

        public IEnumerable<AssetFile> EnumerateAssetFiles(string dirPath) {
            foreach (string filePath in Directory.EnumerateFiles(dirPath)) {
                var assetFile = new AssetFile();

                assetFile.path = filePath;
                assetFile.ext = Path.GetExtension(filePath);
                assetFile.writeTime = File.GetLastWriteTime(filePath);

                if (AssetExtentions.Contains(assetFile.ext))
                    assetFile.type = AssetFileType.Asset;

                else if (SourceExtentions.Contains(assetFile.ext))
                    assetFile.type = AssetFileType.Source;

                else if (CreateExtentions.Contains(assetFile.ext))
                    assetFile.type = AssetFileType.CreateRequest;

                yield return assetFile;
            }
            foreach (string nextDirPath in Directory.EnumerateDirectories(dirPath)) {
                var name = Path.GetFileName(nextDirPath);
                if (name != "Ignore") {
                    foreach (var assetFile in EnumerateAssetFiles(nextDirPath)) {
                        yield return assetFile;
                    }
                }
            }
        }

        private DateTime m_UpdateAssets(string path, in DateTime lastTime) {
            var dirPath = Path.GetFullPath(path);
            var newTime = lastTime;

            foreach (var assetFile in EnumerateAssetFiles(dirPath)) {
                if (assetFile.writeTime > lastTime) {
                    bool watched = false;

                    if (assetFile.type == AssetFileType.Asset) {
                        m_UpdateAssetPath(assetFile);
                        watched = true;
                    }
                    else if (assetFile.type == AssetFileType.CreateRequest) {
                        if (m_CreateAssetFromRequest(assetFile))
                            watched = true;
                    }
                    else if (assetFile.type == AssetFileType.Source) {
                        if (m_CreateAssetFromSource(assetFile))
                            watched = true;
                    }
                    if (watched && assetFile.writeTime > newTime)
                        newTime = assetFile.writeTime;
                }
            }
            return newTime;
        }

        /// <summary>
        /// Принимает файл с расширением 'filename.create_mesh' 
        /// и создает соответствующий ассет 'filename.mesh'.
        /// Создает ассет, если файл не пустой и ассета с таким именем не существукт. 
        /// </summary>
        private bool m_CreateAssetFromRequest(AssetFile assetFile) {
            if (assetFile.type != AssetFileType.CreateRequest)
                return false;

            var length = new System.IO.FileInfo(assetFile.path).Length;
            if (length != 0)
                return false;

            var assetExt = GetAssetExtByRequestExt(assetFile.ext);
            var assetPath = Path.ChangeExtension(assetFile.path, assetExt);
            if (File.Exists(assetPath))
                return false;

            var type = Assets.GetAssetTypeByExt(assetPath);
            if (type == null)
                return false;

            File.Move(assetFile.path, assetPath);

            CreateNewAsset(type, assetPath);

            return true;
        }
        
        private bool m_CreateAssetFromSource(AssetFile assetFile) {
            if (assetFile.type != AssetFileType.Source)
                return false;

            var assetExt = Assets.GetAssetExtBySourceExt(assetFile.ext);
            var assetPath = Path.ChangeExtension(assetFile.path, assetExt);
            if (File.Exists(assetPath))
                return false;

            var type = Assets.GetAssetTypeByExt(assetFile.ext);
            if (type == null)
                return false;

            File.WriteAllText(assetPath, "");

            CreateNewAsset(type, assetPath, assetFile.ext);

            return true;
        }

        /// <summary>
        /// Создает ассет указанного типа, с данными по умолчанию. 
        /// Ассет не должен существовать. 
        /// Actor и Scene будут созданы с пустыми данными.
        /// В IAsset будет установлен assetId. 
        /// В ISourceAsseet будет установлено sourceExt.
        /// Остальные типы должны иметь конструктор по умолчанию и GUID. 
        /// <return>Возвращает AssetIDHash</return>
        /// </summary>
        public int CreateNewAsset(Type type, string assetPath, string sourceExt = "") {
            var assetIDGuid = Guid.NewGuid();
            var assetID = assetIDGuid.ToString();
            var assetIDHash = assetIDGuid.GetHashCode();

            var sourcePath = "";

            if (sourceExt != "") {
                sourcePath = Path.ChangeExtension(assetPath, sourceExt);
                if (!File.Exists(sourcePath)) {
                    Console.WriteLine($"CreateNewAsset: Asset source not found: {assetPath}");
                    return 0;
                }
            }
            if (File.Exists(assetPath)) {
                Console.WriteLine($"CreateNewAsset: Asset already exist: {assetPath}");
                return 0;
            }
            if (!GUIDAttribute.HasGuid(type)) {
                Console.WriteLine($"CreateNewAsset: Type hasn't a guid: {type.Name}");
                return 0;
            }

            var assetData = m_CreateNewAssetData(type, assetPath, assetID, sourcePath);

            var assetHeader = new FireBin.AssetHeader() {
                assetID = assetID,
                scriptID = GUIDAttribute.GetGuid(type),
                sourceExt = sourceExt,
            };

            using (var fileStream = new FileStream(assetPath, FileMode.Create)) {
                var assetWriter = new FireBin.AssetWriter(fileStream);
                assetWriter.Write(assetHeader, assetData);
            }
            var asset = new FireBinAsset();

            asset.path = assetPath;
            asset.name = Path.GetFileNameWithoutExtension(assetPath);
            asset.sourceExt = assetHeader.sourceExt;
            asset.time = File.GetLastWriteTime(assetPath);
            asset.assetID = assetHeader.assetID;
            asset.assetIDHash = assetIDHash;
            asset.scriptID = assetHeader.scriptID;
            asset.scriptIDHash = Guid.Parse(assetHeader.scriptID).GetHashCode();
            asset.scriptType = type;
            asset.data = assetData;

            m_asets[asset.assetIDHash] = asset;

            Dll.AssetStore.AddAsset(Game.gameRef, asset.scriptIDHash, asset.assetIDHash, asset.name);

            // AssetStore.Instance.ReloadAssetValues(assetID);

            Console.WriteLine($"CreateNewAsset: {asset.time}: '{assetPath}'");

            return asset.assetIDHash;
        }

        public void UpdataAssetData(Type type, string assetPath) {
            FireBin.AssetHeader assetHeader;

            using (var fileStream = new FileStream(assetPath, FileMode.Open))
                assetHeader = new FireBin.AssetReader(fileStream).ReadHeader();

            var assetIDHash = assetHeader.assetID.GetAssetIDHash();
            var prevAsset = GetAsset(assetIDHash);

            var asset = new FireBinAsset() {
                path = assetPath,
                name = Path.GetFileNameWithoutExtension(assetPath),
                sourceExt = assetHeader.sourceExt,
                time = File.GetLastWriteTime(assetPath),
                assetID = assetHeader.assetID,
                assetIDHash = assetIDHash,
                scriptID = assetHeader.scriptID,
                scriptIDHash = assetHeader.scriptID.GetAssetIDHash(),
                scriptType = type,
                data = prevAsset != null ? prevAsset.data : null,
            };

            m_asets[asset.assetIDHash] = asset;
        }

        private FireBin.Data m_CreateNewAssetData(Type assetType, string assetPath, string assetID, string assetSourcePath = "") {
            var data = new FireBin.Data();
            var serializer = new FireBin.Serializer(data);

            if (assetType == typeof(Engine.Actor) ||
                assetType == typeof(Engine.Scene)) 
            {
                serializer.AddAsNamedList(assetType);
            }
            else {
                var asset = FireBin.Deserializer.CreateInstance(assetType);

                if (FireYaml.FireWriter.IsAsset(assetType))
                    FireYaml.FireReader.InitIAsset(ref asset, assetID, 0);

                if (FireYaml.FireWriter.IsAssetWithSource(assetType)) {
                    if (assetSourcePath == "")
                        throw new Exception("CreateNewAssetData: Asset with source need a sourcePath");

                    var ext = Path.GetExtension(assetSourcePath);

                    FireYaml.FireReader.InitISourceAsset(ref asset, ext);
                }
                serializer.AddAsNamedList(assetType, asset);
            }
            serializer.Serialize();
            return data;
        }

        /// TODO: переименовать в UpdateAsset
        /// <summary>
        /// Обновляет данные в существующем ассете.
        /// </summary>
        public void WriteAsset(int assetIDHash, object valueObj) {
           var asset = GetAsset(assetIDHash);
           if (asset == null)
               throw new Exception($"Asset with assetIDHash: '{assetIDHash}' not exist.");

           if(valueObj.GetType() != asset.scriptType)
               throw new Exception($"Value type: '{valueObj.GetType().Name}' not equal to asset type: '{asset.scriptType.Name}'.");

           var fullPath = Path.GetFullPath(asset.path);
           var projectFullPath = Path.GetFullPath(m_projectPath);

           if (!fullPath.Contains(projectFullPath))
               throw new Exception($"Asset Path not contains project path");

           if (!File.Exists(fullPath))
               throw new Exception($"Asset file: '{fullPath}' not found.");

           asset.data = new FireBin.Data();
           /// TODO: Обновить asset.time

           new FireBin.Serializer(asset.data).Serialize(valueObj);

           var assetHeader = new FireBin.AssetHeader() {
               assetID = asset.assetID,
               scriptID = asset.scriptID,
               sourceExt = asset.sourceExt,
           };

           using (var fileStream = new FileStream(fullPath, FileMode.Open))
               new FireBin.AssetWriter(fileStream).Write(assetHeader, asset.data);
        }

        public int GetAssetIdHashFromFile(string assetPath) {
            if (!File.Exists(assetPath))
                return 0;

            var fileStream = new FileStream(assetPath, FileMode.Open);
            var assetReader = new FireBin.AssetReader(fileStream);
            var assetHeader = assetReader.ReadHeader();

            var assetIDHash = assetHeader.assetID.GetAssetIDHash();
            return assetIDHash;
        }

        private void m_UpdateAssetPath(AssetFile assetFile) {
            var fileStream = new FileStream(assetFile.path, FileMode.Open);
            var assetReader = new FireBin.AssetReader(fileStream);

            var assetHeader = assetReader.ReadHeader();

            var assetID = assetHeader.assetID;
            var scriptID = assetHeader.scriptID;
            Guid assetIDGuid;
            Guid scriptIDGuid;

            if (!Guid.TryParse(assetID, out assetIDGuid)) {
                Console.WriteLine($"FireDB.UpdateAssetPath: Bad AssetID: '{assetID}' in path: '{assetFile.path}'");
                return;
            }
            if (!Guid.TryParse(scriptID, out scriptIDGuid)) {
                Console.WriteLine($"FireDB.UpdateAssetPath: Bad asset ScriptId: '{scriptID}' in path: '{assetFile.path}'");
                return;
            }
            var assetType = GUIDAttribute.GetTypeByGuid(scriptID);
            if(assetType == null) {
                Console.WriteLine($"FireDB.UpdateAssetPath: Misssing ScriptId: '{scriptID}' in path: '{assetFile.path}'");
                return;
            }
            if (assetHeader.sourceExt != "") {
                if(Assets.FindSourceFile(assetType, assetFile.path) == "")
                    Console.WriteLine($"FireDB.UpdateAssetPath: Asset source: '{assetHeader.sourceExt}' not found: '{assetFile.path}'");
            }
            var assetIDHash = assetID.GetAssetIDHash();
            var scriptIDHash = scriptID.GetAssetIDHash();

            var prevAsset = GetAsset(assetIDHash);
            var asset = new FireBinAsset();

            asset.path = assetFile.path;
            asset.name = Path.GetFileNameWithoutExtension(assetFile.path);
            asset.sourceExt = assetHeader.sourceExt;
            asset.time = File.GetLastWriteTime(assetFile.path);
            asset.assetID = assetID;
            asset.assetIDHash = assetIDHash;
            asset.scriptID = scriptID;
            asset.scriptIDHash = scriptIDHash;
            asset.scriptType = assetType;
            asset.data = prevAsset != null ? prevAsset.data : null;

            m_asets[asset.assetIDHash] = asset;

            Console.WriteLine($"FireDB.UpdateAssetPath: {asset.time}: '{assetFile.path}'");
        }
    }
}
