#ifndef DS1_UTILITY_H
#define DS1_UTILITY_H

void ds1ConfigureHdmiSupportedChannel(int channelCount);

void ds1ConfigureRoutingDevice(unsigned int audioDevice);

bool ds1SendBroadcastMessage(int value);

#endif  /* DS1_UTILITY_H */
