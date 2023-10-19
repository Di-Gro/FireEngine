using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;

using EngineDll;
using FireYaml;

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
        Other,
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

    public interface IDeserializer {

        T Instanciate<T>() where T : new();
        object Instanciate();
        void InstanciateTo(object target);
        void InstanciateToWithoutLoad(object target);
        Component InstanciateComponent(Actor actor);

    }

    public interface IAssetMenager {
        IDeserializer GetDeserializer(int assetIdHash, bool writeIDs = true, bool useCsRefs = false);
        Asset GetAsset(int assetIdHash);
        void ForgetAsset(int assetIdHash);
        void Load(string projectPath);
        IEnumerable<Asset> EnumerateAssets();
        int CreateNewAsset(Type type, string assetPath, string sourceExt = "");
        void WriteAsset(int assetIDHash, object valueObj, FireYaml.FireWriter writer = null);
        void LoadAssetData(int assetGuidHash);
        int CreateAssetFromSourceOrRequest(string filePath);
    }

    public class FireBinAssetMenager : IAssetMenager {
        private Dictionary<int, FireBinAsset> m_assetIDHash_asset = new Dictionary<int, FireBinAsset>();

        public string m_projectPath;
        public string assetsPath => $"{m_projectPath}\\Assets";
        public string engineAssetsPath => $"{m_projectPath}\\Engine\\Assets";
        public string editorPath => $"{m_projectPath}\\Editor";

        public static readonly string[] AssetExtentions = new string[] {
            ".prefab",
            ".scene",
            ".tex",
            ".mesh",
            ".image",
            ".mat",
            ".asset",
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

        public FireBinAssetMenager() {
           
        }

        public IDeserializer GetDeserializer(int assetIdHash, bool writeIDs = true, bool useCsRefs = false) {
            var asset = GetAsset(assetIdHash) as FireBinAsset;
            if (asset == null)
                throw new Exception($"GetDeserializer: Asset not exist.");

            LoadAssetData(assetIdHash);

            return new FireBin.Deserializer(asset.data, useCsRefs: useCsRefs, assetId: asset.assetID);
        }

        public Asset GetAsset(int assetIdHash) {
            if (m_assetIDHash_asset.ContainsKey(assetIdHash))
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

        public void ConvertFromYaml(object valueObj, Asset ymlAsset) {
            var assetExt = GetAssetExtByType(ymlAsset.scriptType);
            var assetPath = Path.ChangeExtension(ymlAsset.path, assetExt);
            var binAsset = new FireBinAsset();

            binAsset.path = assetPath;
            binAsset.name = ymlAsset.name;
            binAsset.sourceExt = ymlAsset.sourceExt;
            binAsset.time = ymlAsset.time;
            binAsset.assetID = ymlAsset.assetID;
            binAsset.assetIDHash = ymlAsset.assetIDHash;
            binAsset.scriptID = ymlAsset.scriptID;
            binAsset.scriptIDHash = ymlAsset.scriptIDHash;
            binAsset.scriptType = ymlAsset.scriptType;
            binAsset.data = null;

            m_assetIDHash_asset[binAsset.assetIDHash] = binAsset;

            File.WriteAllText(assetPath, "");
            WriteAsset(binAsset.assetIDHash, valueObj);
        }

        public void ConvertFromYaml(IAssetMenager fymlAM, string projectPath, Asset ymlAsset) {
            m_projectPath = projectPath;

            if (ymlAsset.scriptType == typeof(Scene) ||
                ymlAsset.scriptType == typeof(Actor))
                return;

            var valueObj = fymlAM.GetDeserializer(ymlAsset.assetIDHash).Instanciate();

            ConvertFromYaml(valueObj, ymlAsset);

            // var resObj = GetDeserializer(binAsset.assetIDHash).Instanciate();
        }

        public void ConvertFromYaml(IAssetMenager fymlAM, string projectPath) {
            foreach (var ymlAsset in fymlAM.EnumerateAssets()) {
                ConvertFromYaml(fymlAM, projectPath, ymlAsset);
            }
        }       

        public string GetAssetExtByType(Type type)
        {
            if (type == typeof(Actor)) return ".prefab";
            if (type == typeof(Scene)) return ".scene";
            if (type == typeof(Texture)) return ".tex";
            if (type == typeof(Mesh)) return ".mesh";
            if (type == typeof(Image)) return ".image";
            if (type == typeof(StaticMaterial)) return ".mat";
            return ".asset";
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
            var assetIDHash = assetID.GetAssetIDHash();

            var sourcePath = "";

            if (sourceExt != "") {
                sourcePath = Path.ChangeExtension(assetPath, sourceExt);
                if (!File.Exists(sourcePath)) {
                    Console.WriteLine($"FBIN.CreateNewAsset: Asset source not found: {assetPath}");
                    return 0;
                }
            }
            if (File.Exists(assetPath)) {
                Console.WriteLine($"FBIN.CreateNewAsset: Asset already exist: {assetPath}");
                return 0;
            }
            if (!GUIDAttribute.HasGuid(type)) {
                Console.WriteLine($"FBIN.CreateNewAsset: Type hasn't a guid: {type.Name}");
                return 0;
            }

            var assetData = m_CreateNewAssetData(type, assetPath, assetID, sourcePath);

            var assetHeader = new FireBin.AssetHeader() {
                assetID = assetID,
                scriptID = GUIDAttribute.GetGuid(type),
                sourceExt = sourceExt,
            };

            FileStream fileStream = null;
            try {
                fileStream = new FileStream(assetPath, FileMode.Create);
                var assetWriter = new FireBin.AssetWriter(fileStream);
                assetWriter.Write(assetHeader, assetData);
            } finally {
                if (fileStream != null)
                    fileStream.Close();
            }
            var asset = new FireBinAsset();

            asset.path = assetPath;
            asset.name = Path.GetFileNameWithoutExtension(assetPath);
            asset.sourceExt = assetHeader.sourceExt;
            asset.time = File.GetLastWriteTime(assetPath);
            asset.assetID = assetHeader.assetID;
            asset.assetIDHash = assetIDHash;
            asset.scriptID = assetHeader.scriptID;
            asset.scriptIDHash = assetHeader.scriptID.GetAssetIDHash();
            asset.scriptType = type;
            asset.data = assetData;

            m_assetIDHash_asset[asset.assetIDHash] = asset;

            Dll.AssetStore.AddAsset(Game.gameRef, asset.scriptIDHash, asset.assetIDHash, asset.name);
            
            Console.WriteLine($"FBIN.CreateNewAsset: {asset.time}: '{assetPath}'");

            return asset.assetIDHash;
        }

        /// <summary>
        /// Обновляет данные в существующем ассете.
        /// </summary>
        public void WriteAsset(int assetIDHash, object valueObj, FireYaml.FireWriter writer = null) {
            var asset = GetAsset(assetIDHash) as FireBinAsset;
            if (asset == null)
                throw new Exception($"FBIN.Asset with assetIDHash: '{assetIDHash}' not exist.");

            if (valueObj.GetType() != asset.scriptType)
                throw new Exception($"FBIN.Value type: '{valueObj.GetType().Name}' not equal to asset type: '{asset.scriptType.Name}'.");

            var fullPath = Path.GetFullPath(asset.path);
            var projectFullPath = Path.GetFullPath(m_projectPath);

            if (!fullPath.Contains(projectFullPath))
                throw new Exception($"FBIN.Asset Path not contains project path");

            if (!File.Exists(fullPath))
                throw new Exception($"FBIN.Asset file: '{fullPath}' not found.");

            asset.data = new FireBin.Data();
            /// TODO: Обновить asset.time

            new FireBin.Serializer(asset.data).Serialize(valueObj);

            var assetHeader = new FireBin.AssetHeader() {
                assetID = asset.assetID,
                scriptID = asset.scriptID,
                sourceExt = asset.sourceExt,
            };

            FileStream fileStream = null;
            try {
                fileStream = new FileStream(fullPath, FileMode.Truncate);
                new FireBin.AssetWriter(fileStream).Write(assetHeader, asset.data);
            } finally {
                if (fileStream != null)
                    fileStream.Close();
            }           
        }

        public int CreateAssetFromSourceOrRequest(string filePath) {
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
                        if (m_CreateAssetFromRequest(assetFile) != 0)
                            watched = true;
                    }
                    else if (assetFile.type == AssetFileType.Source) {
                        if (m_CreateAssetFromSource(assetFile) != 0)
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
        private int m_CreateAssetFromRequest(AssetFile requestFile) {
            if (requestFile.type != AssetFileType.CreateRequest)
                return 0;

            var length = new System.IO.FileInfo(requestFile.path).Length;
            if (length != 0)
                return 0;

            var assetExt = GetAssetExtByRequestExt(requestFile.ext);
            var assetPath = Path.ChangeExtension(requestFile.path, assetExt);
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

            var assetExt = Assets.GetAssetExtBySourceExt(sourceFile.ext);
            var assetPath = Path.ChangeExtension(sourceFile.path, assetExt);
            if (File.Exists(assetPath))
                return 0;

            var type = Assets.GetAssetTypeByExt(sourceFile.ext);
            if (type == null)
                return 0;

            return CreateNewAsset(type, assetPath, sourceFile.ext);
        }

        public void UpdataAssetData(Type type, string assetPath) {
            FireBin.AssetHeader assetHeader;

            using (var fileStream = new FileStream(assetPath, FileMode.Open))
                assetHeader = new FireBin.AssetReader(fileStream).ReadHeader();

            var assetIDHash = assetHeader.assetID.GetAssetIDHash();
            var prevAsset = GetAsset(assetIDHash) as FireBinAsset;

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

            m_assetIDHash_asset[asset.assetIDHash] = asset;
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
                        throw new Exception("FBIN.CreateNewAssetData: Asset with source need a sourcePath");

                    var ext = Path.GetExtension(assetSourcePath);

                    FireYaml.FireReader.InitISourceAsset(ref asset, ext);
                }
                serializer.AddAsNamedList(assetType, asset);
            }
            serializer.Serialize();
            return data;
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

        public void LoadAssetData(int assetGuidHash) {
            var asset = GetAsset(assetGuidHash) as FireBinAsset;
            if (asset == null || asset.data != null)
                return;

            var fileStream = new FileStream(asset.path, FileMode.Open);
            var assetReader = new FireBin.AssetReader(fileStream);

            asset.data = assetReader.ReadData();

            fileStream.Close();
        }

        private void m_UpdateAssetPath(AssetFile assetFile) {
            var fileStream = new FileStream(assetFile.path, FileMode.Open);
            var assetReader = new FireBin.AssetReader(fileStream);
            var assetHeader = assetReader.ReadHeader();
            fileStream.Close();

            var assetID = assetHeader.assetID;
            var scriptID = assetHeader.scriptID;
            Guid assetIDGuid;
            Guid scriptIDGuid;

            if (!Guid.TryParse(assetID, out assetIDGuid)) {
                Console.WriteLine($"FBIN.UpdateAssetPath: Bad AssetID: '{assetID}' in path: '{assetFile.path}'");
                return;
            }
            if (!Guid.TryParse(scriptID, out scriptIDGuid)) {
                Console.WriteLine($"FBIN.UpdateAssetPath: Bad asset ScriptId: '{scriptID}' in path: '{assetFile.path}'");
                return;
            }
            var assetType = GUIDAttribute.GetTypeByGuid(scriptID);
            if(assetType == null) {
                Console.WriteLine($"FBIN.UpdateAssetPath: Misssing ScriptId: '{scriptID}' in path: '{assetFile.path}'");
                return;
            }
            if (assetHeader.sourceExt != "") {
                if(Assets.FindSourceFile(assetType, assetFile.path) == "")
                    Console.WriteLine($"FBIN.UpdateAssetPath: Asset source: '{assetHeader.sourceExt}' not found: '{assetFile.path}'");
            }
            var assetIDHash = assetID.GetAssetIDHash();
            var scriptIDHash = scriptID.GetAssetIDHash();

            var prevAsset = GetAsset(assetIDHash) as FireBinAsset;
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

            m_assetIDHash_asset[asset.assetIDHash] = asset;

            Console.WriteLine($"FBIN.UpdateAssetPath: {asset.time}: '{assetFile.path}'");
        }
    }
}
