// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "StdOutLogger.h"

#include <iostream>
#include <sstream>
#include <thread>

#define TIMESTAMP_BUFFER_SIZE 25

using namespace std;
using namespace Aws::Iot::DeviceClient::Logging;

void StdOutLogger::writeLogMessage(unique_ptr<LogMessage> message)
{
    char time_buffer[TIMESTAMP_BUFFER_SIZE];
    LogUtil::generateTimestamp(message->getTime(), TIMESTAMP_BUFFER_SIZE, time_buffer);

    cout << time_buffer << " " << LogLevelMarshaller::ToString(message->getLevel()) << " {" << message->getTag()
         << "}: " << message->getMessage() << endl;
}

void StdOutLogger::run()
{
    while (!needsShutdown)
    {
        unique_ptr<LogMessage> message = logQueue->getNextLog();

        if (NULL != message)
        {
            writeLogMessage(std::move(message));
        }
    }
}

bool StdOutLogger::start()
{
    thread log_thread(&StdOutLogger::run, this);
    log_thread.detach();

    return true;
}

void StdOutLogger::shutdown()
{
    needsShutdown = true;
    logQueue->shutdown();

    // If we've gotten here, we must be shutting down so we should dump the remaining messages and exit
    flush();
}

void StdOutLogger::flush()
{
    while (logQueue->hasNextLog())
    {
        unique_ptr<LogMessage> message = logQueue->getNextLog();
        writeLogMessage(std::move(message));
    }
}

void StdOutLogger::queueLog(
    LogLevel level,
    const char *tag,
    std::chrono::time_point<std::chrono::system_clock> t,
    std::string message)
{
    logQueue.get()->addLog(unique_ptr<LogMessage>(new LogMessage(level, tag, t, message)));
}