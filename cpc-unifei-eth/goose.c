#include "goose.h"
#include <string.h>

goose_pdu pdu_tmp;
uint8_t* pdu_tmp_bytes;
//goose_all_data all_data_tmp;

void create_goose_field(goose_field* out_field, uint8_t type, uint8_t* data, uint32_t len);
size_t get_goose_field_size(goose_field* field);
size_t get_goose_pdu_size(goose_pdu* pdu);
void serialize_goose_field(goose_field* field, uint8_t** buffer_ptr);
uint8_t* serialize_goose_pdu(goose_pdu* pdu, size_t* out_len);
size_t get_goose_frame_size(goose_frame* frame);
uint8_t* serialize_goose_frame(goose_frame* frame, size_t* out_len);

uint32_t goose_set_field(goose_buffer* buf, uint8_t field_id, uint8_t* data, uint8_t len)
{
	uint32_t ret = 0;
    
	goose_field *field = NULL;
    
	switch (field_id)
	{
	case GOCBREF_FIELD:
		field = &pdu_tmp.gocbref;
		create_goose_field(field, GOCBREF_TAG, data, len);
		break;

	case TIME_ALLOWED_TO_LIVE_FIELD:
		field = &pdu_tmp.time_allowed_to_live;
		create_goose_field(field, TIME_ALLOWED_TO_LIVE_TAG, data, len);
		break;

	case DATASET_FIELD:
		field = &pdu_tmp.dataset;
		create_goose_field(field, DATASET_TAG, data, len);
		break;

	case GOID_FIELD:
		field = &pdu_tmp.go_id;
		create_goose_field(field, GOID_TAG, data, len);
		break;

	case T_FIELD:
		field = &pdu_tmp.t;
		create_goose_field(field, T_TAG, data, len);
		break;

	case STNUM_FIELD:
		field = &pdu_tmp.st_num;
		create_goose_field(field, STNUM_TAG, data, len);
		break;

	case SQNUM_FIELD:
		field = &pdu_tmp.sq_num;
		create_goose_field(field, SQNUM_TAG, data, len);
		break;

	case SIMULATION_FIELD:
		field = &pdu_tmp.simulation;
		create_goose_field(field, SIMULATION_TAG, data, len);
		break;

	case CONFREV_FIELD:
		field = &pdu_tmp.conf_rev;
		create_goose_field(field, CONFREV_TAG, data, len);
		break;

	case NDSCON_FIELD:
		field = &pdu_tmp.nds_com;
		create_goose_field(field, NDSCON_TAG, data, len);
		break;

	case NUMDATASETENTRIES_FIELD:
		field = &pdu_tmp.num_dataset_entries;
		create_goose_field(field, NUMDATASETENTRIES_TAG, data, len);
		break;
		
	case ALLDATA_FIELD:
		field = &pdu_tmp.all_data;
		create_goose_field(field, ALLDATA_TAG, data, len);
		break;
		
	case PDU_FIELD:
		field = &buf->frame.pdu;
		size_t pdu_len;
		pdu_tmp_bytes = serialize_goose_pdu(&pdu_tmp, &pdu_len);
		create_goose_field(field, PDU_TAG, pdu_tmp_bytes, pdu_len);

	default:
		ret = 1; // Invalid field_id
		break;
	}
    
	return ret;
}

void create_goose_field(goose_field* out_field, uint8_t type, uint8_t* data, uint32_t len)
{
	// Determine the size of the length field
	uint32_t len_len;
	if (len < 128)
	{
		// Short form: only 1 byte needed for length
		len_len = 1;
		out_field->len = (uint8_t*)malloc(len_len);
		if (out_field->len == NULL) return;
		out_field->len[0] = (uint8_t)len;
	}
	else if (len <= 255)
	{
		// Long form: 2 bytes needed for length
		len_len = 2;
		out_field->len = (uint8_t*)malloc(len_len);
		if (out_field->len == NULL) return;
		out_field->len[0] = 0x81; // Long form with 1 additional byte
		out_field->len[1] = (uint8_t)len;
	}
	else if (len <= 65535)
	{
		// Long form: 3 bytes needed for length
		len_len = 3;
		out_field->len = (uint8_t*)malloc(len_len);
		if (out_field->len == NULL) return;
		out_field->len[0] = 0x82; // Long form with 2 additional bytes
		out_field->len[1] = (uint8_t)((len >> 8) & 0xFF); // High byte
		out_field->len[2] = (uint8_t)(len & 0xFF); // Low byte
	}
	else
	{
		// Long form: 4 bytes needed for length
		len_len = 4;
		out_field->len = (uint8_t*)malloc(len_len);
		if (out_field->len == NULL) return;
		out_field->len[0] = 0x83; // Long form with 3 additional bytes
		out_field->len[1] = (uint8_t)((len >> 16) & 0xFF); // High byte
		out_field->len[2] = (uint8_t)((len >> 8) & 0xFF); // Mid byte
		out_field->len[3] = (uint8_t)(len & 0xFF); // Low byte
	}

	// Allocate memory for the value field and copy the data
	out_field->value = (uint8_t*)malloc(len);
	if (out_field->value == NULL)
	{
		free(out_field->len); // Free memory if allocation fails
		return;
	}
	memcpy(out_field->value, data, len);
	out_field->value_len = len;
	out_field->len_len = len_len;
	out_field->type = type;
}

// Helper function to calculate the size of a single goose_field
size_t get_goose_field_size(goose_field* field)
{
	return sizeof(field->type) + field->len_len + field->value_len;
}

// Helper function to calculate the total size of the goose_pdu structure
size_t get_goose_pdu_size(goose_pdu* pdu)
{
	size_t total_size = 0;
	total_size += get_goose_field_size(&pdu->gocbref);
	total_size += get_goose_field_size(&pdu->time_allowed_to_live);
	total_size += get_goose_field_size(&pdu->dataset);
	total_size += get_goose_field_size(&pdu->go_id);
	total_size += get_goose_field_size(&pdu->t);
	total_size += get_goose_field_size(&pdu->st_num);
	total_size += get_goose_field_size(&pdu->sq_num);
	total_size += get_goose_field_size(&pdu->simulation);
	total_size += get_goose_field_size(&pdu->conf_rev);
	total_size += get_goose_field_size(&pdu->nds_com);
	total_size += get_goose_field_size(&pdu->num_dataset_entries);
	return total_size;
}

// Function to serialize a single goose_field into the buffer
void serialize_goose_field(goose_field* field, uint8_t** buffer_ptr)
{
	uint8_t* ptr = *buffer_ptr;

	// Copy the type field
	memcpy(ptr, &field->type, sizeof(field->type));
	ptr += sizeof(field->type);

	// Copy the len field
	memcpy(ptr, field->len, field->len_len);
	ptr += field->len_len;

	// Copy the value field
	memcpy(ptr, field->value, field->value_len);
	ptr += field->value_len;

	// Update the buffer pointer to the new position
	*buffer_ptr = ptr;
}

// Function to serialize the entire goose_pdu into a byte array
uint8_t* serialize_goose_pdu(goose_pdu* pdu, size_t* out_len)
{
	// Calculate the total size of the serialized PDU
	size_t total_size = get_goose_pdu_size(pdu);

	// Allocate a buffer to hold the serialized data
	uint8_t* buffer = (uint8_t*)malloc(total_size);
	if (!buffer) return NULL;  // Handle memory allocation failure

	// Pointer to track the current position in the buffer
	uint8_t* buffer_ptr = buffer;

	// Serialize each goose_field
	serialize_goose_field(&pdu->gocbref, &buffer_ptr);
	serialize_goose_field(&pdu->time_allowed_to_live, &buffer_ptr);
	serialize_goose_field(&pdu->dataset, &buffer_ptr);
	serialize_goose_field(&pdu->go_id, &buffer_ptr);
	serialize_goose_field(&pdu->t, &buffer_ptr);
	serialize_goose_field(&pdu->st_num, &buffer_ptr);
	serialize_goose_field(&pdu->sq_num, &buffer_ptr);
	serialize_goose_field(&pdu->simulation, &buffer_ptr);
	serialize_goose_field(&pdu->conf_rev, &buffer_ptr);
	serialize_goose_field(&pdu->nds_com, &buffer_ptr);
	serialize_goose_field(&pdu->num_dataset_entries, &buffer_ptr);

	// Return the size of the serialized data
	*out_len = total_size;
	return buffer;
}

// Helper function to calculate total size of goose_frame
size_t get_goose_frame_size(goose_frame* frame)
{
	size_t total_size = 0;
	total_size += MAC_ADDRESS_SIZE; // source_mac_address
	total_size += MAC_ADDRESS_SIZE; // destination_mac_address
	total_size += ETHER_TYPE_SIZE; // ether_type
	total_size += APP_ID_SIZE; // app_id
	total_size += LEN_SIZE; // len
	total_size += RESERVED_1_SIZE; // reserved_1
	total_size += RESERVED_2_SIZE; // reserved_2
	total_size += get_goose_field_size(&frame->pdu); // pdu field (dynamic)
	return total_size;
}

uint8_t* serialize_goose_frame(goose_frame* frame, size_t* out_len)
{
	// Calculate the total size of the serialized frame
	size_t total_size = get_goose_frame_size(frame);

	// Allocate memory for the byte array
	uint8_t* buffer = (uint8_t*)malloc(total_size);
	if (!buffer) return NULL;  // Handle memory allocation failure

	// Pointer to keep track of the current position in the buffer
	uint8_t* ptr = buffer;

	// Copy fixed-length fields
	memcpy(ptr, frame->source_mac_address, MAC_ADDRESS_SIZE);
	ptr += MAC_ADDRESS_SIZE;

	memcpy(ptr, frame->destination_mac_address, MAC_ADDRESS_SIZE);
	ptr += MAC_ADDRESS_SIZE;

	memcpy(ptr, frame->ether_type, ETHER_TYPE_SIZE);
	ptr += ETHER_TYPE_SIZE;

	memcpy(ptr, frame->app_id, APP_ID_SIZE);
	ptr += APP_ID_SIZE;

	memcpy(ptr, frame->len, LEN_SIZE);
	ptr += LEN_SIZE;

	memcpy(ptr, frame->reserved_1, RESERVED_1_SIZE);
	ptr += RESERVED_1_SIZE;

	memcpy(ptr, frame->reserved_2, RESERVED_2_SIZE);
	ptr += RESERVED_2_SIZE;

	// Serialize the pdu field (which is a goose_field)
	serialize_goose_field(&frame->pdu, &ptr);

	// Return the size of the serialized data
	*out_len = total_size;

	// Return the serialized byte array
	return buffer;
}

uint32_t goose_get_bytes(goose_buffer* buf, uint8_t** out_bytes, size_t* out_size)
{
	*out_bytes = serialize_goose_frame(&(buf->frame), out_size);
	
	return 0;
}