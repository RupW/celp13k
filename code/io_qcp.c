/* io_qcp.c - Simple QCP (RFC 3625) file I/O  */

/*
 * For now these use the existing packet->data format, which is big endian-
 * shorts extended to and stored as ints. Even though this is silly.
 *
 * Also for now we store QCP file information between calls as global state.
 * We assume we're only ever reading or writing one QCP file at once.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "coder.h"

#define qcp_buffer_size 16384
static FILE *qcp_file;
static char qcp_buffer[qcp_buffer_size];
static int qcp_cursor;
static int qcp_buffer_end;
static int qcp_logical_length;
static int qcp_have_fmt;
static int qcp_have_vrat;
static int qcp_packet_count;
static int qcp_packets_left;

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

  if (f != NULL)
  {
    qcp_file = f;
    qcp_cursor = 0;
    qcp_buffer_end = 0;
    qcp_have_fmt = 0;
    qcp_have_vrat = 0;

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
    /* Assuming we're little-endian! */
    qcp_logical_length = qcp_cursor + *((int32_t*)(qcp_buffer + 4));

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

    int bytes_left;
    switch (mode)
    {
    case BLANK:
    case ERASURE:
      bytes_left = 0;
      break;
    case EIGHTH:
      bytes_left = 3;
      break;
    case QUARTERRATE_UNVOICED:
    case QUARTERRATE_VOICED:
      bytes_left = 7;
      break;
    case HALFRATE_VOICED:
      bytes_left = 16;
      break;
    case FULLRATE_VOICED:
      bytes_left = 34;
      break;
    default:
      printf("Unknown packet type %d\n", mode);
      fclose(qcp_file);
      exit(1);
    }

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


// open_qcp_output_file
void open_qcp_output_file(
  FILE **fout,
  char *filename
  )
{
  printf("open_qcp_output_file not implemented\n");
  exit(1);
}

int write_qcp_packet(
  FILE  *fout,
  int   *outbuf,
  int   outsamples
  )
{
  printf("write_qcp_packet not implemented\n");
  exit(1);
}
