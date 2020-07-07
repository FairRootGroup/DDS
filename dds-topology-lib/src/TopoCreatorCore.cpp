// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

// DDS
#include "TopoCreatorCore.h"
#include "TopoProperty.h"
#include "TopoRequirement.h"
#include "TopoTrigger.h"
#include "TopoUtils.h"
#include "UserDefaults.h"
// BOOST
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

using namespace std;
using namespace dds;
using namespace topology_api;
using namespace user_defaults_api;

CTopoCreatorCore::CTopoCreatorCore()
{
}

CTopoCreatorCore::CTopoCreatorCore(const std::string& _fileName, const std::string& _schemaFilename)
{
    m_main->initFromXML(_fileName, _schemaFilename);
}

CTopoCreatorCore::CTopoCreatorCore(std::istream& _stream, const std::string& _schemaFilename)
{
    m_main->initFromXML(_stream, _schemaFilename);
}

CTopoCreatorCore::~CTopoCreatorCore()
{
}

void CTopoCreatorCore::save(std::ostream& _stream)
{
    boost::property_tree::ptree pt;
    save(pt);
    auto settings = boost::property_tree::xml_writer_settings<typename boost::property_tree::ptree::key_type>(' ', 4);
    boost::property_tree::write_xml(_stream, pt, settings);
}

void CTopoCreatorCore::save(const std::string& _filename)
{
    boost::property_tree::ptree pt;
    save(pt);
    auto settings = boost::property_tree::xml_writer_settings<typename boost::property_tree::ptree::key_type>(' ', 4);
    boost::property_tree::write_xml(_filename, pt, std::locale(), settings);
}

void CTopoCreatorCore::save(boost::property_tree::ptree& _pt)
{
    if (m_main != nullptr)
    {
        objectMap_t declElements;

        auto elements = m_main->getElements();
        for (const auto& element : elements)
        {
            addDeclElements(element, declElements);
        }

        vector<CTopoBase::EType> declOrder{ CTopoBase::EType::TOPO_VARS,   CTopoBase::EType::TOPO_PROPERTY,
                                            CTopoBase::EType::REQUIREMENT, CTopoBase::EType::TRIGGER,
                                            CTopoBase::EType::TASK,        CTopoBase::EType::COLLECTION };

        for (const auto& declType : declOrder)
        {
            auto decl = declElements.find(declType);
            if (decl != declElements.end())
            {
                for (const auto& element : decl->second)
                {
                    boost::property_tree::ptree elementPT;
                    element.second->saveToPropertyTree(elementPT);
                    std::string path = "topology." + TopoTypeToDeclTag(element.second->getType());
                    _pt.add_child(path, elementPT.get_child(path));
                }
            }
        }

        boost::property_tree::ptree mainPT;
        m_main->saveToPropertyTree(mainPT);
        _pt.add_child("topology.main", mainPT.get_child("topology.main"));

        _pt.put("topology.<xmlattr>.name", "topology");
    }
    else
    {
        throw runtime_error("Topology is empty: init() must be called before saving the topology");
    }
}

void CTopoCreatorCore::addDeclElements(CTopoElement::Ptr_t _element, objectMap_t& _declElements)
{
    switch (_element->getType())
    {
        case CTopoBase::EType::TASK:
            addDeclElements(static_pointer_cast<CTopoTask>(_element), _declElements);
            break;

        case CTopoBase::EType::COLLECTION:
            addDeclElements(static_pointer_cast<CTopoCollection>(_element), _declElements);
            break;

        case CTopoBase::EType::GROUP:
            addDeclElements(static_pointer_cast<CTopoGroup>(_element), _declElements);
            break;

        default:
            break;
    }
}

void CTopoCreatorCore::addDeclElements(CTopoTask::Ptr_t _task, objectMap_t& _declElements)
{
    _declElements[CTopoBase::EType::TASK][_task->getName()] = static_pointer_cast<CTopoBase>(_task);

    const auto& requirements = _task->getRequirements();
    for (const auto& requirement : requirements)
    {
        _declElements[CTopoBase::EType::REQUIREMENT][requirement->getName()] =
            static_pointer_cast<CTopoBase>(requirement);
    }

    const auto& properties = _task->getProperties();
    for (const auto& property : properties)
    {
        _declElements[CTopoBase::EType::TOPO_PROPERTY][property.first] =
            static_pointer_cast<CTopoBase>(property.second);
    }

    const auto& triggers = _task->getTriggers();
    for (const auto& trigger : triggers)
    {
        _declElements[CTopoBase::EType::TRIGGER][trigger->getName()] = static_pointer_cast<CTopoBase>(trigger);
    }
}

void CTopoCreatorCore::addDeclElements(CTopoCollection::Ptr_t _collection, objectMap_t& _declElements)
{
    _declElements[CTopoBase::EType::COLLECTION][_collection->getName()] = static_pointer_cast<CTopoBase>(_collection);

    const auto& requirements = _collection->getRequirements();
    for (const auto& requirement : requirements)
    {
        _declElements[CTopoBase::EType::REQUIREMENT][requirement->getName()] =
            static_pointer_cast<CTopoBase>(requirement);
    }

    const auto& elements = _collection->getElements();
    for (const auto& element : elements)
    {
        if (element->getType() == CTopoBase::EType::TASK)
        {
            addDeclElements(static_pointer_cast<CTopoTask>(element), _declElements);
        }
    }
}

void CTopoCreatorCore::addDeclElements(CTopoGroup::Ptr_t _group, objectMap_t& _declElements)
{
    const auto& elements = _group->getElements();
    for (const auto& element : elements)
    {
        if (element->getType() == CTopoBase::EType::TASK)
        {
            addDeclElements(static_pointer_cast<CTopoTask>(element), _declElements);
        }
        else if (element->getType() == CTopoBase::EType::COLLECTION)
        {
            addDeclElements(static_pointer_cast<CTopoCollection>(element), _declElements);
        }
    }
}

CTopoGroup::Ptr_t CTopoCreatorCore::getMainGroup() const
{
    return m_main;
}
