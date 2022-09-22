/*******************************************************************************
 * xor file converter                                                          *
 * 2020-05-31 - fabianofurtado.com                                             *
 *                                                                             *
 * COMPILING: ./make                                                           *
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

int
xor_file( const char *xor_bytes,
          const char *infile,
          const char *outfile,
          uint64_t *offset,
          uint64_t *size )
{
  FILE *fpi,
       *fpo;
  char *endPtr;
  unsigned char c=0;
  uint64_t i,
           j,
           file_len;
  unsigned int key_len;  
  char tmp_key[3] = {'\0'}; // NULL byte

/*
  if ( *offset < 0 ) {
    fprintf( stderr, "[-] \"offset\" cannot be a negative number!\n" );
    return EXIT_FAILURE;
  }
*/
  if ( *offset == 0 )
    fprintf( stdout, "[+] The \"offset\" is not defined.\n" );
  else
    fprintf( stdout, "[+] The \"offset\" is set to %lu...\n", *offset );
/*  
  if ( *size < 0 ) {
    fprintf( stderr, "[-] The \"size\" cannot be a negative number!\n" );
    return EXIT_FAILURE;    
  }
*/
  if ( *size == 0 )
    fprintf( stdout, "[+] The \"size\" is not defined.\n" );
  else
    fprintf( stdout, "[+] The \"size\" is set to %lu...\n", *size );

  key_len = strlen( xor_bytes );
  if ( ( key_len % 2 ) != 0 ) {
    fprintf( stderr,
             "[-] xor bytes \"%s\" length error!\n"
             "    For example, the sequence \"4142\" is iqual to bytes "
             "0x41 and 0x42 (0x4142).\n\n",
             xor_bytes );
    return EXIT_FAILURE;
  }

  key_len = key_len/2;
  // dynamic alloc space on stack for "xor_key" array
  unsigned char xor_key[ key_len ];

  for ( i=0 ; i < key_len ; i++ ) {
    tmp_key[0] = xor_bytes[2*i];
    tmp_key[1] = xor_bytes[2*i+1];
    
    // convert string hex key (2 bytes) to unsigned char xor_key
    xor_key[i] = strtoul( tmp_key, &endPtr, 16 );
    if ( *endPtr ) {
      fprintf( stderr,
               "[-] Unable to convert hex key string \"0x'%2s'\" to "
               "unsigned char.\n",
               tmp_key );
      return EXIT_FAILURE;
    }
  }               
               
  fprintf( stdout, "[+] Opening input file \"%s\"...\n", infile );
  if ( ( fpi = fopen( infile, "rb" ) ) == NULL ) {
    fprintf( stderr,
             "[-] Cannot open input file %s: %s\n",
             infile, strerror(errno) );
    return EXIT_FAILURE;
  }

  fprintf( stdout, "[+] Creating output file \"%s\"...\n", outfile );
  if ((fpo = fopen(outfile, "wb")) == NULL) {
    fprintf( stderr,
             "[-] Cannot open output file %s: %s\n",
             outfile, strerror(errno) );
    fclose( fpi );
    return EXIT_FAILURE;
  }

  //while ((c = (unsigned char)getc(fpi)) != EOF)
  //  putc(c ^ xor_byte, fpo);
  
  fseek( fpi, 0, SEEK_END );
  file_len = ftell( fpi );
  rewind( fpi );

  fprintf( stdout, "[+] File length (bytes): %lu\n", file_len);
  
  if ( *offset >= file_len ) {    
    fclose( fpi );
    fclose( fpo );
    fprintf( stderr,
             "[-] The \"offset\" (%lu bytes) must be less than file "
             "length (%lu bytes)...\n", *offset, file_len );
    return EXIT_FAILURE; 
  }

  if ( *size == 0 ) {
    // The size parameter is not initialized and must be set
    *size = file_len - *offset;
  }
  else {
    // Verify and correct the right size value
    // Example: file size=13 ; offset = 9 ; size = 5
    //          ===> size must be 4
    if ( ( *offset + *size ) > file_len )
      *size = file_len - *offset;
  }

  fprintf( stdout, "[+] Total of bytes to xor: %lu\n", *size);

  fprintf( stdout, "[+] Starting the xor process using \"0x" );
  for (i=0 ; i < key_len ; i++ )
    fprintf( stdout, "%02X", xor_key[i] );
  fprintf( stdout,  "\" key...\n" );
  for( i=0 ; i < file_len ; i=i+key_len ) {
    for (j=0 ; j < key_len ; j++ ) {
      if ( ( i + j ) < file_len ) {
        fread( &c, 1, 1, fpi );
        // Start xor in the offset and size != 0
        if ( ( ( i+j ) >= *offset ) && ( *size ) ) {
          c ^= xor_key[j];
          if (*size)
            (*size)--;
        }
        fwrite( &c, 1, 1, fpo );
      }
    }
  }

  fclose( fpi );
  fclose( fpo );
  fprintf( stdout,
           "[+] Done! Output file \"%s\" has been created!\n\n", outfile );

  return EXIT_SUCCESS;
}

int
main( int argc, char *argv[] )
{
  char *endPtr = NULL;
  uint64_t offset = 0,
           size = 0;

  if ( argc < 4 ) {
    printf( "[-] Error! Usage:\n"
            "      %s <xor bytes in hex> <input file> <output file> "
            "<optional offset> <optional size>\n\n      Example:\n"
            "        %s 414243 file.txt file.txt.xor 123 45\n"
            "      where \"414243\" = \"0x414243\"\n\n",
            argv[0], argv[0] );
    return EXIT_FAILURE;
  }

  // offset
  if ( argc > 4 ) {
    offset = (uint64_t)strtoull( argv[4], &endPtr, 10 );
    if ( offset < 1 ) {
      fprintf( stderr,
               "[-] Unable to convert \"offset\" \"%s\" to integer.\n",
               argv[4] );
      return EXIT_FAILURE;
    }
  }

  // size
  if ( argc > 5 ) {
    size = (uint64_t)strtoull( argv[5], &endPtr, 10 );
    if ( size < 1 ) {
      fprintf( stderr,
               "[-] Unable to convert \"size\" \"%s\" to integer.\n",
               argv[5] );
      return EXIT_FAILURE;
    }
  }

  return xor_file( argv[1], argv[2], argv[3], &offset, &size );
}