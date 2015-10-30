/* io_qcp.c - Simple QCP (RFC 3625) file I/O  */

/*
 * For now these use the existing packet->data format, which is big endian-
 * shorts extended to and stored as ints. Even though this is silly.
 *
 * Also for now we store QCP file information between calls as global state.
 * We assume we're only ever reading or writing one QCP file at once.
 *
 * For now this all assumes we're running on a little-endian host.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "coder.h"

/* input QCP file state */
#define qcp_buffer_size 16384
static FILE *qcp_file;
/* buffer of QCP data */
static char qcp_buffer[qcp_buffer_size];
/* next byte within QCP buffer */
static int qcp_cursor;
/* number of bytes in the QCP buffer */
static int qcp_buffer_end;
/* number of packets in file (from vrat header) */
static int qcp_packet_count;
/* number of packets left unprocessed */
static int qcp_packets_left;

/* output QCP file state */
/* offset of the 'data' chunk length field in the file, to be filled in at the end */
static int qcp_out_offset_data_length;
/* offset of the 'RIFF' header length field in the file, to be filled in at the end */
static int qcp_out_offset_riff_length;

static const char qcelp_guid_1[16] = { 0x41, 0x6D, 0x7F, 0x5E, 0x15, 0xB1, 0xD0, 0x11, 0xBA, 0x91, 0x00, 0x80, 0x5F, 0xB4, 0xB9, 0x7E };
static const char qcelp_guid_2[16] = { 0x42, 0x6D, 0x7F, 0x5E, 0x15, 0xB1, 0xD0, 0x11, 0xBA, 0x91, 0x00, 0x80, 0x5F, 0xB4, 0xB9, 0x7E };

static int read_qcp_ensure(int bytes)
{
  int bytesLeft = qcp_buffer_end - qcp_cursor;
  if (bytesLeft < bytes)
  {
    if (bytesLeft > 0)
    {
      memmove(qcp_buffer, qcp_buffer + qcp_cursor, bytesLeft);
      qcp_buffer_end = bytesLeft;
    }
    else
    {
      qcp_buffer_end = 0;
    }
    qcp_cursor = 0;

    int bytes_read = fread(qcp_buffer + qcp_buffer_end, 1, qcp_buffer_size - qcp_buffer_end, qcp_file);
    qcp_buffer_end += bytes_read;
    bytesLeft += bytes_read;
  }

  return (bytesLeft >= bytes);
}

void open_qcp_input_file(
  FILE  **fin,
  char *filename
  )
{
  FILE *f = fopen(filename, "rb");

  printf("QCP input file  - %s\n", filename);

  if (f != NULL)
  {
    qcp_file = f;
    qcp_cursor = 0;
    qcp_buffer_end = 0;
    /* fmt and vrat chunks read flags */
    int qcp_have_fmt = 0;
    int qcp_have_vrat = 0;

    if (!read_qcp_ensure(12))
    {
      printf("Input not a QCP file: too small for header and one chunk\n");
      fclose(f);
      exit(1);
    }

    /* RIFF header: RIFF<length>QLCM */
    if (memcmp(qcp_buffer, "RIFF", 4) != 0)
    {
      printf("Input not a QCP file: missing RIFF signature\n");
      fclose(f);
      exit(1);
    }
    if (memcmp(qcp_buffer + 8, "QLCM", 4) != 0)
    {
      printf("Input not a QCP file: missing QLCM RIFF-type\n");
      fclose(f);
      exit(1);
    }
    qcp_cursor = 12;

    /* Read chunks until we find data */
    while (read_qcp_ensure(8))
    {
      int32_t chunk_len = *((int32_t*)(qcp_buffer + qcp_cursor + 4));
      char* chunk_tag = qcp_buffer + qcp_cursor;
      qcp_cursor += 8;
      if (memcmp(chunk_tag, "fmt ", 4) == 0)
      {
        if (qcp_have_fmt)
        {
          printf("Invalid QCP file: multiple 'fmt ' chunks\n");
          fclose(f);
          exit(1);
        }
        if (!read_qcp_ensure(chunk_len))
        {
          printf("Invalid QCP file: end of file in 'fmt ' chunk\n");
          fclose(f);
          exit(1);
        }
        qcp_have_fmt = 1;

        if (((qcp_buffer[qcp_cursor] != 1) && (qcp_buffer[qcp_cursor] != 2))
          || (qcp_buffer[qcp_cursor + 1] != 0))
        {
          printf("Invalid QCP file: unsupported version %d.%d\n", qcp_buffer[qcp_cursor], qcp_buffer[qcp_cursor + 1]);
          fclose(f);
          exit(1);
        }

        if ((memcmp(qcp_buffer + qcp_cursor + 2, qcelp_guid_1, 16) != 0) &&
          (memcmp(qcp_buffer + qcp_cursor + 2, qcelp_guid_2, 16) != 0))
        {
          printf("Invalid QCP file: codec GUID is not QCELP\n");
          fclose(f);
          exit(1);
        }

        /* We'll ignore the rest for now; we'll assume 8 KHz and that packet IDs match QCELP rate IDs */

        qcp_cursor += chunk_len;
      }
      else if (memcmp(chunk_tag, "vrat", 4) == 0)
      {
        if (qcp_have_vrat)
        {
          printf("Invalid QCP file: multiple 'vrat' chunks\n");
          fclose(f);
          exit(1);
        }
        if (!read_qcp_ensure(chunk_len))
        {
          printf("Invalid QCP file: end of file in 'vrat' chunk\n");
          fclose(f);
          exit(1);
        }
        qcp_have_vrat = 1;

        int32_t vrat_flag = *((int32_t*)(qcp_buffer + qcp_cursor));
        qcp_packet_count = *((int32_t*)(qcp_buffer + qcp_cursor + 4));

        if (vrat_flag == 0)
        {
          printf("Invalid QCP file: fixed rate files not supported\n");
          fclose(f);
          exit(1);
        }
        qcp_cursor += chunk_len;
      }
      else if (memcmp(chunk_tag, "data", 4) == 0)
      {
        /* Data chunk */
        if (!(qcp_have_fmt))
        {
          printf("Invalid QCP file: no 'fmt ' chunk before 'data'\n");
          fclose(f);
          exit(1);
        }
        if (!(qcp_have_vrat))
        {
          /* Technically this isn't a problem, but we only support simple files */
          printf("Invalid QCP file: files without a 'vrat' chunk not supported\n");
          fclose(f);
          exit(1);
        }

        qcp_packets_left = qcp_packet_count;
        break;
      }
      else
      {
        // skip
        read_qcp_ensure(chunk_len);
        qcp_cursor += chunk_len;
      }
    }

    /* Valid QCP file and we have read past the headers */
    *fin = qcp_file;
  }
}

int get_qcp_packet_count() {
  return qcp_packet_count;
}

static int mode_to_bytes(int mode)
{
  switch (mode)
  {
  case BLANK:
  case ERASURE:
    return 0;
  case EIGHTH:
    return 3;
  case QUARTERRATE_UNVOICED:
  case QUARTERRATE_VOICED:
    return 7;
  case HALFRATE_VOICED:
    return 16;
  case FULLRATE_VOICED:
    return 34;
  default:
    printf("Unknown packet type %d\n", mode);
    fclose(qcp_file);
    exit(1);
  }
}

int read_qcp_packet(
  FILE  *fin,
  int   *inbuf,
  int   inbufmax
  )
{
  int cursor = 0;
  memset(inbuf, 0, sizeof(int) * inbufmax);

  if ((qcp_packets_left > 0) && read_qcp_ensure(1))
  {
    int mode = qcp_buffer[qcp_cursor] & 0xff;
    inbuf[0] = mode;
    ++cursor;
    ++qcp_cursor;

    int bytes_left = mode_to_bytes(mode);

    if (!read_qcp_ensure(bytes_left))
    {
      printf("End of QCP file mid-packet\n");
      fclose(qcp_file);
      exit(1);
    }

    while ((bytes_left >= 2) && (cursor < inbufmax))
    {
      inbuf[cursor] = (*((unsigned char*)(qcp_buffer + qcp_cursor)) & 0xff) << 8 |
        (*((unsigned char*)(qcp_buffer + qcp_cursor + 1)) & 0xff);
      qcp_cursor += 2;
      ++cursor;
      bytes_left -= 2;
    }
    if ((bytes_left >= 1) && (cursor < inbufmax))
    {
      inbuf[cursor] = (*((unsigned char*)(qcp_buffer + qcp_cursor)) & 0xff) << 8;
      ++qcp_cursor;
      ++cursor;
      --bytes_left;
    }
  }

  return cursor;
}

void open_qcp_output_file(
  FILE **fout,
  char *filename,
  int frame_count
  )
{
  printf("QCP output file  - %s\n", filename);
  if ((*fout = fopen(filename, "wb")) == NULL)
  {
    fprintf(stderr, "ERROR: Unable to open output file %s\n", filename);
    exit(-1);
  }

  char qcp_header_buffer[256];
  int cursor = 0;

  // RIFF header, no length yet
  memcpy(qcp_header_buffer, "RIFF", 4);
  qcp_out_offset_riff_length = 4;
  memcpy(qcp_header_buffer + 8, "QLCM", 4);
  cursor += 12;

  /* fmt header; leave size for now */
  memcpy(qcp_header_buffer + cursor, "fmt ", 4);
  cursor += 8;
  int qcp_chunk_start = cursor;
  /* Format 1.0 */
  qcp_header_buffer[cursor++] = 1;
  qcp_header_buffer[cursor++] = 0;
  /* QCELP GUID */
  memcpy(qcp_header_buffer + cursor, qcelp_guid_1, 16);
  cursor += 16;
  /* Codec version 1 (2 is also allowed: is this code 2 not 1?) */
  *(int16_t*)(qcp_header_buffer + cursor) = 1;
  cursor += 2;
  /* 80-character codec name */
  memset(qcp_header_buffer + cursor, 0, 80);
  strcpy(qcp_header_buffer + cursor, "Qcelp 13K");
  cursor += 80;
  /* Average bitrate 13K */
  *(int16_t*)(qcp_header_buffer + cursor) = 13000;
  cursor += 2;
  /* Maximum packet size - assume will generate at least one full packet for now! */
  *(int16_t*)(qcp_header_buffer + cursor) = mode_to_bytes(FULLRATE_VOICED);
  cursor += 2;
  /* Samples per frame */
  *(int16_t*)(qcp_header_buffer + cursor) = FSIZE;
  cursor += 2;
  /* Sample rate */
  *(int16_t*)(qcp_header_buffer + cursor) = 8000;
  cursor += 2;
  /* Bits per sample */
  *(int16_t*)(qcp_header_buffer + cursor) = 16;
  cursor += 2;
  /* Variable rate table: five rates */
  *(int32_t*)(qcp_header_buffer + cursor) = 5;
  cursor += 4;
  qcp_header_buffer[cursor++] = mode_to_bytes(FULLRATE_VOICED);
  qcp_header_buffer[cursor++] = FULLRATE_VOICED;
  qcp_header_buffer[cursor++] = mode_to_bytes(HALFRATE_VOICED);
  qcp_header_buffer[cursor++] = HALFRATE_VOICED;
  qcp_header_buffer[cursor++] = mode_to_bytes(QUARTERRATE_UNVOICED);
  qcp_header_buffer[cursor++] = QUARTERRATE_UNVOICED;
  qcp_header_buffer[cursor++] = mode_to_bytes(EIGHTH);
  qcp_header_buffer[cursor++] = EIGHTH;
  qcp_header_buffer[cursor++] = mode_to_bytes(BLANK);
  qcp_header_buffer[cursor++] = BLANK;
  /* Pad the table with three more empty entries */
  memset(qcp_header_buffer + cursor, 0, 6);
  cursor += 6;
  /* Five reserved words */
  memset(qcp_header_buffer + cursor, 0, 20);
  cursor += 20;
  /* Chunk complete; fill in the length */
  *(int32_t*)(qcp_header_buffer + qcp_chunk_start - 4) = (cursor - qcp_chunk_start);

  /* vrat chunk, length 8 */
  memcpy(qcp_header_buffer + cursor, "vrat", 4);
  *(int32_t*)(qcp_header_buffer + cursor + 4) = 8;
  /* variable rate flag */
  *(int32_t*)(qcp_header_buffer + cursor + 8) = 1;
  /* frame count */
  *(int32_t*)(qcp_header_buffer + cursor + 12) = frame_count;
  cursor += 16;

  /* data chunk header; leave length blank */
  memcpy(qcp_header_buffer + cursor, "data", 4);
  cursor += 4;
  qcp_out_offset_data_length = cursor;
  memset(qcp_header_buffer + cursor, 0, 4);
  cursor += 4;

  /* Write what we've got so far */
  fwrite(qcp_header_buffer, 1, cursor, *fout);
}

int write_qcp_packet(
  FILE  *fout,
  int   *outbuf,
  int   outsamples
  )
{
  unsigned char buffer[40];
  int mode = outbuf[0];
  int bytes = mode_to_bytes(mode);
  int byte_cursor = 0;
  int packet_cursor = 1;
  int bytes_left = bytes;
  int bytes_written;

  if ((bytes + 1) > sizeof(buffer))
  {
    printf("write_qcp_packet buffer too small!\n");
    exit(-2);
  }

  buffer[byte_cursor++] = (unsigned char)(mode);
  while (bytes_left >= 2)
  {
    int sample = outbuf[packet_cursor++];
    buffer[byte_cursor++] = (unsigned char)((sample >> 8) & 0xff);
    buffer[byte_cursor++] = (unsigned char)(sample & 0xff);
    bytes_left -= 2;
  }
  if (bytes_left >= 1)
  {
    int sample = outbuf[packet_cursor++];
    buffer[byte_cursor++] = (unsigned char)((sample >> 8) & 0xff);
    --bytes_left;
  }

  bytes_written = fwrite(buffer, 1, byte_cursor, fout);

  return (bytes_written + 2) / 2;
}

void finish_qcp_output_file(
  FILE  *fout
  )
{
  long length = ftell(fout);

  /* The QCP "length" header contains the length immediately following the header, which is 4 bytes long. */
  if (qcp_out_offset_data_length > 0)
  {
    fseek(fout, qcp_out_offset_data_length, SEEK_SET);
    int32_t data_length = (length - (qcp_out_offset_data_length + 4));
    fwrite(&data_length, 4, 1, fout);
  }

  /* The RIFF "length" header contains the length immediately following the header, which is 4 bytes long. */
  if (qcp_out_offset_riff_length > 0)
  {
    fseek(fout, qcp_out_offset_riff_length, SEEK_SET);
    int32_t riff_length = (length - (qcp_out_offset_riff_length + 4));
    fwrite(&riff_length, 4, 1, fout);
  }
}
