#pragma once
#include <unordered_map>
#include <vector>
#include <iterator>
#include <unordered_set>

#include "AssetStore.h"


class AssetIterator {
private:
    const AssetStore::AssetsMap& m_assets;
    const std::unordered_set<AssetStore::TypeHash>& m_typesSet;
    bool m_hasRule;
    bool m_isSelect;

    AssetStore::AssetsMap::const_iterator m_typeIter;
    size_t m_assetIndex;

public:
    AssetIterator(
        const AssetStore::AssetsMap& assets,
        const std::unordered_set<AssetStore::TypeHash>& typesSet,
        bool hasRule,
        bool isSelect
    ) :
        m_assets(assets),
        m_typesSet(typesSet),
        m_typeIter(m_assets.begin()),
        m_assetIndex(0),
        m_hasRule(hasRule),
        m_isSelect(isSelect)
    {
        m_SkipEmptyTypes();
    }

    AssetIterator& operator++() {
        if (m_typeIter != m_assets.end()) {
            ++m_assetIndex;

            if (m_assetIndex >= m_typeIter->second.size()) {
                m_assetIndex = 0;
                ++m_typeIter;
                m_SkipEmptyTypes();
            }
        }
        return *this;
    }

    AssetStore::AssetHash operator*() const {
        return m_typeIter->second[m_assetIndex];
    }

    bool operator==(const AssetIterator& other) const {
        return m_typeIter == other.m_typeIter && m_assetIndex == other.m_assetIndex;
    }

    bool operator!=(const AssetIterator& other) const {
        return !(*this == other);
    }

    void ToEnd() {
        m_typeIter = m_assets.end();
        m_assetIndex = 0;
    }

private:
    void m_SkipEmptyTypes() {
        for (; m_typeIter != m_assets.end(); m_typeIter++) {
            if (m_typeIter->second.empty())
                continue;

            if (m_hasRule) {
                if (m_isSelect && !m_typesSet.contains(m_typeIter->first))
                    continue;

                if (!m_isSelect && m_typesSet.contains(m_typeIter->first))
                    continue;
            }
            break;
        }
    }
};

class AssetSelection {
private:
    const AssetStore::AssetsMap& m_assets;
    std::unordered_set<AssetStore::TypeHash> m_typesSet;
    bool m_hasRule = false;
    bool m_isSelect = true;

public:
    AssetSelection(const AssetStore::AssetsMap& assets)
        : m_assets(assets) {}

    void Include(AssetStore::TypeHash typeHash) {
        m_typesSet.insert(typeHash);

        m_hasRule = true;
        m_isSelect = true;
    }

    void Exclude(AssetStore::TypeHash typeHash) {
        m_typesSet.insert(typeHash);

        m_hasRule = true;
        m_isSelect = false;
    }

    AssetIterator begin() const {
        return AssetIterator(m_assets, m_typesSet, m_hasRule, m_isSelect);
    }

    AssetIterator end() const {
        auto iter = AssetIterator(m_assets, m_typesSet, m_hasRule, m_isSelect);
        iter.ToEnd();

        return iter;
    }
};