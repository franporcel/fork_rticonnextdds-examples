/*****************************************************************************/
/*         (c) Copyright, Real-Time Innovations, All rights reserved.        */
/*                                                                           */
/*         Permission to modify and use for internal purposes granted.       */
/* This software is provided "as is", without warranty, express or implied.  */
/*                                                                           */
/*****************************************************************************/

#include "FileStreamWriter.hpp"
#include <rti/topic/to_string.hpp>

using namespace rti::community::examples;

const std::string FileStreamWriter::OUTPUT_FILE_PROPERTY_NAME = "example.adapter.output_file";

FileStreamWriter::FileStreamWriter(const PropertySet &property)
{
    std::string outputfilename;
    for (const auto &it: property) {
        if (it.first == OUTPUT_FILE_PROPERTY_NAME) {
            outputfilename = it.second;
            std::cout << "Output file name: " << outputfilename << std::endl;
            output_file_.open(outputfilename);
        }
    }

    if (!output_file_.is_open()) {
        throw std::logic_error("Open output file: " + outputfilename);
    }
};

int FileStreamWriter::write(
        const std::vector<dds::core::xtypes::DynamicData *> &samples,
        const std::vector<dds::sub::SampleInfo *> &infos)
{
    for (std::vector<dds::core::xtypes::DynamicData *>::const_iterator it = samples.begin(); 
            it != samples.end(); 
            ++it) {
        std::string str = rti::topic::to_string(
                (**it), 
                rti::topic::PrintFormatProperty::Default());
        std::cout << "Received Sample: " << std::endl << str << std::endl;

        output_file_ << (*it)->value<std::string>("color") << "," 
                << (*it)->value<int32_t>("x") << "," 
                << (*it)->value<int32_t>("y") << "," 
                << (*it)->value<int32_t>("shapesize") << std::endl;
    }
    return 0;
}