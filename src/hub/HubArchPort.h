/*
 *  Copyright (c) 2016, Stefan Johnson
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without modification,
 *  are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice, this list
 *     of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright notice, this
 *     list of conditions and the following disclaimer in the documentation and/or other
 *     materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef HackingGame_HubArchPort_h
#define HackingGame_HubArchPort_h

#include <Blob2D/Blob2D.h>

/*!
 * @brief The port connection.
 * @description Allows @b CCRetain.
 */
typedef struct HKHubArchPortConnectionInfo *HKHubArchPortConnection;

typedef uint8_t HKHubArchPortID;

typedef void *HKHubArchPortDevice;

typedef struct {
    uint8_t address;
    uint8_t size;
} HKHubArchPortMessage;

typedef enum {
    /// Operation succeeded
    HKHubArchPortResponseSuccess,
    /// Operation is not possible
    HKHubArchPortResponseTimeout,
    /// Operation requires more cycles
    HKHubArchPortResponseRetry
} HKHubArchPortResponse;

/*!
 * @brief The transmitter protocol to be used for sending or receiving data.
 * @param Connection The connection being used.
 * @param Device The device handling this transmit.
 * @param Message The message that is being sent or the message to be written too.
 * @param ConnectedDevice The device being communicated with.
 * @param Timestamp The cycle timestamp the event originated at.
 */
typedef HKHubArchPortResponse (*HKHubArchPortTransmit)(HKHubArchPortConnection Connection, HKHubArchPortDevice Device, HKHubArchPortMessage *Message, HKHubArchPortDevice ConnectedDevice, size_t Timestamp); //TODO: needs work

/*!
 * @brief Disconnect the device at the given port.
 * @param Port The port being disconnected.
 */
typedef void (*HKHubArchPortDisconnect)(HKHubArchPortID Port);

/*!
 * @brief Destructor to destroy the device.
 * @param Device The device to be destroyed.
 */
typedef void (*HKHubArchPortDeviceDestructor)(HKHubArchPortDevice Device);

typedef struct {
    HKHubArchPortTransmit sender;
    HKHubArchPortTransmit receiver;
    HKHubArchPortDevice device;
    HKHubArchPortDeviceDestructor destructor;
    HKHubArchPortDisconnect disconnect;
    HKHubArchPortID id;
} HKHubArchPort;

typedef struct HKHubArchPortConnectionInfo {
    HKHubArchPort port[2];
} HKHubArchPortConnectionInfo;

extern const CCCollectionElementDestructor HKHubArchPortConnectionDestructorForCollection;
extern const CCDictionaryElementDestructor HKHubArchPortConnectionDestructorForDictionary;


/*!
 * @brief Create a connection between ports.
 * @param Allocator The allocator to be used.
 * @param PortA The description of one of the connected device ports.
 * @param PortB The description of the other connected device port.
 * @return The port connection. Must be destroyed to free memory.
 */
CC_NEW HKHubArchPortConnection HKHubArchPortConnectionCreate(CCAllocatorType Allocator, HKHubArchPort PortA, HKHubArchPort PortB);

/*!
 * @brief Destroy a port connection.
 * @warning Disconnects the connected devices if it hasn't been already.
 * @param Connection The port connection to be destroyed.
 */
void HKHubArchPortConnectionDestroy(HKHubArchPortConnection CC_DESTROY(Connection));

/*!
 * @brief Disconnect a port connection.
 * @param Connection The port connection to be disconnected.
 */
void HKHubArchPortConnectionDisconnect(HKHubArchPortConnection Connection);

#endif