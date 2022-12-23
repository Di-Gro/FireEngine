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

    public class Deserializer {

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

        public Deserializer(YamlValues values) {
            m_values = values;
        }

        public Deserializer(string assetId) {
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
            var target = LoadDocument(selfPath, true);

            m_EndInstanciate();
            return target;
        }

        public List<T> InstanciateAll<T>() where T: new() {
            var objects = new List<T>();
            var type = typeof(T);

            var files = m_values.GetFiles();
            foreach (var file in files) {
                if (file != "file0" && type == GetTypeOf($".{file}"))
                    objects.Add((T)LoadDocument(file));
            }
            m_EndInstanciate();
            return objects;
        }

        private void m_EndInstanciate() {
            m_ResolveLinks();
            foreach (var asset in m_assets)
                asset.LoadAsset();
        }

        public object? LoadDocument(string fullPath, bool isEntryPoint = false) {
            if (!HasFile(fullPath))
                return null;
                        
            var type = GetTypeOf(fullPath);
            var target = m_CreateInstance(type);

            LoadDocument(fullPath, ref target, isEntryPoint);
            
            return target;
        }

        public void LoadDocument(string fullPath, ref object target, bool isEntryPoint = false) {
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
            IFile.SetPrefabId(prefabId, ref target);  

            if (prefabId != IFile.NotPrefab)
                m_LoadPrefab(fullPath, type, ref target, prefabId);
            else
                m_LoadObject(fullPath, type, ref target);

            if (isEntryPoint && m_assetId != "")
                IFile.SetPrefabId(m_assetId, ref target);
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

            var typeName = AssetStore.Instance.GetTypeByAssetId(scriptId);
            if (typeName == null)
                throw new Exception("Component script asset not found");

            var type = Type.GetType(typeName);
            if (type == null)
                throw new Exception("Component type not exists");

            return type;
        }

        private void m_LoadObject(string selfPath, Type type, ref object instance) {

            var serializer = Serializer.GetSerializer(type);
            var fields = Serializer.GetFields(type, instance, serializer);

            if(serializer.NeedIncludeBase(type))
                m_LoadBaseObject(selfPath, ref instance);

            foreach (var field in fields) {
                var fieldPath = $"{selfPath}.{field.name}";
                m_LoadField(fieldPath, field);
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

        private void m_LoadField(string fieldPath, Field field) {
            m_LoadFieldsFromValues(m_values, fieldPath, field);
        }

        private bool m_LoadFieldsFromValues(YamlValues? values, string fieldPath, Field field) {
            if (values == null)
                return false;

            //var fullPath = m_GetFullPath(fieldPath);

            /// Поле записано.
            /// Либо простой тип, либо объект равный нулю, либо ссылка.
            if (values.HasValue(fieldPath)) {
                var yamlValue = values.GetValue(fieldPath);
                object? value = null;

                if (yamlValue.type == YamlValue.Type.Scalar)
                    value = m_Convert(field.type, yamlValue.value);

                if (yamlValue.type == YamlValue.Type.Link) {
                    var link = new InnerLink();

                    link.fileName = yamlValue.value; // $"{m_rootPath}.{yamlValue.value}";
                    link.field = field;

                    m_links.Add(link);
                }
                if (yamlValue.type == YamlValue.Type.AssetId) {
                    value = m_Convert(field.type, yamlValue.value);

                    m_assets.Add((IAsset)field.Instance);
                }
                field.SetValue(value);
                return true;
            }

            /// Есть дочерний путь.
            /// Объект или список не равные нулю.
            var hasChildren = values.HasChildren(fieldPath);
            if (hasChildren) {
                /// Список
                if (Serializer.IsList(field.type))
                    m_LoadList(fieldPath, field);

                /// Объект, для которого есть поля, значит можно инстанцировать.
                else {
                    if (field.Value == null)
                        field.SetValue(m_CreateInstance(field.type));

                    var fieldValue = field.Value;
                    m_LoadObject(fieldPath, field.type, ref fieldValue);
                    if (field.type.IsValueType)
                        field.SetValue(fieldValue);
                }
                return true;
            }

            /// Нет ни пути, ни дочерних путей.
            /// Ничего не делаем - оставляем значение по умолчанию.
            return false;
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
                list.Add(m_CreateInstance(itemType));
                m_LoadField(itemPath, new Field(list, itemType, i));
            }
        }

        private IList m_InitListInField(Field field) {
            IList list = null;

            if (field.Value != null) {
                list = field.Value as IList;
            }
            else {
                list = m_CreateInstance(field.type) as IList;
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

        private object m_CreateInstance(Type type) {
            var instance = Activator.CreateInstance(type);
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
