using System;
using YamlDotNet.Serialization;
using YamlDotNet.Serialization.NamingConventions;
using YamlDotNet.RepresentationModel;
using System.Numerics;
using System.Collections;
using System.Collections.Generic;
using System.Reflection;
using System.Globalization;
using System.Diagnostics;
using System.Linq;


namespace FireYaml {

    public class FireReader {
        public event Action EndLoadEvent;

        private class InnerLink {
            // Ссылка на документ
            public string fileName = "";

            // Поле в котором находится внутренняя ссылка
            public Field field;
        }

        private YamlValues m_values;
        //private YamlValues? m_mods = null;
        private string m_rootPath = "";
        private string m_assetId = "";
        private ulong m_assetInst = 0;

        private List<IAsset> m_assets = new List<IAsset>();
        private List<InnerLink> m_links = new List<InnerLink>();

        private Dictionary<string, object> m_loadedDocs = new Dictionary<string, object>();

        BindingFlags m_flags =
                BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic |
                BindingFlags.GetField | BindingFlags.SetField | BindingFlags.GetProperty |
                BindingFlags.SetProperty;

        public bool showLog = true;

        public FireReader(YamlValues values) {
            m_values = values;
        }

        public FireReader(string assetId) {
            m_assetId = assetId;
            m_values = AssetStore.Instance.ThrowAssetValues(m_assetId);
        }

        public YamlValues GetField(string fieldPath) => m_values.GetObject(fieldPath);

        public T Instanciate<T>() where T : new() {
            return (T)Instanciate();
        }

        public object Instanciate() {
            var yamlRef = new YamlRef(1);

            var hasFile = m_values.HasValue($".{yamlRef.Name}!scriptId");
            if (!hasFile)
                throw new Exception("Prefab must contains root file 'file1'");

            var selfPath = GetFullPath(yamlRef.Name);
            var target = LoadDocument(selfPath, isEntryPoint: true);

            m_EndInstanciate();
            return target;
        }

        // public object InstanciateTo(ref object target) {
        //     var yamlRef = new YamlRef(1);

        //     var hasFile = m_values.HasValue($".{yamlRef.Name}!scriptId");
        //     if (!hasFile)
        //         throw new Exception("Prefab must contains root file 'file1'");

        //     var selfPath = GetFullPath(yamlRef.Name);
        //     LoadDocument(selfPath, ref target, isEntryPoint: true);

        //     m_EndInstanciate();
        //     return target;
        // }

        public void InstanciateIAssetAsFile(IAsset target) {
            var yamlRef = new YamlRef(1);

            var hasFile = m_values.HasValue($".{yamlRef.Name}!scriptId");
            if (!hasFile)
                throw new Exception("Prefab must contains root file 'file1'");

            var selfPath = GetFullPath(yamlRef.Name);
            object self = target;
            LoadDocument(selfPath, ref self, isEntryPoint: true, isIAssetAsFile: true);

            m_EndInstanciate();
        }

        // public List<T> InstanciateAll<T>() where T: new() {
        //     var objects = new List<T>();
        //     var type = typeof(T);

        //     var files = m_values.GetFiles();
        //     foreach (var file in files) {
        //         if (file != "file0" && type == GetTypeOf($".{file}"))
        //             objects.Add((T)LoadDocument(file));
        //     }
        //     m_EndInstanciate();
        //     return objects;
        // }

        private void m_EndInstanciate() {
            m_ResolveLinks();
            foreach (var asset in m_assets)
                asset.LoadAsset();

            EndLoadEvent?.Invoke();
        }

        public object? LoadDocument(string fullPath, bool isEntryPoint = false) {
            if (!HasFile(fullPath))
                return null;
                        
            var type = GetTypeOf(fullPath);
            var target = CreateInstance(type);

            LoadDocument(fullPath, ref target, isEntryPoint);
            
            return target;
        }

        public void LoadDocument(string fullPath, ref object target, bool isEntryPoint = false, bool isIAssetAsFile = false) {
            if (!HasFile(fullPath))
                return;

            if (target.GetType().GetInterface(nameof(IFile)) == null)
               throw new Exception("Target must be assignable to IFile");

            if (m_assetInst == 0)
                m_assetInst = IFile.GetAssetInstance(ref target);

            var splited = YamlValues.SplitPath(fullPath);
            var fileName = splited.Last().Replace(".", "");

            var type = GetTypeOf(fullPath);
            var fileId = new YamlRef(fileName).Index;
            var prefabId = m_values.GetValue($"{fullPath}!prefabId", IFile.NotPrefab);

            m_SetAssignedObject(fullPath, ref target);

            IFile.SetAssetInstance(m_assetInst, ref target);
            IFile.SetFileId(fileId, ref target);

            if(!isIAssetAsFile) {
                if(isEntryPoint &&  FireWriter.IsAsset(type)) {
                    m_LoadAsset(fullPath, m_assetId, target);
                    return;
                }
            }

            IFile.SetPrefabId(prefabId, ref target);  

            if (prefabId != IFile.NotPrefab)
                m_LoadPrefab(fullPath, type, ref target, prefabId);
            else
                m_LoadObject(fullPath, type, ref target, isIAssetAsFile);

            if (isEntryPoint && m_assetId != "")
                IFile.SetPrefabId(m_assetId, ref target);
        }

        public void LoadAsset<TAsset>(string fieldPath, TAsset asset) where TAsset: IAsset {
            if (!m_values.HasValue(fieldPath))
                return;

            var yamlValue = m_values.GetValue(fieldPath);

            m_LoadAsset(fieldPath, yamlValue.value, asset);
        }

        private void m_LoadAsset(string path, string assetId, object asset) {
            var fieldName = nameof(IAsset.assetId);
            var serializer = new Engine.SerializerBase();
            var field = FireWriter.GetField(fieldName, asset.GetType(), asset, serializer);

            var value = m_Convert(field.type, assetId);

            m_assets.Add((IAsset)field.Instance);
            field.SetValue(value);
        }


        /// <summary>
        /// Инициализирует IAsset независимо от того, есть ассет в AssetStore или нет.
        /// </summary>
        /// <param name="asset">Ссылка на объект asset-а</param>
        /// <param name="assetId">ID asset-а</param>
        /// <param name="cppRef">Ссылка на Cpp-объект, нужна только в том случае, 
        /// если asset загружается как файл для asset-а созданного в C++</param>
        public static void InitIAsset(ref object asset, string assetId, Engine.CppRef cppRef) {
            var flags =
               BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic |
               BindingFlags.GetField | BindingFlags.SetField | BindingFlags.GetProperty |
               BindingFlags.SetProperty;

            var type = asset.GetType();

            var idProp = type.GetProperty(nameof(IAsset.assetId), flags);
            var hashProp = type.GetProperty(nameof(IAsset.assetIdHash), flags);
            var refProp = type.GetProperty(nameof(IAsset.cppRef), flags);

            idProp.SetValue(asset, assetId);
            hashProp.SetValue(asset, assetId.GetHashCode());
            refProp.SetValue(asset, cppRef);
        }

        private void m_SetAssignedObject(string fullPath, ref object obj) {
            m_loadedDocs[fullPath] = obj;
        }

        private object m_GetAssignedObject(string fullPath) {
            if (m_loadedDocs.ContainsKey(fullPath))
                return m_loadedDocs[fullPath];
            return null;
        }

        public bool HasFile(string fullPath) {
            return m_values.HasValue($"{fullPath}!scriptId");
            //return m_values.HasValue($".{fileName}!scriptId");
        }

        public Type GetTypeOf(string documentPath) {
            var scriptId = "";

            var scriptPath = $"{documentPath}!scriptId";
            if (m_values.HasValue(scriptPath))
                scriptId = m_values.GetValue(scriptPath).value;

            if (scriptId == "")
                throw new Exception("Yaml document not contains scriptId");

            var typeName = AssetStore.Instance.GetTypeFullName(scriptId);
            if (typeName == null)
                throw new Exception("Component script asset not found");

            var type = Type.GetType(typeName);
            if (type == null)
                throw new Exception("Component type not exists");

            return type;
        }

        private void m_LoadObject(string selfPath, Type type, ref object instance, bool isIAssetAsFile = false) {

            var serializer = FireWriter.GetSerializer(type);
            var fields = FireWriter.GetFields(type, instance, serializer);

            if(serializer.NeedIncludeBase(type))
                m_LoadBaseObject(selfPath, ref instance);

            foreach (var field in fields) {
                var fieldPath = $"{selfPath}.{field.name}";
                m_LoadField(fieldPath, field, isIAssetAsFile);
            }
            serializer.OnDeserialize(this, selfPath, type, ref instance);
        }

        private void m_LoadPrefab(string selfPath, Type type, ref object instance, string prefabId) {

            /// Запоминаем старое состояние
            var last_m_assetId = m_assetId;
            var last_rootPath = m_rootPath;
            var last_m_assetInst = m_assetInst;
            var last_fileId = IFile.GetFileId(ref instance);
            var last_prefabId = IFile.GetPrefabId(ref instance);

            /// Устанавливаем новое состояние
            m_assetId = prefabId;
            m_rootPath = $"{selfPath}!";
            m_assetInst = IFile.GetAssetInstance(ref instance);

            var values = AssetStore.Instance.ThrowAssetValues(prefabId);
            m_values = YamlValues.Merge(values, m_values, m_rootPath, true);
            
            /// Внутри нового состояния
            var entryPoint = GetFullPath("file1");
            LoadDocument(entryPoint, ref instance, true);

            /// Восстанавливаем старое состояние
            m_assetId = last_m_assetId;
            m_rootPath = last_rootPath;
            m_assetInst = last_m_assetInst;
            IFile.SetFileId(last_fileId, ref instance);
            IFile.SetPrefabId(last_prefabId, ref instance);
        }

        private void m_LoadField(string fieldPath, Field field, bool isIAssetAsFile = false) {
            if (m_values == null)
                return;

            var isList = FireWriter.IsList(field.type);

            /// Поле записано.
            /// Либо простой тип, либо объект равный нулю, либо ссылка.
            if (m_values.HasValue(fieldPath)) {
                var yamlValue = m_values.GetValue(fieldPath);
                object? value = null;

                if (yamlValue.type == YamlValue.Type.Var)
                    value = m_Convert(field.type, yamlValue.value);

                if (yamlValue.type == YamlValue.Type.Ref) {
                    var link = new InnerLink();

                    link.fileName = yamlValue.value; // $"{m_rootPath}.{yamlValue.value}";
                    link.field = field;

                    m_links.Add(link);
                }
                if (yamlValue.type == YamlValue.Type.AssetId) {
                    value = m_Convert(field.type, yamlValue.value);

                    if (!isIAssetAsFile)
                        m_assets.Add((IAsset)field.Instance);
                }
                if (yamlValue.type == YamlValue.Type.Null) {
                    if (isList) {
                        m_InitListInField(field);
                        return;
                    }
                }
                field.SetValue(value);
                return;
            }

            /// Есть дочерний путь.
            /// Объект или список не равные нулю.
            var hasChildren = m_values.HasChildren(fieldPath);
            if (hasChildren) {
                /// Список
                if (isList)
                    m_LoadList(fieldPath, field);

                /// Объект, для которого есть поля, значит можно инстанцировать.
                else {
                    if (field.Value == null)
                        field.SetValue(CreateInstance(field.type));

                    var fieldValue = field.Value;
                    m_LoadObject(fieldPath, field.type, ref fieldValue);
                    if (field.type.IsValueType)
                        field.SetValue(fieldValue);
                }
                return;
            }

            /// Нет ни пути, ни дочерних путей.
            /// Ничего не делаем - оставляем значение по умолчанию.
            return;
        }

        private void m_LoadList(string listPath, Field field) {
            var itemType = field.type.GetGenericArguments()[0];
            var list = m_InitListInField(field);
            var items = m_values.GetObject(listPath);
            var count = items.GetItemsCount(listPath);

            for (int i = 0; i < count; i++) {
                var itemPath = $"{listPath}.{i}";
                var hasValue = items.HasValue(itemPath);
                var hasChildren = items.HasChildren(itemPath);

                if (hasValue && !hasChildren) {
                    var itemValue = items.GetValue(itemPath);
                    if (itemValue.type == YamlValue.Type.Null) {
                        list.Add(null);
                        continue;
                    }
                }
                list.Add(CreateInstance(itemType));
                m_LoadField(itemPath, new Field(list, itemType, i));
            }
        }

        private IList m_InitListInField(Field field) {
            IList list = null;

            if (field.Value != null) {
                list = field.Value as IList;
            }
            else {
                list = CreateInstance(field.type) as IList;
                field.SetValue(list);
            }

            list.Clear();
            return list;
        }

        private void m_LoadBaseObject(string selfPath, ref object target) {

            var baseScriptPath = $"{selfPath}!base!scriptId";
            if (!m_values.HasValue(baseScriptPath))
                return;

            var basePath = $"{selfPath}!base";
            var baseType = GetTypeOf(basePath);

            m_LoadObject(basePath, baseType, ref target);
        }

        public static object CreateInstance(Type type) {
            var instance = Activator.CreateInstance(type, true);
            if (instance == null)
                throw new Exception("Parameterless constructor needed to load object.");

            return instance;
        }

        private void m_ResolveLinks() {
            foreach (var link in m_links) {
                var objectRef = m_GetAssignedObject($".{link.fileName}");
                link.field.SetValue(objectRef);
            }
        }

        private object m_Convert(Type targetType, string value) {
            if (targetType == typeof(float) || targetType == typeof(double))
                return System.Convert.ChangeType(value, targetType, CultureInfo.InvariantCulture);

            if (targetType.IsEnum)
                return Enum.Parse(targetType, value);

            return System.Convert.ChangeType(value, targetType);
        }

        public string GetFullPath(string fileName) {
            return $"{m_rootPath}.{fileName}";
        }
    }

    }
