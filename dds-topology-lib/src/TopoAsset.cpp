// Copyright 2022 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "TopoAsset.h"
#include "TopoUtils.h"
// STD
#include <boost/regex.hpp>
#include <iostream>

using namespace std;
using namespace boost::property_tree;
using namespace dds;
using namespace topology_api;

CTopoAsset::CTopoAsset(const string& _name)
    : CTopoBase(_name)
{
    setType(CTopoBase::EType::ASSET);
}

CTopoAsset::~CTopoAsset()
{
}

void CTopoAsset::initFromPropertyTree(const boost::property_tree::ptree& _pt)
{
    try
    {
        const auto& assetPT =
            (FindElementInPropertyTree(CTopoBase::EType::ASSET, getName(), _pt.get_child("topology")))->second;
        setAssetType(TagToAssetType(assetPT.get<string>("<xmlattr>.type", "")));
        setAssetVisibility(TagToAssetVisibility(assetPT.get<string>("<xmlattr>.visibility", "")));

        // Add asset value if the asset container doesn't have it.
        if (getNameToValueCache() && !getNameToValueCache()->exists(getName()))
        {
            getNameToValueCache()->insertValue(getName(), assetPT.get<string>("<xmlattr>.value", ""));
        }
    }
    catch (exception& error) // ptree_error, runtime_error
    {
        throw logic_error("Unable to initialize asset " + getName() + " error:" + error.what());
    }
}

void CTopoAsset::saveToPropertyTree(boost::property_tree::ptree& _pt)
{
    try
    {
        string tag("topology.asset.<xmlattr>");
        _pt.put(tag + ".name", getName());
        _pt.put(tag + ".type", AssetTypeToTag(getAssetType()));
        _pt.put(tag + ".visibility", AssetVisibilityToTag(getAssetVisibility()));
        _pt.put(tag + ".value", getValue());
    }
    catch (exception& error) // ptree_error, runtime_error
    {
        throw logic_error("Unable to save asset " + getName() + " error:" + error.what());
    }
}

string CTopoAsset::getValue() const
{
    CNameToValueCachePtr_t assetCache{ getNameToValueCache() };

    if (!assetCache)
        return string();

    return (assetCache->getValue(getName()));
}

void CTopoAsset::setValue(const string& _val)
{
    m_value = _val;
}

CTopoAsset::EType CTopoAsset::getAssetType() const
{
    return m_type;
}

void CTopoAsset::setAssetType(const CTopoAsset::EType& _val)
{
    m_type = _val;
}

CTopoAsset::EVisibility CTopoAsset::getAssetVisibility() const
{
    return m_visibility;
}

void CTopoAsset::setAssetVisibility(const CTopoAsset::EVisibility& _val)
{
    m_visibility = _val;
}

string CTopoAsset::toString() const
{
    stringstream ss;
    ss << "Asset: name=" << getName() << " type=" << AssetTypeToTag(getAssetType())
       << " visibility=" << AssetVisibilityToTag(getAssetVisibility()) << " value=" << getValue();
    return ss.str();
}

ostream& operator<<(ostream& _strm, const CTopoAsset& _requirement)
{
    _strm << _requirement.toString();
    return _strm;
}

string CTopoAsset::hashString() const
{
    // WORKAROUND:
    // Assets value can be big.
    // We therefore can't use its value in hash calculations as it significantly drops the performance.
    // see GH-454
    // That means, if an asset value is changed, DDS will be not able to detect changes.
    // Therefore, if you need to change the asset value and request a topology update, change its name too.
    stringstream ss;
    ss << "|Asset|" << getName() << "|" << AssetTypeToTag(getAssetType()) << "|"
       << AssetVisibilityToTag(getAssetVisibility()) << "|" /*<< getValue() << "|"*/;
    return ss.str();
}
