/*******************************************************************************
 (c) 2005-2014 Copyright, Real-Time Innovations, Inc.  All rights reserved.
 RTI grants Licensee a license to use, modify, compile, and create derivative
 works of the Software.  Licensee has the right to distribute object form only
 for use with RTI products.  The Software is provided "as is", with no warranty
 of any type, including any warranty for fitness for any purpose. RTI is under
 no obligation to maintain or support the Software.  RTI shall not be liable for
 any incidental or consequential damages arising out of the use or inability to
 use the software.
 ******************************************************************************/
/* ordered_group_subscriber.c

   A subscription example

   This file is derived from code automatically generated by the rtiddsgen 
   command:

   rtiddsgen -language C -example <arch> ordered_group.idl

   Example subscription of type ordered_group automatically generated by 
   'rtiddsgen'. To test them, follow these steps:

   (1) Compile this file and the example publication.

   (2) Start the subscription with the command
   objs/<arch>/ordered_group_subscriber <domain_id> <sample_count>

   (3) Start the publication with the command
   objs/<arch>/ordered_group_publisher <domain_id> <sample_count>

   (4) [Optional] Specify the list of discovery initial peers and 
   multicast receive addresses via an environment variable or a file 
   (in the current working directory) called NDDS_DISCOVERY_PEERS. 

   You can run any number of publishers and subscribers programs, and can 
   add and remove them dynamically from the domain.


   Example:

   To run the example application on domain <domain_id>:

   On UNIX systems: 

   objs/<arch>/ordered_group_publisher <domain_id> 
   objs/<arch>/ordered_group_subscriber <domain_id> 

   On Windows systems:

   objs\<arch>\ordered_group_publisher <domain_id>  
   objs\<arch>\ordered_group_subscriber <domain_id>   


   modification history
   ------------ -------       
 */

#include <stdio.h>
#include <stdlib.h>
#include "ndds/ndds_c.h"
#include "ordered_group.h"
#include "ordered_groupSupport.h"


void ordered_groupListener_on_data_on_readers(
        void *listener_data, DDS_Subscriber* subscriber)
{
    struct DDS_DataReaderSeq MyDataReaders;	
    DDS_ReturnCode_t retcode;
    int i;

    /* IMPORTANT for GROUP access scope: Invoking 
     * DDS_Subscriber_begin_access() */
    DDS_Subscriber_begin_access(subscriber);

    /* Obtain DataReaders. We obtain a sequence of DataReaders that 
     * specifies the order in which each sample should be read */
    retcode = DDS_Subscriber_get_datareaders(subscriber,
            &MyDataReaders,
            DDS_ANY_SAMPLE_STATE,
            DDS_ANY_VIEW_STATE,
            DDS_ANY_INSTANCE_STATE);
    if (retcode != DDS_RETCODE_OK) {
        printf("ERROR error %d\n", retcode);
        /* IMPORTANT. Remember to invoke DDS_Subscriber_end_access() before a 
         * return call. Also reset DataReaders sequence */
        DDS_DataReaderSeq_ensure_length(&MyDataReaders,0,0);			
        DDS_Subscriber_end_access(subscriber); 
        return;
    }

    /* Read the samples received, following the DataReaders sequence */
    for (i = 0; i < DDS_DataReaderSeq_get_length(&MyDataReaders); i++) {	
        ordered_groupDataReader *ordered_group_reader = NULL;
        struct ordered_group data;		
        struct DDS_SampleInfo info;
        ordered_group_initialize(&data);	

        ordered_group_reader = 
                ordered_groupDataReader_narrow(
                        DDS_DataReaderSeq_get(&MyDataReaders,i));
        if (ordered_group_reader == NULL) {
            printf("DataReader narrow error\n");
            /* IMPORTANT. Remember to invoke DDS_Subscriber_end_access() before 
             * a return call. Also reset DataReaders sequence */
            DDS_DataReaderSeq_ensure_length(&MyDataReaders,0,0);			
            DDS_Subscriber_end_access(subscriber); 
            return;
        }

        /* IMPORTANT. Use take_next_sample(). We need to take only
	   one sample each time, as we want to follow the sequence of 
	   DataReaders. This way the samples will be returned in the
	   order in which they were modified */
        retcode = ordered_groupDataReader_take_next_sample(ordered_group_reader,
                &data, 
                &info);

        /* In case there is no data in current DataReader, 
	   check next in the sequence */
        if (retcode == DDS_RETCODE_NO_DATA) {
            continue;
        } else if (retcode != DDS_RETCODE_OK) {
            printf("take error %d\n", retcode);
            continue;
        } 

        /* Print data sample */
        if (info.valid_data) {
            ordered_groupTypeSupport_print_data(&data);
        }

    }
    /* Reset DataReaders sequence */
    DDS_DataReaderSeq_ensure_length(&MyDataReaders,0,0);			

    /* IMPORTANT for GROUP access scope: Invoking DDS_Subscriber_end_access() */
    DDS_Subscriber_end_access(subscriber); 
}


/* Delete all entities */
static int subscriber_shutdown(DDS_DomainParticipant *participant)
{
    DDS_ReturnCode_t retcode;
    int status = 0;

    if (participant != NULL) {
        retcode = DDS_DomainParticipant_delete_contained_entities(participant);
        if (retcode != DDS_RETCODE_OK) {
            printf("delete_contained_entities error %d\n", retcode);
            status = -1;
        }

        retcode = DDS_DomainParticipantFactory_delete_participant(
                DDS_TheParticipantFactory, participant);
        if (retcode != DDS_RETCODE_OK) {
            printf("delete_participant error %d\n", retcode);
            status = -1;
        }
    }

    /* RTI Connext provides the finalize_instance() method on
       domain participant factory for users who want to release memory used
       by the participant factory. Uncomment the following block of code for
       clean destruction of the singleton. */
    /*
    retcode = DDS_DomainParticipantFactory_finalize_instance();
    if (retcode != DDS_RETCODE_OK) {
        printf("finalize_instance error %d\n", retcode);
        status = -1;
    }
     */

    return status;
}

static int subscriber_main(int domainId, int sample_count)
{
    DDS_DomainParticipant *participant = NULL;
    DDS_Subscriber *subscriber = NULL;

    DDS_Topic *topic1 = NULL;
    DDS_Topic *topic2 = NULL;
    DDS_Topic *topic3 = NULL;

    struct DDS_SubscriberListener subscriber_listener =
            DDS_SubscriberListener_INITIALIZER;

    DDS_DataReader *reader1 = NULL;
    DDS_DataReader *reader2 = NULL;
    DDS_DataReader *reader3 = NULL;

    DDS_ReturnCode_t retcode;
    const char *type_name = NULL;
    int count = 0;
    struct DDS_Duration_t poll_period = {4,0};

    /* To customize participant QoS, use 
       the configuration file USER_QOS_PROFILES.xml */
    participant = DDS_DomainParticipantFactory_create_participant(
            DDS_TheParticipantFactory, domainId, &DDS_PARTICIPANT_QOS_DEFAULT,
            NULL /* listener */, DDS_STATUS_MASK_NONE);
    if (participant == NULL) {
        printf("create_participant error\n");
        subscriber_shutdown(participant);
        return -1;
    }

    /* Set up Subscriber listener and establish on_data_on_readers callback */
    /* Set up a data reader listener */
    subscriber_listener.on_data_on_readers  =
            ordered_groupListener_on_data_on_readers;

    /* To customize subscriber QoS, use 
       the configuration file USER_QOS_PROFILES.xml */
    subscriber = DDS_DomainParticipant_create_subscriber(
            participant, &DDS_SUBSCRIBER_QOS_DEFAULT, 
            &subscriber_listener /* listener */, DDS_DATA_ON_READERS_STATUS);
    if (subscriber == NULL) {
        printf("create_subscriber error\n");
        subscriber_shutdown(participant);
        return -1;
    }

    /* Register the type before creating the topic */
    type_name = ordered_groupTypeSupport_get_type_name();
    retcode = ordered_groupTypeSupport_register_type(participant, type_name);
    if (retcode != DDS_RETCODE_OK) {
        printf("register_type error %d\n", retcode);
        subscriber_shutdown(participant);
        return -1;
    }

    /* TOPICS */ 

    /* To customize topic QoS, use 
       the configuration file USER_QOS_PROFILES.xml */
    topic1 = DDS_DomainParticipant_create_topic(
            participant, "Topic1",
            type_name, &DDS_TOPIC_QOS_DEFAULT, NULL /* listener */,
            DDS_STATUS_MASK_NONE);
    if (topic1 == NULL) {
        printf("create_topic error\n");
        subscriber_shutdown(participant);
        return -1;
    }

    topic2 = DDS_DomainParticipant_create_topic(
            participant, "Topic2",
            type_name, &DDS_TOPIC_QOS_DEFAULT, NULL /* listener */,
            DDS_STATUS_MASK_NONE);
    if (topic2 == NULL) {
        printf("create_topic error\n");
        subscriber_shutdown(participant);
        return -1;
    }

    topic3 = DDS_DomainParticipant_create_topic(
            participant, "Topic3",
            type_name, &DDS_TOPIC_QOS_DEFAULT, NULL /* listener */,
            DDS_STATUS_MASK_NONE);
    if (topic3 == NULL) {
        printf("create_topic error\n");
        subscriber_shutdown(participant);
        return -1;
    }

    /* DATAREADERS */

    /* To customize data reader QoS, use 
       the configuration file USER_QOS_PROFILES.xml */
    reader1 = DDS_Subscriber_create_datareader(
            subscriber, DDS_Topic_as_topicdescription(topic1),
            &DDS_DATAREADER_QOS_DEFAULT, NULL, DDS_STATUS_MASK_NONE);
    if (reader1 == NULL) {
        printf("create_datareader error\n");
        subscriber_shutdown(participant);
        return -1;
    }

    reader2 = DDS_Subscriber_create_datareader(
            subscriber, DDS_Topic_as_topicdescription(topic2),
            &DDS_DATAREADER_QOS_DEFAULT, NULL, DDS_STATUS_MASK_NONE);
    if (reader2 == NULL) {
        printf("create_datareader error\n");
        subscriber_shutdown(participant);
        return -1;
    }

    reader3 = DDS_Subscriber_create_datareader(
            subscriber, DDS_Topic_as_topicdescription(topic3),
            &DDS_DATAREADER_QOS_DEFAULT, NULL, DDS_STATUS_MASK_NONE);
    if (reader3 == NULL) {
        printf("create_datareader error\n");
        subscriber_shutdown(participant);
        return -1;
    }

    /* Main loop */
    for (count=0; (sample_count == 0) || (count < sample_count); ++count) {
        printf("ordered_group subscriber sleeping for %d sec...\n",
                poll_period.sec);
        NDDS_Utility_sleep(&poll_period);
    }

    /* Cleanup and delete all entities */ 
    return subscriber_shutdown(participant);
}

#if defined(RTI_WINCE)
int wmain(int argc, wchar_t** argv)
{
    int domainId = 0;
    int sample_count = 0; /* infinite loop */

    if (argc >= 2) {
        domainId = _wtoi(argv[1]);
    }
    if (argc >= 3) {
        sample_count = _wtoi(argv[2]);
    }

    /* Uncomment this to turn on additional logging
    NDDS_Config_Logger_set_verbosity_by_category(
        NDDS_Config_Logger_get_instance(),
        NDDS_CONFIG_LOG_CATEGORY_API, 
        NDDS_CONFIG_LOG_VERBOSITY_STATUS_ALL);
     */

    return subscriber_main(domainId, sample_count);
}
#elif !(defined(RTI_VXWORKS) && !defined(__RTP__)) && !defined(RTI_PSOS)
int main(int argc, char *argv[])
{
    int domainId = 0;
    int sample_count = 0; /* infinite loop */

    if (argc >= 2) {
        domainId = atoi(argv[1]);
    }
    if (argc >= 3) {
        sample_count = atoi(argv[2]);
    }

    /* Uncomment this to turn on additional logging
    NDDS_Config_Logger_set_verbosity_by_category(
        NDDS_Config_Logger_get_instance(),
        NDDS_CONFIG_LOG_CATEGORY_API, 
        NDDS_CONFIG_LOG_VERBOSITY_STATUS_ALL);
     */

    return subscriber_main(domainId, sample_count);
}
#endif

#ifdef RTI_VX653
const unsigned char* __ctype = NULL;

void usrAppInit ()
{
#ifdef  USER_APPL_INIT
    USER_APPL_INIT;         /* for backwards compatibility */
#endif

    /* add application specific code here */
    taskSpawn("sub", RTI_OSAPI_THREAD_PRIORITY_NORMAL, 0x8, 0x150000, 
            (FUNCPTR)subscriber_main, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

}
#endif
