#include "usb.h"
#include "usb_proto.h"
#include "usb_msd.h"
#include "uart.h"
#include "common.h"
#include "wifi.h"

#define CBW_SIGNATURE 		0x43425355
#define CSW_SIGNATURE		0x53425355

#define STATUS_PASSED		0x0
#define STATUS_FAILED		0x1
#define STATUS_PHASE_ERROR	0x2

#define INQUIRY			0x12
#define READ_FORMAT_CAPACITIES	0x23
#define REQUEST_SENSE		0x03
#define MODE_SENSE		0x1a
#define READ_CAPACITY_10	0x25
#define READ_10			0x28
#define WRITE_10		0x2A
#define TEST_UNIT_READY		0x00
#define PREVENT_ALLOW_REMOVAL	0x1e

#define SCSI_SET_SENSE(key, acode, aqual)  { product_sense.sense_key                 = key; \
                                                product_sense.additional_sense_code      = acode; \
                                                product_sense.additional_sense_qualifier = aqual; }

#define SCSI_SENSE_KEY_GOOD                            0x00
#define SCSI_SENSE_KEY_RECOVERED_ERROR                 0x01
#define SCSI_SENSE_KEY_NOT_READY                       0x02
#define SCSI_SENSE_KEY_MEDIUM_ERROR                    0x03
#define SCSI_SENSE_KEY_HARDWARE_ERROR                  0x04
#define SCSI_SENSE_KEY_ILLEGAL_REQUEST                 0x05
#define SCSI_SENSE_KEY_UNIT_ATTENTION                  0x06
#define SCSI_SENSE_KEY_DATA_PROTECT                    0x07
#define SCSI_SENSE_KEY_BLANK_CHECK                     0x08
#define SCSI_SENSE_KEY_VENDOR_SPECIFIC                 0x09
#define SCSI_SENSE_KEY_COPY_ABORTED                    0x0A
#define SCSI_SENSE_KEY_ABORTED_COMMAND                 0x0B
#define SCSI_SENSE_KEY_VOLUME_OVERFLOW                 0x0D
#define SCSI_SENSE_KEY_MISCOMPARE                      0x0E
#define SCSI_ASENSE_NO_ADDITIONAL_INFORMATION          0x00
#define SCSI_ASENSE_LOGICAL_UNIT_NOT_READY             0x04
#define SCSI_ASENSE_INVALID_FIELD_IN_CDB               0x24
#define SCSI_ASENSE_NOT_READY_TO_READY_CHANGE          0x28
#define SCSI_ASENSE_WRITE_PROTECTED                    0x27
#define SCSI_ASENSE_FORMAT_ERROR                       0x31
#define SCSI_ASENSE_INVALID_COMMAND                    0x20
#define SCSI_ASENSE_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE 0x21
#define SCSI_ASENSE_MEDIUM_NOT_PRESENT                 0x3A
#define SCSI_ASENSEQ_NO_QUALIFIER                      0x00
#define SCSI_ASENSEQ_FORMAT_COMMAND_FAILED             0x01
#define SCSI_ASENSEQ_INITIALIZING_COMMAND_REQUIRED     0x02
#define SCSI_ASENSEQ_OPERATION_IN_PROGRESS             0x07

uint32_t current_tag = 0;
uint8_t bulk_buffer[1024];
uint8_t msd_has_status = 0;
uint8_t status = 0;
uint32_t residue = 0;


typedef struct
{
	uint32_t signature;
	uint32_t tag;
	uint32_t length;
	uint8_t flags;
	uint8_t lun;
	uint8_t cb_length;
	uint8_t	cb_data[16];
} PACK cbw_t;

typedef struct
{
	uint32_t signature;
	uint32_t tag;
	uint32_t residue;
	uint8_t status;
} PACK csw_t;

typedef struct
{
	uint8_t op;
	uint8_t evpd;
	uint8_t page;
	uint8_t res;
	uint8_t length;
	uint8_t control;
} PACK inquiry_t;

typedef struct
{
	uint8_t device_type : 5;
	uint8_t peripheral_qualifier : 3;

	uint8_t res1: 7;
	uint8_t rmb : 1;

	uint8_t version;

	uint8_t data_format : 4;
	uint8_t hisup : 1;
	uint8_t normaca : 1;
	uint8_t obs : 1;
	uint8_t aerc : 1;

	uint8_t additional_length;

	uint8_t res2 : 7;
	uint8_t sccs : 1;

	uint8_t addr16 : 1;
	uint8_t obs2 : 1;
	uint8_t obs1 : 1;
	uint8_t mchngr : 1;
	uint8_t multip : 1;
	uint8_t vs : 1;
	uint8_t encserv : 1;
	uint8_t bque : 1;

	uint8_t vs1 : 1;
	uint8_t cmdque : 1;
	uint8_t obs4 : 1;
	uint8_t linked : 1;
	uint8_t sync : 1;
	uint8_t wbus16 : 1;
	uint8_t obs3 : 1;
	uint8_t reladr : 1;

	uint16_t vid;
	uint16_t pid;
	uint16_t rev;

	/*uint8_t vendor_specific[20];

	uint8_t res3 : 4;
	uint8_t clocking : 2;
	uint8_t qas : 1;
	uint8_t ius : 1;

	uint16_t version_descriptor1;
	uint16_t version_descriptor2;
	uint16_t version_descriptor3;
	uint16_t version_descriptor4;
	uint16_t version_descriptor5;
	uint16_t version_descriptor6;
	uint16_t version_descriptor7;
	uint16_t version_descriptor8;

	uint8_t reserved[22];*/
} PACK inquiry_response_t;

typedef struct
{
	uint8_t op;
	uint8_t reserved[6];
	uint16_t allocation_length;
	uint8_t control;
} PACK read_format_t;

typedef struct
{
	uint8_t reserved[3];
	uint8_t list_length;
	uint32_t num_blocks;
	uint8_t type;
	uint8_t block_size_high;
	uint16_t block_size;
} PACK read_format_response_t;

typedef struct
{
	uint8_t op;
	uint8_t desc : 1;
	uint8_t reserved1 : 7;
	uint16_t reserved;
	uint8_t allocation_length;
	uint8_t control;
} PACK request_sense_t;

typedef struct
{
	uint8_t code;
	uint8_t segment;
	uint8_t sense_key : 4;
	uint8_t reserved : 1;
	uint8_t ili : 1;
	uint8_t eom : 1;
	uint8_t file_mark : 1;
	uint8_t information[4];
	uint8_t additional_length;
	uint8_t specific_information[4];
	uint8_t additional_sense_code;
	uint8_t additional_sense_qualifier;
	uint8_t field_replaceable_unit_code;
	uint8_t sense_key_specific[3];
} PACK request_sense_response_t;

typedef struct
{
	uint8_t op;
	uint8_t obs : 1;
	uint8_t fua_nv : 1;
	uint8_t res : 1;
	uint8_t fua : 1;
	uint8_t dpo : 1;
	uint8_t rdprotect : 3;
	uint32_t lba;
	uint8_t group_number : 5;
	uint8_t res2 : 3;
	uint16_t num_blocks;
	uint8_t control;
} PACK read_10_t;

read_format_response_t product_read_format =
{
	.list_length = 0,
	.num_blocks = 1024*1024,
	.type = 2, // formatted media
	.block_size = 512
};

inquiry_response_t product_inquiry =
{
	.peripheral_qualifier = 0,
	.device_type = 0,
	.rmb = 1,
	.version = 0x04,
	.additional_length = sizeof(inquiry_t) - 4,
	.vid = 0xfefe,
	.pid = 0xcdcd,
	.rev = 0x0001
};

request_sense_response_t product_sense =
{
	.code = 0x70,
	.additional_length = 0x0a
};

typedef struct
{
	uint8_t active;
	uint32_t lba;
	uint16_t blocks;
	uint16_t blocks_sent;
} read_operation_t;

read_operation_t read_operation =
{
	.active = 0
};

read_operation_t write_operation =
{
	.active = 0
};

uint8_t usb_msd_has_more_data()
{
	uint32_t offset;
	if(read_operation.blocks_sent == read_operation.blocks)
	{
		return 0;
	}
	bulk_buffer[0] = READ_10;
	offset = read_operation.lba + read_operation.blocks_sent;
	memcpy(&bulk_buffer[1], &offset, 4);

	wifi_send(bulk_buffer, 5);

	if(wifi_recv(bulk_buffer, 512))
	{
	}

	usb_bulk_send(bulk_buffer, 512);
	read_operation.blocks_sent += 1;
	return 1;
}

uint8_t usb_msd_write_active()
{
	return write_operation.active;
}

uint8_t usb_msd_wants_more_data(const uint8_t *data, uint32_t len)
{
	uint32_t offset;
	if(write_operation.blocks_sent == write_operation.blocks)
	{
		write_operation.active = 0;
		return 0;
	}

	// If we are delivered data, relay it to server
	if(data)
	{
		bulk_buffer[0] = WRITE_10;
		offset = write_operation.lba + write_operation.blocks_sent;
		memcpy(&bulk_buffer[1], &offset, 4);
		wifi_send(bulk_buffer, 5);
		wifi_send(data, len);
		write_operation.blocks_sent++;
	}

	// Request a new block

	usb_bulk_receive(bulk_buffer, 512);
	return 1;
}

uint8_t usb_msd_request_sense(const request_sense_t *data)
{
	uint32_t bytes_to_send = MIN(data->allocation_length, sizeof(request_sense_response_t));
	usb_bulk_send((uint8_t*)&product_sense, bytes_to_send);

	return bytes_to_send;
}

uint32_t usb_msd_read_format_capacities(const read_format_t *frm)
{
	uint32_t bytes_to_send = MIN(frm->allocation_length, sizeof(read_format_response_t));
	usb_bulk_send((uint8_t*)&product_read_format, bytes_to_send);

	return bytes_to_send;
}

uint8_t usb_msd_inquiry(const inquiry_t *inq)
{
        if(inq->evpd != 0)
        {
		// Serial
		SCSI_SET_SENSE(SCSI_SENSE_KEY_ILLEGAL_REQUEST, SCSI_ASENSE_INVALID_FIELD_IN_CDB, SCSI_ASENSEQ_NO_QUALIFIER);
		usb_write_endpoint(BULK_IN_ENDPOINT, 0, 0);
		return 0;
        }
        usb_bulk_send((uint8_t*)&product_inquiry, sizeof(inquiry_response_t));

	return sizeof(inquiry_response_t);
}

uint8_t usb_msd_read_capacity_10()
{
	uint32_t last_block = SWAP32(1024*1024-1);
	uint32_t block_size = SWAP32(0x200);

	uint32_t *data = (uint32_t*)bulk_buffer;

	data[0] = last_block;
	data[1] = block_size;
	usb_bulk_send(bulk_buffer, 8);
	return 8;
}

uint8_t usb_msd_mode_sense()
{
	bulk_buffer[0] = 0;
	bulk_buffer[1] = 0;
	bulk_buffer[2] = 0x80; //READ ONLY
	bulk_buffer[3] = 0;
	usb_bulk_send(bulk_buffer, 4);
	return 4;
}

uint32_t usb_msd_read_10(read_10_t *data)
{
	data->num_blocks = SWAP16(data->num_blocks);
	data->lba = SWAP32(data->lba);

	read_operation.active = 1;
	read_operation.lba = data->lba;
	read_operation.blocks = data->num_blocks;
	read_operation.blocks_sent = 0;

	usb_msd_has_more_data();
	return 512;
}

uint32_t usb_msd_write_10(read_10_t *data)
{
	data->num_blocks = SWAP16(data->num_blocks);
	data->lba = SWAP32(data->lba);

	write_operation.active = 1;
	write_operation.lba = data->lba;
	write_operation.blocks = data->num_blocks;
	write_operation.blocks_sent = 0;

	usb_msd_wants_more_data(0x0, 0);
	return 512;
}

void usb_msd_status()
{
	csw_t *csw = (csw_t*)bulk_buffer;
	csw->signature = CSW_SIGNATURE;
	csw->tag = current_tag;
	csw->residue = residue;
	csw->status = status;
	if(status == STATUS_PASSED)
	{
		SCSI_SET_SENSE(SCSI_SENSE_KEY_GOOD, SCSI_ASENSE_NO_ADDITIONAL_INFORMATION, SCSI_ASENSEQ_NO_QUALIFIER);
	}
	usb_bulk_send((uint8_t*)bulk_buffer, sizeof(csw_t));
	msd_has_status = 0;
}


void usb_msd_out(const uint8_t *data, uint32_t len)
{
	//printf("Got MSD data: ");
	//uart_print_hex_str(data, len);
	//printf("\n");
	uint32_t data_sent = 0;
	cbw_t *cbw = (cbw_t*)data;

	current_tag = cbw->tag;

	if(cbw->signature != CBW_SIGNATURE)
	{
		printf("Invalid signature detected\n");
		usb_stall_endpoint(BULK_IN_ENDPOINT);
	}

	switch(cbw->cb_data[0])
	{
	case TEST_UNIT_READY:
	case PREVENT_ALLOW_REMOVAL:
		residue = 0;
		msd_has_status = 1;
		status = STATUS_PASSED;
		usb_msd_status();
		return;
	case READ_10:
		usb_msd_read_10((read_10_t*)cbw->cb_data);
		data_sent = cbw->length;
		break;
	case WRITE_10:
		usb_msd_write_10((read_10_t*)cbw->cb_data);
		data_sent = cbw->length;
	case MODE_SENSE:
		data_sent = usb_msd_mode_sense();
		break;
	case REQUEST_SENSE:
		printf("Request sense\n");
		data_sent = usb_msd_request_sense((request_sense_t*)cbw->cb_data);
		break;
	case INQUIRY:
		data_sent = usb_msd_inquiry((inquiry_t*)cbw->cb_data);
		break;
	case READ_FORMAT_CAPACITIES:
		data_sent = usb_msd_read_format_capacities((read_format_t*)cbw->cb_data);
		break;
	case READ_CAPACITY_10:
		data_sent = usb_msd_read_capacity_10();
		break;
	default:
		printf("Unknown command: ");
		uart_print_hex_str(data, len);
		SCSI_SET_SENSE(SCSI_SENSE_KEY_ILLEGAL_REQUEST, SCSI_ASENSE_INVALID_FIELD_IN_CDB, SCSI_ASENSEQ_NO_QUALIFIER);
		usb_write_endpoint(BULK_IN_ENDPOINT, 0, 0);
		data_sent = 0;
		break;
	}

	residue = cbw->length - data_sent;
	msd_has_status = 1;
	if(data_sent)
	{
		status = STATUS_PASSED;
	}
	else
	{
		status = STATUS_FAILED;
	}
}

