// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "TopoCreator.h"
#include "TopoCreatorCore.h"

using namespace std;
using namespace dds;
using namespace topology_api;

CTopoCreator::CTopoCreator()
    : m_topoCreator(std::make_shared<CTopoCreatorCore>())
{
}

CTopoCreator::~CTopoCreator()
{
}

void CTopoCreator::init()
{
    m_topoCreator->init();
}

void CTopoCreator::init(const std::string& _fileName)
{
    m_topoCreator->init(_fileName);
}

void CTopoCreator::init(const std::string& _fileName, const std::string& _schemaFileName)
{
    m_topoCreator->init(_fileName, _schemaFileName);
}

void CTopoCreator::save(const std::string& _filename)
{
    m_topoCreator->save(_filename);
}

CTopoGroup::Ptr_t CTopoCreator::getMainGroup() const
{
    return m_topoCreator->getMainGroup();
}
