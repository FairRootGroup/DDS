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

CTopoCreator::CTopoCreator(const std::string& _fileName, const std::string& _schemaFilename)
    : m_topoCreator(std::make_shared<CTopoCreatorCore>(_fileName, _schemaFilename))
{
}

CTopoCreator::CTopoCreator(std::istream& _stream, const std::string& _schemaFilename)
    : m_topoCreator(std::make_shared<CTopoCreatorCore>(_stream, _schemaFilename))
{
}

CTopoCreator::~CTopoCreator()
{
}

void CTopoCreator::save(const std::string& _filename)
{
    m_topoCreator->save(_filename);
}

void CTopoCreator::save(std::ostream& _stream)
{
    m_topoCreator->save(_stream);
}

CTopoGroup::Ptr_t CTopoCreator::getMainGroup() const
{
    return m_topoCreator->getMainGroup();
}
