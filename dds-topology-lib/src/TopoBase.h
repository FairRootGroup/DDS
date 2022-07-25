// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__TopoBase__
#define __DDS__TopoBase__

// STD
#include <map>
#include <sstream>
#include <string>
#include <vector>
// BOOST
#include <boost/property_tree/ptree.hpp>

namespace dds
{
    namespace topology_api
    {
        class CNameToValueCache
        {
            using container_t = std::map<std::string, std::string>;

          public:
            void insertValue(const std::string& _name, const std::string _value)
            {
                if (exists(_name))
                    return;

                m_container.insert(std::make_pair(_name, _value));
            }

            std::string getValue(const std::string& _name)
            {
                auto found = m_container.find(_name);
                if (found == m_container.end())
                    return std::string();

                return found->second;
            }

            bool exists(const std::string& _name)
            {
                auto found = m_container.find(_name);
                return (found != m_container.end());
            }

          private:
            container_t m_container;
        };

        class CTopoBase
        {
          public:
            enum class EType
            {
                TOPO_BASE,
                TOPO_PROPERTY,
                TOPO_ELEMENT,
                TASK,
                COLLECTION,
                GROUP,
                REQUIREMENT,
                TOPO_VARS,
                TRIGGER,
                ASSET
            };

            using Ptr_t = std::shared_ptr<CTopoBase>;
            using PtrVector_t = std::vector<CTopoBase::Ptr_t>;
            using CNameToValueCachePtr_t = std::shared_ptr<CNameToValueCache>;

            /// Modifiers
            void setName(const std::string& _name);
            void setParent(CTopoBase* _parent);
            void setNameToValueCache(CNameToValueCachePtr_t _container);

            /// Accessors
            const std::string& getName() const;
            CTopoBase::EType getType() const;
            CTopoBase* getParent() const;
            CNameToValueCachePtr_t getNameToValueCache() const;

            /// \brief Return full path to topo element or property.
            std::string getPath() const;

            /// \brief Convenience API to create topology object from XML file.
            /// \param[in] _name Name of the object as in input file.
            /// \param[in] _filepath Path to the topology file.
            /// \param[in] _schemaFilepath Path to the XSD schema file.
            /// \param[out] _topologyName Topology name.
            /// \throw std::runtime_error.
            template <class Object_t>
            static typename Object_t::Ptr_t make(const std::string& _objectName,
                                                 const std::string& _filepath,
                                                 const std::string& _schemaFilepath = "",
                                                 std::string* _topologyName = nullptr)
            {
                typename Object_t::Ptr_t newObject{ std::make_shared<Object_t>(_objectName) };
                newObject->initFromXML(_filepath, _schemaFilepath, _topologyName);
                return newObject;
            }

            /// \brief Convenience API to create topology object from XML file.
            /// \param[in] _name Name of the object as in input file.
            /// \param[in] _stream Input stream
            /// \param[in] _schemaFilepath Path to the XSD schema file.
            /// \param[out] _topologyName Topology name.
            /// \throw std::runtime_error.
            template <class Object_t>
            static typename Object_t::Ptr_t make(const std::string& _objectName,
                                                 std::istream& _stream,
                                                 const std::string& _schemaFilepath = "",
                                                 std::string* _topologyName = nullptr)
            {
                typename Object_t::Ptr_t newObject{ std::make_shared<Object_t>(_objectName) };
                newObject->initFromXML(_stream, _schemaFilepath, _topologyName);
                return newObject;
            }

            /// \brief Convenience API to create topology object with data from property tree.
            /// \param[in] _name Name of the object as in input file.
            /// \param[in] _pt Property tree starting from root.
            /// \throw std::runtime_error
            template <class Object_t>
            static typename Object_t::Ptr_t make(const std::string& _name, const boost::property_tree::ptree& _pt)
            {
                typename Object_t::Ptr_t newObject{ std::make_shared<Object_t>(_name) };
                newObject->initFromPropertyTree(_pt);
                return newObject;
            }

            /// \brief Initializes object with data from XML file.
            /// \param[in] _filepath Path to the topology file.
            /// \param[in] _schemaFilepath Path to the XSD schema file.
            /// \param[out] _topologyName Topology name.
            /// \throw std::runtime_error.
            void initFromXML(const std::string& _filepath,
                             const std::string& _schemaFilepath = "",
                             std::string* _topologyName = nullptr);

            /// \brief Initializes object with data from XML input stream.
            /// \param[in] _stream Input stream.
            /// \param[in] _schemaFilepath Path to the XSD schema file.
            /// \param[out] _topologyName Topology name.
            /// \throw std::runtime_error.
            void initFromXML(std::istream& _stream,
                             const std::string& _schemaFilepath = "",
                             std::string* _topologyName = nullptr);

            /// \brief Initialize object with data from property tree.
            /// \param[in] _name Name of the object as in input file.
            /// \param[in] _pt Property tree starting from root.
            virtual void initFromPropertyTree(const boost::property_tree::ptree& _pt) = 0;

            /// \brief Save object to a property tree.
            /// \param[out] _pt Output ptoperty tree.
            virtual void saveToPropertyTree(boost::property_tree::ptree& _pt) = 0;

            /// \brief Returns string representation of an object.
            /// \return String representation of an object.
            virtual std::string toString() const;

            /// \brief Operator << for convenient output to ostream.
            /// \return Insertion stream in order to be able to call a succession of
            /// insertion operations.
            friend std::ostream& operator<<(std::ostream& _strm, const CTopoBase& _element);

            /// \brief Returns string which is used to calculate CRC checksum of the object
            virtual std::string hashString() const = 0;

          protected:
            /// \brief Constructor.
            CTopoBase(const std::string& _name);

            /// \brief Destructor.
            virtual ~CTopoBase();

            void setType(CTopoBase::EType _type);

          private:
            std::string m_name;                          ///< Name of topology element
            CTopoBase::EType m_type{ EType::TOPO_BASE }; ///< Type of the topology element
            CTopoBase* m_parent{ nullptr };              ///< Pointer to parent element
            CNameToValueCachePtr_t m_nameToValueCache;
        };
    } // namespace topology_api
} // namespace dds
#endif /* defined(__DDS__TopoBase__) */
