// Copyright 2022 GSI, Inc. All rights reserved.
//
//
//

#ifndef __DDS__TopoAsset__
#define __DDS__TopoAsset__

// DDS
#include "TopoBase.h"
// STD
#include <string>

namespace dds
{
    namespace topology_api
    {
        /// \class CTopoAsset
        /// \brief Data class to hold task trigger.
        class CTopoAsset : public CTopoBase
        {
          public:
            enum class EType
            {
                Inline
            };

            enum class EVisibility
            {
                Task,  ///< The asset is visible only for the task it is assigned to
                Global ///< The asset is visible for all tasks of the given session.
            };

            using Ptr_t = std::shared_ptr<CTopoAsset>;
            using PtrVector_t = std::vector<CTopoAsset::Ptr_t>;

            /// \brief Constructor.
            CTopoAsset(const std::string& _name);

            /// \brief Destructor.
            virtual ~CTopoAsset();

            /// \brief Inherited from TopoBase
            void initFromPropertyTree(const boost::property_tree::ptree& _pt);

            /// \brief Inherited from TopoBase
            void saveToPropertyTree(boost::property_tree::ptree& _pt);

            std::string getValue() const;
            void setValue(const std::string& _val);

            CTopoAsset::EType getAssetType() const;
            void setAssetType(const CTopoAsset::EType& _val);

            CTopoAsset::EVisibility getAssetVisibility() const;
            void setAssetVisibility(const CTopoAsset::EVisibility& _val);

            /// \brief Returns string representation of an object.
            /// \return String representation of an object.
            virtual std::string toString() const;

            /// \brief Operator << for convenient output to ostream.
            /// \return Insertion stream in order to be able to call a succession of
            /// insertion operations.
            friend std::ostream& operator<<(std::ostream& _strm, const CTopoAsset& _trigger);

            /// \brief Inherited from TopoBase
            virtual std::string hashString() const;

          private:
            std::string m_value;                                                   ///< Asset value
            CTopoAsset::EType m_type{ CTopoAsset::EType::Inline };                 ///< Asset type
            CTopoAsset::EVisibility m_visibility{ CTopoAsset::EVisibility::Task }; /// < Asset visibility
        };
    } // namespace topology_api
} // namespace dds

#endif /* defined(__DDS__TopoAsset__) */
