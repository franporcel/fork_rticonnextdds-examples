/*
 * (c) 2024 Copyright, Real-Time Innovations, Inc.  All rights reserved.
 *
 * RTI grants Licensee a license to use, modify, compile, and create derivative
 * works of the Software.  Licensee has the right to distribute object form
 * only for use with RTI products.  The Software is provided "as is", with no
 * warranty of any type, including any warranty for fitness for any purpose.
 * RTI is under no obligation to maintain or support the Software.  RTI shall
 * not be liable for any incidental or consequential damages arising out of the
 * use or inability to use the software.
 */

#ifndef SOCKETADAPTER_HPP
#define SOCKETADAPTER_HPP

#include <dds/core/corefwd.hpp>
#include <dds/core/macros.hpp>
#include <rti/routing/PropertySet.hpp>
#include <rti/routing/RoutingService.hpp>
#include <rti/routing/adapter/AdapterPlugin.hpp>

/*
 * This is the initialization of the RS adapter. For simplicity, this adapter
 * only reads from UDP and writes to DDS, not the other way around
 */
class SocketAdapter : public rti::routing::adapter::AdapterPlugin {
public:
    explicit SocketAdapter(rti::routing::PropertySet &);

    rti::routing::adapter::Connection *create_connection(
            rti::routing::adapter::detail::StreamReaderListener *,
            rti::routing::adapter::detail::StreamReaderListener *,
            const rti::routing::PropertySet &) final;

    void delete_connection(rti::routing::adapter::Connection *connection) final;

    rti::config::LibraryVersion get_version() const;
};

/**
 * This macro defines a C-linkage symbol that can be used as create function
 * for plug-in registration through XML.
 *
 * The generated symbol has the name:
 *
 * \code
 * SocketAdapterPlugin_create_adapter_plugin
 * \endcode
 */
RTI_ADAPTER_PLUGIN_CREATE_FUNCTION_DECL(SocketAdapter)

#endif
