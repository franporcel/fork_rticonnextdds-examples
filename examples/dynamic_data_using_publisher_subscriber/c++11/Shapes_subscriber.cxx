/*******************************************************************************
 (c) 2005-2015 Copyright, Real-Time Innovations, Inc.  All rights reserved.
 RTI grants Licensee a license to use, modify, compile, and create derivative
 works of the Software.  Licensee has the right to distribute object form only
 for use with RTI products.  The Software is provided "as is", with no warranty
 of any type, including any warranty for fitness for any purpose. RTI is under
 no obligation to maintain or support the Software.  RTI shall not be liable for
 any incidental or consequential damages arising out of the use or inability to
 use the software.
 ******************************************************************************/

#include <algorithm>
#include <iostream>

#include <dds/dds.hpp>
#include "Shapes.hpp"

using namespace dds::core;
using namespace dds::core::cond;
using namespace dds::core::xtypes;
using namespace dds::domain;
using namespace dds::topic;
using namespace dds::sub;
using namespace dds::sub::cond;
using namespace dds::sub::status;

void subscriber_main(int domain_id, int sample_count)
{
    // Create the participant.
    DomainParticipant participant(domain_id);

    // Create DynamicData using the type defined in the IDL file.
    const StructType& shape_type = rti::topic::dynamic_type<ShapeType>::get();

    // If you want to create the type from code instead of using an IDL
    // file with rtiddsgen, comment out the previous declaration and
    // uncomment the following code.

    // StructType shape_type("ShapeType");
    // shape_type.add_member(Member("color", StringType(128)).key(true));
    // shape_type.add_member(Member("x", primitive_type<int32_t>()));
    // shape_type.add_member(Member("y", primitive_type<int32_t>()));
    // shape_type.add_member(Member("shapesize", primitive_type<int32_t>()));

    // Create a Topic -- and automatically register the type.
    // Make sure both publisher and subscriber share the same topic name.
    // In the Shapes example: we are subscribing to a Square, wich is the
    // topic name. If you want to publish other shapes (Triangle or Circle),
    // you just need to update the topic name.
    Topic<DynamicData> topic(participant, "Square", shape_type);

    // Create a DataReader (Subscriber created in-line).
    DataReader<DynamicData> reader(Subscriber(participant), topic);

    // Create a ReadCondition for any data on this reader and associate
    // a handler.
    int count = 0;
    ReadCondition read_condition(
        reader,
        DataState::any(),
        [&reader, &count]()
    {
        // Take all samples
        LoanedSamples<DynamicData> samples = reader.take();
        for (auto sample : samples){
            if (sample.info().valid()){
                count++;
                std::cout << sample.data() << std::endl;
            }
        }
    }
    );

    // Create a WaitSet and attach the ReadCondition
    WaitSet waitset;
    waitset += read_condition;

    while (count < sample_count || sample_count == 0) {
        // Dispatch will call the handlers associated to the WaitSet conditions
        // when they activate
        std::cout << "ShapeType subscriber sleeping for 1 sec..." << std::endl;
        waitset.dispatch(dds::core::Duration(1)); // Wait up to 1s each time
    }
}

int main(int argc, char *argv[])
{
    int domain_id = 0;
    int sample_count = 0; // Infinite loop

    if (argc >= 2) {
        domain_id = atoi(argv[1]);
    }

    if (argc >= 3) {
        sample_count = atoi(argv[2]);
    }

    // To turn on additional logging, include <rti/config/Logger.hpp> and
    // uncomment the following line:
    // rti::config::Logger::instance().verbosity(rti::config::Verbosity::STATUS_ALL);

    try {
        subscriber_main(domain_id, sample_count);
    } catch (const std::exception& ex) {
        // This will catch DDS exceptions
        std::cerr << "Exception in subscriber_main: " << ex.what() << std::endl;
        return -1;
    }

    return 0;
}
