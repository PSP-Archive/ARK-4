#include "comms.h"
#include "gpio.h"
#include "sysreg.h"
#include "spi.h"

#define SYSCON_MAX_PACKET_SIZE  (0x10)

struct SysconSendPacketHeader {
    unsigned char cmd;
    unsigned char length;
}  __attribute__ ((__packed__));

struct SysconRecvPacketHeader {
    unsigned char cmd;
    unsigned char length;
    unsigned char resp_code;
}  __attribute__ ((__packed__));

static unsigned char calculate_checksum(unsigned char *data, unsigned int length)
{
    unsigned char sum = 0;

    for (unsigned i = 0; i < length; ++i) {
        sum += data[i];
    }

    return ~sum;
}

static void syscon_transmit_packet(unsigned char *packet, unsigned char length)
{
    // calculate a checksum for the packet
    packet[length] = calculate_checksum(packet, length);
    packet[length + 1] = 0xFF;

    // signal syscon that we want to transmit a packet to it
    gpio_clear(GPIO_PORT_SYSCON_REQUEST);

    // read out any data from the SPI shift register
    while (spi_is_data_available(SPI_SYSCON)) {
        (void)spi_read(SPI_SYSCON);
    }

    // clear interrupts in preparation of starting a transaction
    spi_clear_interrupts(SPI_SYSCON);

    // send the packet to syscon
    for (unsigned int i = 0; i < (length + 1); i += 2) {
        spi_write(SPI_SYSCON, (packet[i] << 8) | packet[i + 1]);
    }

    // enable synchronous serial port function and notify syscon
    // that we have sent it our packet
    spi_enable_ssp(SPI_SYSCON);
    gpio_set(GPIO_PORT_SYSCON_REQUEST);
}

static int syscon_receive_packet(unsigned char *packet)
{
    int result = 0;

    // read the response from syscon into our packet buffer
    for (unsigned int i = 0; i < SYSCON_MAX_PACKET_SIZE; i += 2) {
        // if there is no more data to be read then exit
        if (!spi_is_data_available(SPI_SYSCON)) {
            break;
        }

        // read the data from the shift register
        const unsigned int read_data = spi_read(SPI_SYSCON);

        // the first byte (id) is our result
        if (i == 0) {
            result = read_data >> 8;
        }

        packet[i] = read_data >> 8;
        packet[i + 1] = read_data & 0xFF;
    }

    // disable synchronous serial and depress our request call
    spi_disable_ssp(SPI_SYSCON);
    gpio_clear(GPIO_PORT_SYSCON_REQUEST);

    // if there is an error we can now safely exit
    if (result < 0) {
        return result;
    }
    
    // get the length from the packet
    unsigned char length = ((struct SysconRecvPacketHeader *)packet)->length;

    // it should be at least 3 bytes to fit the header and not more (or 
    // equal since there is checksum byte) than the max packet size
    if (length < (sizeof(struct SysconRecvPacketHeader)) ||
        length >= SYSCON_MAX_PACKET_SIZE) {
        return -2;
    }

    // finally if the checksum doesn't match we return an error
    if (calculate_checksum(packet, length) != packet[length]) {
        return -2;
    }

    return result;
}

static int syscon_issue_command(enum SysconCommand cmd, const unsigned char *data, unsigned int length, unsigned char *out_data, unsigned int* out_size)
{
    unsigned char packet[SYSCON_MAX_PACKET_SIZE];
    struct SysconSendPacketHeader *header = (struct SysconSendPacketHeader *)packet;

    // fill the packet with the command data
    header->cmd = cmd;
    header->length = length + sizeof(struct SysconSendPacketHeader);
    for (unsigned int i = 0; i < length; ++i) {
        packet[sizeof(struct SysconSendPacketHeader) + i] = data[i];
    }

retry:
    // send the packet to syscon
    syscon_transmit_packet(packet, header->length);

    // now we poll the syscon GPIO to tell us when it has responded
    while (gpio_query_interrupt(GPIO_PORT_TACHYON_SPI_CS) == 0);
    gpio_acquire_interrupt(GPIO_PORT_TACHYON_SPI_CS);

    // read the result from syscon, and return it to the caller
    const int result = syscon_receive_packet(out_data);

    if (result < 0) {
        return result;
    }

    struct SysconRecvPacketHeader *out_header = (struct SysconRecvPacketHeader *)out_data;
    *out_size = out_header->length - sizeof(struct SysconRecvPacketHeader);
    
    switch (out_data[2]) {
        case 0x80:
        case 0x81:
            goto retry;
        default:
            break;
    }
    return 0;
}

int syscon_issue_command_write(enum SysconCommand cmd, const unsigned char *data, unsigned int length)
{
    unsigned char dummy_packet[SYSCON_MAX_PACKET_SIZE];
    unsigned int dummy_length;
    return syscon_issue_command(cmd, data, length, dummy_packet, &dummy_length);
}

int syscon_issue_command_read(enum SysconCommand cmd, unsigned char *data)
{
    unsigned char recv_packet[SYSCON_MAX_PACKET_SIZE];
    unsigned int recv_length;
    int res = syscon_issue_command(cmd, (void *)0, 0, recv_packet, &recv_length);

    // exit early if we have some error
    if (res < 0) {
        return res;
    }

    // copy data from syscon to the provided parameter
    for (int i = 0; i < recv_length; ++i) {
        data[i] = recv_packet[sizeof(struct SysconRecvPacketHeader) + i];
    }

    return res;
}

int syscon_issue_command_read_write(enum SysconCommand cmd, const unsigned char *data, unsigned int length, unsigned char *out_data)
{
    unsigned char recv_packet[SYSCON_MAX_PACKET_SIZE];
    unsigned int recv_length;
    int res = syscon_issue_command(cmd, (void *)data, length, recv_packet, &recv_length);

    // exit early if we have some error
    if (res < 0) {
        return res;
    }

    // copy data from syscon to the provided parameter
    for (int i = 0; i < recv_length; ++i) {
        out_data[i] = recv_packet[sizeof(struct SysconRecvPacketHeader) + i];
    }

    return recv_length;
}
