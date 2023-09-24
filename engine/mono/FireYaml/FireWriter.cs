using YamlDotNet.Serialization;
using YamlDotNet.Serialization.NamingConventions;
using YamlDotNet.RepresentationModel;
using System.Numerics;
using System.Collections;
using System.Collections.Generic;
using System.Reflection;
using System.Globalization;
using System.Diagnostics;
using System;

using Engine;

namespace FireYaml {

    public class Field {
        public string name;
        public Type type;

        public object? Value => GetValue();

        /// Объект, в котором находится поле
        public object? Instance { get; private set; }

        private FieldInfo m_field = null;
        private PropertyInfo m_prop = null;
        private IList m_list = null;

        private int m_index = -1;

        public Field(FieldInfo field, object instance) {
            m_field = field;
            name = field.Name; ;
            type = field.FieldType;
            Instance = instance;
        }

        public Field(PropertyInfo prop, object instance) {
            m_prop = prop;
            name = prop.Name; ;
            type = prop.PropertyType;
            Instance = instance;
        }

        public Field(IList list, Type itemType, int index) {
            type = itemType;
            m_list = list;
            m_index = index;
            Instance = m_list;
            name = m_index.ToString();
        }

        public void SetValue(object? value) {
            if (Instance == null)
                return;

            if (m_field != null)
                m_field.SetValue(Instance, value);
            else if (m_prop != null)
                m_prop.SetValue(Instance, value);
            else if (m_list != null)
                m_list[m_index] = value;
        }

        public object? GetValue() {
            if (Instance == null)
                return null;

            if (m_field != null)
                return m_field.GetValue(Instance);
            else if (m_prop != null)
                return m_prop.GetValue(Instance);
            else if (m_list != null)
                return m_list[m_index];
            return null;
        }

        public override string ToString() {
            return $"{name}";
        }

        public TAttribute GetCustomAttribute<TAttribute>() where TAttribute : Attribute {
            if (Instance == null)
                return null;

            if (m_field != null)
                return m_field.GetCustomAttribute<TAttribute>();
            else if (m_prop != null)
                return m_prop.GetCustomAttribute<TAttribute>();

            return null;
        }
    }

    public class FireWriter {
        public static bool showLog = false;

        private class InnerLink {
            public string rootPath = "";

            // Поле в котором находится внутренняя ссылка
            public string fieldPath = null;

            // Объект, на который ссылается поле
            public object instance = null;

            // Тип объекта
            public Type type = null;
        }

        public static string TagInnerRef = "!R";
        public static string TagMeta = "!M";

        private YamlValues m_values = new YamlValues();
        private YamlValues m_origValues = new YamlValues();
        private YamlValues m_mods = null;
        private string m_rootPath = "";
        //private string m_assetId = "";
        private ulong m_assetInst = 0;

        //private YamlDocument m_documents = new YamlDocument();
        //AssetStore tmp_assetStore = new AssetStore();
        private Dictionary<object, string> m_writedDocs = new Dictionary<object, string>();
        private List<InnerLink> m_links = new List<InnerLink>();

        private int m_nextDocIndex = 1;
        private bool m_ignoreExistingIds = false;
        private bool m_writeNewIds = false;
        private bool m_useCsRefs = false;

        public bool Result { get; private set; }

        public string Text => m_values.ToSortedText();
        public YamlValues Values => m_values;
        public YamlValues Mods => m_mods;
        public int FilesCount => m_nextDocIndex - 1;


        public static BindingFlags s_flags =
               BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic |
               BindingFlags.GetField | BindingFlags.SetField | BindingFlags.GetProperty |
               BindingFlags.SetProperty;

        /// <summary>
        /// 
        /// </summary>
        /// <param name="startId">Начальный ID файла.</param>
        /// <param name="ignoreExistingIds">Игнорирует ID файлов, с которыми был загружен объект.</param>
        /// <param name="writeNewIds">Записывает новые ID файла в объект.</param>
        public FireWriter(int startId = 1, bool ignoreExistingIds = false, bool writeNewIds = true, bool useCsRefs = false) {
            m_nextDocIndex = startId;
            m_ignoreExistingIds = ignoreExistingIds;
            m_writeNewIds = writeNewIds;
            m_useCsRefs = useCsRefs;
        }

        public void Serialize(object obj) {
            Result = false;

            if (obj == null)
                return;

            m_values.Clear();

            var type = obj.GetType();

            if (!CanSerialize(type))
                throw new Exception($"!CanSerialize({type.FullName})");

            if (obj.GetType().GetInterface(nameof(IFile)) == null)
                throw new Exception("Target must be assignable to IFile");

            if (m_assetInst == 0)
                m_assetInst = IFile.GetAssetInstance(ref obj);

            // var prefabId = IFile.GetPrefabId(ref obj);
            // if (prefabId != IFile.NotPrefab) {
            //     var origValues = AssetStore.Instance.ThrowAssetValues(prefabId);
            //     var basePrefabId = origValues.GetValue(".file1!prefabId", IFile.NotPrefab);
            //     IFile.SetPrefabId(basePrefabId, ref obj);
            // }

            CreateDocument(type, obj, notSaveAsAsset: true);

            // if (prefabId != IFile.NotPrefab)
            //     IFile.SetPrefabId(prefabId, ref obj);

            m_ResolveLinks();

            m_mods = YamlValues.Mods(m_origValues, m_values); //TODO: Mods

            Result = true;
        }

        public string CreateDocument(Type type, object instance, bool notSaveAsAsset = false) {
            //var existName = m_GetAssignedDoc(m_rootPath, type, instance);
            var existName = m_GetAssignedDoc(instance, m_rootPath);
            if (existName != "")
                return existName;

            if(!GUIDAttribute.HasGuid(type))
                throw new Exception($"FireWriter: Type '{type.FullName}' don't have GUID attribute");

            var typeGuid = GUIDAttribute.GetGuid(type); // m_GetAssetId(type); 
            var yamlRef = m_GetFileRef(ref instance);
            var docName = yamlRef.Name;
            //var docPath = $".{docName}";
            var fullPath = GetFullPath(docName);
            var scriptPath = $"{fullPath}!scriptId";
            var scriptIdValue = new YamlValue(YamlValue.Type.AssetId, typeGuid);
            var scriptValue = new YamlValue(YamlValue.Type.Var, type.Name);
            
            if (showLog) Console.WriteLine($"CreateDocument {fullPath}");

            m_values.AddValue($"{fullPath}!script", scriptValue);
            m_values.AddValue(scriptPath, scriptIdValue);

            // m_SetAssignedDoc(fullPath, m_rootPath, type, instance);
            m_SetAssignedDoc(fullPath, m_rootPath, instance);

            // var prefabId = IFile.GetPrefabId(ref instance);
            // if (prefabId != IFile.NotPrefab) {
                // m_CreatePrefab(fullPath, type, ref instance, prefabId);
            // }
            // else {
                m_CreateObject(fullPath, type, instance, canSaveAsLink: false, notSaveAsAsset: notSaveAsAsset);
            // }

            IFile.SetAssetInstance(m_assetInst, ref instance);

            return docName;
        }

        private void m_CreatePrefab(string selfPath, Type type, ref object instance, string prefabId) {

            // IFile.SetPrefabId(prefabId, ref instance);

            var prefabPath = $"{selfPath}!prefabId";
            var prefabValue = new YamlValue(YamlValue.Type.AssetId, prefabId);
            m_values.AddValue(prefabPath, prefabValue);

            return;

            ////
            // var prefabInfo = AssetStore.Instance.GetAssetInfo(prefabId);
            // var origValues = AssetStore.Instance.ThrowAssetValues(prefabId);
            // var basePrefabId = origValues.GetValue(".file1!prefabId", IFile.NotPrefab);

            // var prefabPath = $"{selfPath}!prefabId";
            // var prefabValue = new YamlValue(YamlValue.Type.AssetId, prefabId);
            // m_values.AddValue(prefabPath, prefabValue);

            // /// Запоминаем старое состояние
            // var lastRoot = m_rootPath;
            // var lastNextIndex = m_nextDocIndex;
            // var last_m_assetInst = m_assetInst;
            // var lsst_fileId = IFile.GetFileId(ref instance);

            // /// Устанавливаем новое состояние
            // m_assetInst = IFile.GetAssetInstance(ref instance);
            // m_rootPath = $"{selfPath}!";
            // m_nextDocIndex = prefabInfo.files + 1;
            // IFile.SetPrefabId(basePrefabId, ref instance); /// TODO: это просто, чтобы обновить информацию
            // IFile.SetFileId(1, ref instance);

            // /// Внутри нового состояния
            // m_origValues.Append(origValues, m_rootPath);

            // m_origValues.RemoveValue($"{m_rootPath}.file0.assetId");
            // m_origValues.RemoveValue($"{m_rootPath}.file0.files");

            // CreateDocument(type, instance);

            // /// Восстанавливаем старое состояние
            // m_rootPath = lastRoot;
            // m_nextDocIndex = lastNextIndex;
            // IFile.SetAssetInstance(last_m_assetInst, ref instance);
            // IFile.SetPrefabId(prefabId, ref instance);
            // IFile.SetFileId(lsst_fileId, ref instance);

        }

        private void m_CreateObject(string selfPath, Type type, object obj, bool canSaveAsLink = true, bool notSaveAsAsset = false) {
            if (obj == null)
                throw new ArgumentNullException(nameof(obj));

            if (canSaveAsLink && NeedSaveAsLink(type)) {
                var link = new InnerLink();
                link.type = type;
                link.instance = obj;
                link.fieldPath = selfPath;
                link.rootPath = m_rootPath;
                m_links.Add(link);
                return;
            }
            bool isAsset = IsAsset(type);
            if(!notSaveAsAsset && isAsset) {
                m_CreateAsset(selfPath, ref obj);
                return;
            }
            var serializer = GetSerializer(type);
            var fields = GetFields(type, obj, serializer);

            if (fields.Count == 0 && !IsComponent(type)) {
                m_AddScalar(selfPath, obj);
                return;
            }
            foreach (var field in fields) {
                var fieldPath = $"{selfPath}.{field.name}";
                if(isAsset && field.name == nameof(IAsset.assetId))
                    continue;
                    
                AddField(fieldPath, field.type, field.Value);
            }
            serializer.OnSerialize(this, selfPath, type, obj);

            if (serializer.NeedIncludeBase(type) && type.BaseType != null) 
                m_CreateBaseObject(selfPath, type.BaseType, ref obj);
        }

        private void m_CreateBaseObject(string selfPath, Type baseType, ref object obj) {
            var basePath = $"{selfPath}!base";
            var scriptPath = $"{basePath}!scriptId";
            var baseTypeGuid = GUIDAttribute.GetGuid(baseType); // m_GetAssetId(baseType); 
            var scriptValue = new YamlValue(YamlValue.Type.AssetId, baseTypeGuid);

            if(!GUIDAttribute.HasGuid(baseType))
                throw new Exception($"FireWriter: Base type '{baseType.FullName}' don't have GUID attribute");

            m_values.AddValue(scriptPath, scriptValue);

            m_CreateObject(basePath, baseType, obj, false);
        }

        private void m_CreateAsset(string selfPath, ref object obj) {
            var asset = (IAsset)obj;

            bool isTmpId = AssetStore.IsTmpAssetId(asset.assetIdHash);
            if(isTmpId) {
                m_values.AddValue(selfPath, new YamlValue());
                return;
            }

            var assetId = new YamlValue();
            assetId.type = YamlValue.Type.AssetId;
            assetId.value = ToYamlString(asset.assetId);

            if (showLog) Console.WriteLine($"Add {selfPath}: {assetId}");

            var assetIdPath = $"{selfPath}.assetId";
            m_values.AddValue(assetIdPath, assetId);
        }

        public static bool IsAsset(Type type){
            return type.GetInterface(nameof(IAsset)) != null;
        }

        public static bool IsAssetWithSource(Type type) {
            return type.GetInterface(nameof(ISourceAsset)) != null;
        }

        public static bool IsFile(Type type) {
            return type.GetInterface(nameof(IFile)) != null;
        }

        private YamlRef m_GetFileRef(ref object instance) {
            if (m_ignoreExistingIds) {
                if (showLog) Console.WriteLine($"m_GetFileRef -> m_nextDocIndex++ -> {m_nextDocIndex}");
                return new YamlRef(m_nextDocIndex++);
            }

            var assetInst = IFile.GetAssetInstance(ref instance);
            if (assetInst != m_assetInst) {
                var fileId2 = m_nextDocIndex++;
                if (m_writeNewIds)
                    IFile.SetFileId(fileId2, ref instance);

                if (showLog) Console.WriteLine($"m_GetFileRef -> fileId2 -> {fileId2}");
                return new YamlRef(fileId2);
            }

            var fileId = IFile.GetFileId(ref instance);
            if (fileId >= 0) {
                if (showLog) Console.WriteLine($"m_GetFileRef -> fileId -> {fileId}");
                return new YamlRef(fileId);
            }

            fileId = m_nextDocIndex++;
            if (m_writeNewIds)
                IFile.SetFileId(fileId, ref instance);

            if (showLog) Console.WriteLine($"m_GetFileRef -> fileId = m_nextDocIndex++ -> {fileId}");
            return new YamlRef(fileId);
        }

        public string GetFullPath(string fileName) {
            return $"{m_rootPath}.{fileName}";
        }

        public static IFile? ToIFile(ref object target) {
            var type = target.GetType();
            var ifileType = typeof(FireYaml.IFile);

            if (type.GetInterface(nameof(IFile)) != null)
                return (IFile)target;

            return null;
        }

        private void m_AddScalar(string selfPath, object? obj) {
            var selfValue = new YamlValue();
            selfValue.type = YamlValue.Type.Var;
            selfValue.value = obj != null ? ToYamlString(obj) : "";

            if (showLog) Console.WriteLine($"Add {selfPath}: {selfValue}");

            m_values.AddValue(selfPath, selfValue);
        }

        public void AddField(string fullFieldPath, Type type, object? value) {
            if (showLog) Console.WriteLine($"# AddField: {fullFieldPath}, type:{type.Name}");

            bool isList = IsList(type);
            if (showLog) Console.WriteLine($"# AddField: isList:{isList}");

            if (value == null) {
                var selfValue = new YamlValue(YamlValue.Type.Null);
                m_values.AddValue(fullFieldPath, selfValue);
                return;
            }
            else if (isList)
                m_CreateList(fullFieldPath, type, value);
            else
                m_CreateObject(fullFieldPath, type, value);
        }

        private void m_CreateList(string listPath, Type listType, object listInstance) {
            var list = listInstance as IList;
            if (list == null)
                throw new Exception("List not implements IList interface");

            var genericType = listType.GetGenericArguments()[0];

            if (showLog) Console.WriteLine($"List {listPath}: ");

            if(list.Count == 0) {
                AddField(listPath, listType, null);
                return;
            }

            for (int i = 0; i < list.Count; i++) {
                var value = list[i];
                var valueType = value != null ? value.GetType() : genericType;
                var itemPath = $"{listPath}.{i}";
                AddField(itemPath, valueType, value);
            }
        }

        // private string m_GetAssetId(Type type) {
        //     string assetId = "";

        //     if (!AssetStore.Instance.TryGetAssetIdByType(type.FullName, out assetId))
        //         throw new Exception($"AssetId of {type.FullName} not found");

        //     return assetId;
        // }

        public static bool NeedSaveAsLink(Type type) {
            if (type == typeof(Engine.Actor))
                return true;

            if (typeof(Engine.Component).IsAssignableFrom(type))
                return true;

            return false;
        }

        public static bool IsComponent(Type type) {
            if (typeof(Engine.Component).IsAssignableFrom(type))
                return true;

            return false;
        }

        public static bool IsUserComponent(Type type) {
            bool notBaseClass = true
                && type != typeof(Engine.CSComponent)
                && type != typeof(Engine.CppComponent);

            bool isAssignable = false
                || typeof(Engine.CSComponent).IsAssignableFrom(type)
                || typeof(Engine.CppComponent).IsAssignableFrom(type);

            return notBaseClass && isAssignable;
        }

        private void m_SetAssignedDoc(string docName, string rootPath, object obj) {
            if (obj == null)
                throw new ArgumentNullException("obj");

            var hash = $"{obj.GetHashCode()}_{rootPath}";

            m_writedDocs[hash] = docName;
        }

        private string m_GetAssignedDoc(object obj, string rootPath) {
            if (obj == null)
                throw new ArgumentNullException();

            var hash = $"{obj.GetHashCode()}_{rootPath}";

            if (m_writedDocs.ContainsKey(hash))
                return m_writedDocs[hash];
            return "";
        }

        private void m_ResolveLinks() {
            foreach (var link in m_links) {
                var value = new YamlValue();

                var docName = m_GetAssignedDoc(link.instance, link.rootPath);
                if (docName != "") {
                    value.type = YamlValue.Type.Ref;
                    value.value = docName.Substring(1);
                    
                } else if (m_useCsRefs) {
                    var cppLinked = link.instance as Engine.CppLinked;

                    value.type = YamlValue.Type.CsRef;
                    value.value = $"{cppLinked.csRef.value}";
                }

                m_values.AddValue(link.fieldPath, value);
            }
        }

        private string ToYamlString(object value) {
            if (value == null)
                throw new ArgumentNullException("ToYamlString");

            var type = value.GetType();

            if (type == typeof(float) || type == typeof(double))
                return value.ToString().Replace(',', '.');

            if (type == typeof(string))
                return $"{value}";

            return value.ToString();
        }

        public static bool IsList(Type type) {
            return (type.IsGenericType && (type.GetGenericTypeDefinition() == typeof(List<>)));
        }

        public static bool CanSerialize(Type type) {

            if (typeof(Engine.Component).IsAssignableFrom(type))
                return true;

            if (type == typeof(object))
                return false;

            if (type == typeof(string) ||
                type == typeof(Engine.Vector2) ||
                type == typeof(Engine.Vector3) ||
                type == typeof(Engine.Vector4) ||
                type == typeof(Engine.Quaternion)) {
                return true;
            }

            var isList = IsList(type);
            if (isList) {
                var argType = type.GetGenericArguments()[0];
                return !IsList(argType) && CanSerialize(argType);
            }

            var isPrimitive = type.IsPrimitive;
            var isEnum = type.IsEnum;
            if (isPrimitive || isEnum)
                return true;

            var isClass = type.IsClass;
            var isValueType = type.IsValueType;
            if (isClass || isValueType) {
                var guidAttr = HasGUIDAttr(type);
                var hasEmptyConstructor = HasEmptyConstructor(type);
                return guidAttr && hasEmptyConstructor;
            }

            return false;
        }

        public static bool HasGUIDAttr(Type? type) {
            if(type == null || type == typeof(object))
                return false;

            var attr = type.GetCustomAttribute<Engine.GUIDAttribute>();
            if(attr != null)
                return true;  
            
            return HasGUIDAttr(type.BaseType);
        }

        public static bool HasEmptyConstructor(Type type) {
            var flags = BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic;

            var constructor = type.GetConstructor(flags, null, Type.EmptyTypes, null);

            return constructor != null;
        }

        public static Engine.SerializerBase GetSerializer(Type type) {
            var serializerType = Type.GetType($"{type.FullName}Serializer");
            var isSerializer = typeof(Engine.SerializerBase).IsAssignableFrom(serializerType);

            if (serializerType == null || !isSerializer) {
                bool isCsComponent = typeof(Engine.CSComponent).IsAssignableFrom(type);
                bool isCppComponent = typeof(Engine.CppComponent).IsAssignableFrom(type);

                if (isCsComponent || isCppComponent)
                    return new Engine.ComponentSerializer();

                return new Engine.SerializerBase();
            }
            var serializer = Activator.CreateInstance(serializerType);

            return serializer as Engine.SerializerBase;
        }

        // public static Engine.SerializerBase GetSerializer(Type type) {
        //     var serializerType = Type.GetType($"{type.FullName}Serializer");

        //     var isSerializer = typeof(Engine.SerializerBase).IsAssignableFrom(serializerType);

        //     if (serializerType == null ||
        //         serializerType.BaseType == null ||
        //         serializerType.BaseType != typeof(Engine.SerializerBase)) 
        //     {
        //         bool isCsComponent = typeof(Engine.CSComponent).IsAssignableFrom(type);
        //         bool isCppComponent = typeof(Engine.CppComponent).IsAssignableFrom(type);

        //         if (isCsComponent || isCppComponent)
        //             return new Engine.ComponentSerializer();

        //         return new Engine.SerializerBase();
        //     }
        //     var serializer = Activator.CreateInstance(serializerType);

        //     return serializer as Engine.SerializerBase;
        // }

        public static List<Field> GetFields(Type type, object obj, Engine.SerializerBase serializer) {
            var fields = new List<Field>();

            if (type.IsEnum)
                return fields; 

            var allFields = type.GetFields(s_flags);
            var allProps = type.GetProperties(s_flags);

            foreach (var field in allFields) {
                if (field.DeclaringType != type || !CanSerialize(field.FieldType))
                    continue;
                if (serializer.NeedSerialize(field)) {
                    if(showLog) Console.WriteLine($"#: GetFields: field:{field.Name}");
                    fields.Add(new Field(field, obj));
                }
            }
            foreach (var prop in allProps) {
                if (prop.DeclaringType != type || !CanSerialize(prop.PropertyType))
                    continue;
                if (serializer.NeedSerialize(prop)) {
                    if (showLog) Console.WriteLine($"#: GetFields: prop:{prop.Name}");
                    fields.Add(new Field(prop, obj));
                }
            }
            return fields;
        }

        public static Field GetField(string name, Type containedType, object containedObj, Engine.SerializerBase serializer) {
            if (containedType.IsEnum)
                return null;

            var field = containedType.GetField(name, s_flags);
            var prop = containedType.GetProperty(name, s_flags);

            if(field != null) {
                if (field.DeclaringType != containedType || !CanSerialize(field.FieldType))
                    return null;

                if (serializer.NeedSerialize(field)) 
                    return new Field(field, containedObj);
            }
            if (prop != null) {
                if (prop.DeclaringType != containedType || !CanSerialize(prop.PropertyType))
                    return null;

                if (serializer.NeedSerialize(prop))
                    return new Field(prop, containedObj);
            }
            return null;
        }

    }
}
