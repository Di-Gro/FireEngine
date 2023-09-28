using System;
using System.IO;
using System.Collections;
using System.Collections.Generic;
using System.Text;

using YamlWriter = FireYaml.FireWriter;

/// TODO: Ассеты, сохраненные как AssetRef загружать только в поле с нужным типом ассета. 
/// Сейчас Mesh попробует загрузиться в Material. 

namespace FireBin {
    public class FireBinException : Exception {
        public FireBinException(string msg) : base(msg) { }
    }

    public class Deserializer {
        public event Action EndLoadEvent;

        private FireBin.Data m_data;

        public delegate void FromDelegate(Type type, ref object target, Pointer ptr);

        private readonly FromDelegate[] m_deserializers;

        private string m_typeConflicts = "";
        private string m_nameConflicts = "";

        public Deserializer(FireBin.Data data) {
            m_data = data;

            m_deserializers = new FromDelegate[(int)ValueType._Count] {
                FromList,
                FromNamedList,
                FromEnum,
                FromAssetRef,
                FromRef,
                FromScalar,
                FromString,
                FromVector2,
                FromVector3,
                FromQuaternion,
            };
        }

        public object Deserialize() {
            var structPtr = new Pointer { areaId = AreaId.Structs, offset = 0 };

            var scriptId = GetScriptId(structPtr);
            if(scriptId == "")
                throw new FireBinException("Struct not contains scriptId");

            var type = GetTypeOf(scriptId);
            var target = CreateInstance(type);

            Deserialize(type, ref target);

            return target;
        }

        public void Deserialize(Type type, ref object target) {
            var structPtr = new Pointer { areaId = AreaId.Structs, offset = 0 };

            FromNamedList(type, ref target, structPtr);

            EndLoadEvent?.Invoke();
        }

        public void FromNamedList(Type type, ref object target, Pointer ptr) {
            var structArea = m_data[AreaId.Structs];

            var serializer = YamlWriter.GetSerializer(type);
            var header = m_data.ReadNamedList(ptr);

            for(int i = 0; i< header.fields.Count; i++) {
                var fieldName = header.names.GetName(i);
                var valuePtr = header.fields[i];

                var field = GetSupportedField(fieldName, valuePtr, type, target, serializer);
                if(field == null)
                    continue;

                Console.WriteLine($"names[{i}]: {fieldName}");

                if (valuePtr.offset == Pointer.NullOffset) {
                    field.SetValue(null);
                    continue;
                }

                /// Что если это список сомпонентов? 
                /// Не нужно создавать экземпляр
                if (field.Value == null)
                    field.SetValue(CreateInstance(field.type));

                var fieldReader = m_GetReader(field.type);
                var fieldValue = field.Value;

                fieldReader.Invoke(field.type, ref fieldValue, valuePtr);

                if (field.type.IsValueType)
                    field.SetValue(fieldValue);
            }
            if (header.extra.Count > 0)
                serializer.ReadExtraFields(this, target, header.extra);

        }

        public void FromList(Type type, ref object target, Pointer ptr) {
            var structArea = m_data[AreaId.Structs];
            var header = m_data.ReadList(ptr);

            var targetList = target as IList;
            targetList.Clear();

            var genericType = type.GetGenericArguments()[0];
            var itemReader = m_GetReader(genericType);

            for (int i = 0; i < header.Count; i++) {
                var valuePtr = header[i];

                if (valuePtr.offset == Pointer.NullOffset) {
                    targetList.Add(null);
                    continue;
                }
                /// Что если это список сомпонентов? 
                var fieldValue = CreateInstance(genericType);

                itemReader.Invoke(genericType, ref fieldValue, valuePtr);

                if (field.type.IsValueType)
                    field.SetValue(fieldValue);
            }

        }

        public void FromEnum(Type type, ref object target, Pointer ptr) {
        }

        public void FromAssetRef(Type type, ref object target, Pointer ptr) {
        }

        public void FromRef(Type type, ref object target, Pointer ptr) {
        }

        public void FromScalar(Type type, ref object target, Pointer ptr) {
            var scalarsArea = m_data[AreaId.Scalars];
            var scalar = scalarsArea.SaveOffset(ptr.offset, (r, w) => m_data.ReadScalar());

            target = scalar.value;
        }

        public void FromString(Type type, ref object target, Pointer ptr) {
        }

        public void FromVector2(Type type, ref object target, Pointer ptr) {
        }

        public void FromVector3(Type type, ref object target, Pointer ptr) {
        }

        public void FromQuaternion(Type type, ref object target, Pointer ptr) {
        }

        private FromDelegate m_GetReader(Type type) {
            var valueType = Data.GetValueType(type);
            if (valueType == null)
                throw new FireBinException($"Unsupported type: {type.Name}");

            return m_deserializers[(int)valueType];
        }

        public string GetScriptId(Pointer structPtr) {
            var structsArea = m_data[AreaId.Structs];
            var assetRefArea = m_data[AreaId.AssetRefs];

            var header = m_data.ReadNamedList(structPtr);

            if (header.structType != StructType.NamedList)
                throw new FireBinException($"Struct type: {header.structType} is not {StructType.NamedList} type.");

            if (header.scriptIdPtr.offset == Pointer.NullOffset)
                return "";

            var scriptId = assetRefArea.SaveOffset(header.scriptIdPtr.offset, (r, w) => r.ReadString());

            return scriptId;
        }

        public static Type GetTypeOf(string scriptId) {
            var type = Engine.GUIDAttribute.GetTypeByGuid(scriptId);
            if (type == null)
                throw new FireBinException("Type not found");

            return type;
        }

        public static object CreateInstance(Type type) {
            if (type == typeof(string))
                return "";

            var instance = Activator.CreateInstance(type, true);
            if (instance == null)
                throw new FireBinException("A parameterless constructor is required to load the object.");

            return instance;
        }

        private FireYaml.Field GetSupportedField(string fieldName, Pointer valuePtr, Type type, object target, Engine.SerializerBase serializer) {
            var field = YamlWriter.GetField(fieldName, type, target, serializer);
            if (field == null) {
                m_WriteNameConflict(type, fieldName);
                return null;
            }
            var fieldType = Data.GetValueType(field.type);
            if (fieldType == null)
                return null;

            if (valuePtr.offset == Pointer.NullOffset)
                return field;

            var valueType = m_data.GetValueType(valuePtr);
            if (valueType == null)
                throw new FireBinException($"Unsupported type: {type.Name}");

            if (valueType != fieldType) {
                m_WriteTypeConflict(fieldType.Value, valueType.Value, fieldName);
                return null;
            }
            return field;
        }

        private void m_WriteNameConflict(Type type, string fieldName) {
            m_nameConflicts += $"Field: '{fieldName}' not found in type: '{type.FullName}'.\n";
        }

        private void m_WriteTypeConflict(ValueType fieldType, ValueType valueType, string fieldName) {
            m_typeConflicts += $"Field '{fieldName}' has FireBinType: '{fieldType}', but the value has type: '{valueType}'.\n";
        }

    }
}
