#include <ibus.h>
#include <stdint.h>

int IBUS::update()
{
    readBuffer();
    processChannels();
    return 1;
}
int IBUS::readBuffer()
{
    if (_ibusRX->available() >= 32)
    {
        if (_ibusRX->read() == 0x20)
        {
            uint8_t i;
            for (i = 1; i < 32; i++)
            {
                buffer[i] = _ibusRX->read();
            }
            return 1;
        } else {
            return 0;
        }
    }
    else {
        return 0;
    }
}

int IBUS::processChannels()
{
    for (int channel_index = 0, byte_position = 2; channel_index < 14; channel_index++, byte_position += 2)
    {
        channels[channel_index] = buffer[byte_position + 1] << 8 | buffer[byte_position];
    }
    return 1;
}
