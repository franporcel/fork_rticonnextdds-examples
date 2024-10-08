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

#include "SocketConnection.hpp"
#include "SocketStreamReader.hpp"

using namespace rti::routing;
using namespace rti::routing::adapter;

SocketConnection::SocketConnection(
        StreamReaderListener *input_stream_discovery_listener,
        StreamReaderListener *output_stream_discovery_listener,
        const PropertySet &properties)
        : input_discovery_reader_(
                properties,
                input_stream_discovery_listener) {};

StreamReader *SocketConnection::create_stream_reader(
        Session *session,
        const StreamInfo &info,
        const PropertySet &properties,
        StreamReaderListener *listener)
{
    return new SocketStreamReader(this, info, properties, listener);
}

void SocketConnection::delete_stream_reader(StreamReader *reader)
{
    SocketStreamReader *socket_reader =
            dynamic_cast<SocketStreamReader *>(reader);
    socket_reader->shutdown_socket_reader_thread();
    delete reader;
}

DiscoveryStreamReader *SocketConnection::input_stream_discovery_reader()
{
    return &input_discovery_reader_;
}

void SocketConnection::dispose_discovery_stream(
        const rti::routing::StreamInfo &stream_info)
{
    input_discovery_reader_.dispose(stream_info);
}
