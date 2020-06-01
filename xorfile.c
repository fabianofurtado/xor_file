/*
 * xor file converter
 * 2020-05-31 - fabianofurtado.com
 * 
 * gcc -Wall -O3 -z noseparate-code xorfile.c -o xorfile
 * 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

int xor_file(const char *xor_bytes,
             const char *infile,
             const char *outfile,
             uint64_t *offset,
             uint64_t *size )
{
  FILE *fpi,
       *fpo;
  char *endPtr;
  unsigned char *xor_key,
                c=0;
  uint64_t i,
           j,
           file_len;
  unsigned int key_len;  
  char tmp_key[3] = { 0, 0, 0 }; // NULL byte

  if ( *offset < 0 ) {
    puts( "[-] 'offset' cannot be a negative number!" );
    return 1;    
  }
  if ( *offset )
    printf( "[+] The 'offset' is set to %lu...\n", *offset );
  else
    puts( "[+] The 'offset' is not defined." );
  
  if ( *size < 0 ) {
    puts( "[-] 'size' cannot be a negative number!" );
    return 1;    
  }
  if ( *size )
    printf( "[+] The 'size' is set to %lu...\n", *size );
  else
    puts( "[+] The 'size' is not defined." );

  key_len = strlen(xor_bytes);
  if ( ( key_len % 2 ) != 0 ) {
    puts( "[-] xor bytes length error!\n" );
    return 1;    
  }

  key_len = key_len/2;
  // alloc space on heap for xor_key
  xor_key = malloc( key_len );
  if ( xor_key == NULL ) {
    puts( "[-] Cannot alloc memory for xor key!" );
    return 1;
  }

  for ( i=0 ; i < key_len ; i++ ) {
    tmp_key[0] = xor_bytes[2*i];
    tmp_key[1] = xor_bytes[2*i+1];
    
    // convert string hex key (2 bytes) to unsigned char xor_key
    xor_key[i] = strtoul( tmp_key, &endPtr, 16);
    if ( *endPtr ) {
      printf( "Unable to convert hex key string 0x'%2s' to unsigned char.\n", tmp_key );
      free( xor_key );
      return EXIT_FAILURE;
    }
  }               
               
  printf( "[+] Opening input file %s...\n", infile );
  if ( ( fpi = fopen( infile, "rb" ) ) == NULL ) {
    printf( "[-] Cannot open input file %s: %s\n", infile, strerror(errno) );
    free( xor_key );
    return 1;
  }

  printf( "[+] Creating output file %s...\n", outfile );
  if ((fpo = fopen(outfile, "wb")) == NULL) {
    printf( "[-] Cannot open output file %s: %s\n", outfile, strerror(errno) );
    free( xor_key );
    fclose( fpi );
    return 1;
  }

  //while ((c = (unsigned char)getc(fpi)) != EOF)
  //  putc(c ^ xor_byte, fpo);
  
  fseek( fpi, 0, SEEK_END );
  file_len = ftell( fpi );
  rewind( fpi );

  printf("[+] File length: %lu\n", file_len);
  
  if ( *offset >= file_len ) {    
    free( xor_key );
    fclose( fpi );
    fclose( fpo );
    printf("[-] The 'offset' (%lu bytes) must be less than file length (%lu bytes)...\n", *offset, file_len);
    return 1; 
  }
  
  if ( *size ) {
    // Verify and correct the right size value
    // Example: file size=13 ; offset = 9 ; size = 5
    //          ===> size must be 4
    if ( ( *offset + *size ) > file_len )
      *size = file_len - *offset;
  }
  else
    // The size parameter is not initialized and must be set
    *size = file_len - *offset;
  
  printf("[+] Total of bytes to xor: %lu\n", *size);

  printf( "[+] Starting the xor process using 0x" );
  for (i=0 ; i < key_len ; i++ )
    printf("%02X", xor_key[i] );
  puts( " key..." );
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

  free( xor_key );
  fclose( fpi );
  fclose( fpo );
  printf( "[+] Done! Output file '%s' has been created!\n\n", outfile );

  return 0;
}

int main( int argc, char *argv[] )
{
  char *endPtr = NULL;
  uint64_t offset = 0,
           size = 0;

  if (argc < 4) {
    printf("[-] Error! Usage:\n"
           "      %s <xor bytes in hex> <input file> <output file> <optional offset> <optional size>\n\n"
           "      Example:\n"
           "        %s AABBCCDD file.txt file.txt.xor 123 34\n\n",
           argv[0], argv[0]);
    return EXIT_FAILURE;
  }

  // offset
  if ( argc > 4 ) {
    offset = (uint64_t)strtoull( argv[4], &endPtr, 10 );
    if ( offset < 1 ) {
      printf( "[-] Unable to convert 'offset' '%s' to integer.\n", argv[4] );
      return EXIT_FAILURE;
    }
  }

  // size
  if ( argc > 5 ) {
    size = (uint64_t)strtoull( argv[5], &endPtr, 10 );
    if ( size < 1 ) {
      printf( "[-] Unable to convert 'size' '%s' to integer.\n", argv[5] );
      return EXIT_FAILURE;
    }
  }

  return xor_file( argv[1], argv[2], argv[3], &offset, &size );
}
