using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace FireYaml {

    public class AssetStore {

        public static AssetStore Instance { get; set; }

        private Dictionary<string, string> tmp_keyValue = new Dictionary<string, string>();
        private Dictionary<string, string> tmp_valueKey = new Dictionary<string, string>();


        private Dictionary<string, YamlValues> tmp_assetValues = new Dictionary<string, YamlValues>();
        private Dictionary<string, string> tmp_assetPaths = new Dictionary<string, string>();

        private string m_assetsPath = @"C:\Users\Dmitry\Desktop\Пример\";


        public AssetStore(bool addDefaultPrefabs = true) {
            AddAsset("00000010000", typeof(Engine.Actor).FullName);
            AddAsset("00000010003", typeof(Engine.MeshComponent).FullName);
            AddAsset("00000010004", typeof(Engine.CameraComponent).FullName);
            AddAsset("00000010005", typeof(Engine.TestMesh).FullName);
            AddAsset("00000010006", typeof(UI.TestImGui).FullName);
            AddAsset("00000010007", typeof(Engine.CSComponent).FullName);


            if (addDefaultPrefabs) {
                AddAssetPath("P0000000000", $"{m_assetsPath}\\prefab_0.yml");
                AddAssetPath("P0000000001", $"{m_assetsPath}\\prefab_1.yml");
                AddAssetPath("P0000000002", $"{m_assetsPath}\\prefab_2.yml");
                AddAssetPath("P0000000003", $"{m_assetsPath}\\prefab_3.yml");
                AddAssetPath("P0000000004", $"{m_assetsPath}\\prefab_4.yml");
                AddAssetPath("P0000000005", $"{m_assetsPath}\\prefab_5.yml");
            }
        }

        public string GetTypeByAssetId(string assetId) => tmp_keyValue[assetId];

        public void AddAsset(string assetId, string value) {
            tmp_keyValue[assetId] = value;
            tmp_valueKey[value] = assetId;
        }

        public void AddAssetPath(string assetId, string value) {
            tmp_assetPaths[assetId] = value;
        }

        public bool TryGetAssetIdByType(string typeName, out string assetId) {
            assetId = "";
            if (tmp_valueKey.ContainsKey(typeName)) {
                assetId = tmp_valueKey[typeName];
                return true;
            }
            return false;
        }

        public bool HasAsset(string assetId) {
            return GetAssetPath(assetId) != "";
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

        public void LoadAsset(string prefabId) {
            if (tmp_assetValues.ContainsKey(prefabId))
                return;

            var path = GetAssetPath(prefabId);
            if (path == "")
                return;

            var text = File.ReadAllText(path);
            tmp_assetValues[prefabId] = new YamlValues().LoadFromText(text);
        }

        public string GetAssetPath(string prefabId) {
            if (tmp_assetPaths.ContainsKey(prefabId))
                return tmp_assetPaths[prefabId];
            
            return "";
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

        public void CreateAsset(string name, object data, string tmp_id) {
            var serializer = new Serializer(ignoreExistingIds: true, startId: 1);
            serializer.Serialize(data);

            if (!serializer.Result)
                throw new Exception($"Can not create asset: Serialization failed.");

            var values = serializer.Values;

            values.AddValue(".file0.assetId", new YamlValue(YamlValue.Type.AssetId, tmp_id));
            values.AddValue(".file0.files", new YamlValue(YamlValue.Type.Scalar, "0"));

            File.WriteAllText($"{m_assetsPath}\\{name}", values.ToSortedText());
        }

        public void UpdateAsset(string assetId, object data) {
            if (!HasAsset(assetId))
                throw new Exception($"Asset with assetId:{assetId} not exist");

            var assetInfo = GetAssetInfo(assetId);
            var assetPath = GetAssetPath(assetId);

            var serializer = new Serializer(ignoreExistingIds: false, writeNewIds: true, startId: assetInfo.files + 1);
            serializer.Serialize(data);

            if (!serializer.Result)
                throw new Exception($"Can not update asset: Serialization failed.");

            var values = serializer.Values;

            values.AddValue(".file0.assetId", new YamlValue(YamlValue.Type.AssetId, assetInfo.assetId));
            values.AddValue(".file0.files", new YamlValue(YamlValue.Type.Scalar, $"{serializer.FilesCount}"));

            File.WriteAllText(assetPath, values.ToSortedText());

            tmp_assetValues.Remove(assetId);
            LoadAsset(assetId);
        }

        public void UpdateAssetValues(string assetId) {
            if (!HasAsset(assetId))
                throw new Exception($"Asset with assetId:{assetId} not exist");

            var assetPath = GetAssetPath(assetId);
            var values = GetAssetValues(assetId);

            File.WriteAllText(assetPath, values.ToSortedText());

            tmp_assetValues.Remove(assetId);
            LoadAsset(assetId);
        }

    }
}
