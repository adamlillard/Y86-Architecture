/*
 * CS 261 PA1: Mini-ELF header verifier
 *
 * Assignment : Project1
 * Date : 2/22
 * Author : Adam Lillard lillarar@jmu.edu
 * File name : p1-check.c
 */

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "p1-check.h"

void usage_p1 ()
{
    printf("Usage: y86 <option(s)> mini-elf-file\n");
    printf(" Options are:\n");
    printf("  -h      Display usage\n");
    printf("  -H      Show the Mini-ELF header\n");
   // printf("Options must not be repeated neither explicitly nor implicitly.\n");
}

bool parse_command_line_p1 (int argc, char **argv, bool *header, char **file)
{
    // Checks if paramters are Null
    if (!argv || !header  || !file)  
    {
	return false;
    }

    char* optString = "+Hh";
    *header = false;
    char optn;
    *file = NULL;
    // Switch statement which uses getOpt
    while ((optn = getopt(argc, argv, optString)) != -1) 
    {
	switch(optn)
	{
	    case 'H':
		   if (*header == true) { //checks if already passed
			*header = false;
			usage_p1();
			return false;
		   }

		   *header = true;
		   break;		    
		    
	    case 'h':
		    usage_p1();
		    *header = false;
	  	    return true;
	    default: 
		    usage_p1();
		    *header = false;
		    return false;
	}
    }

    if (optind != argc - 1)
    {
	usage_p1();
	return false;
    }

    *file = argv[optind];
    return true;
}

bool read_header (FILE *file, elf_hdr_t *hdr)
{
    if (!file || !hdr)
    {
	return false;
    } else if (fread(hdr, sizeof(elf_hdr_t), 1, file) != 1) {
	return false;
    } else if ( hdr -> magic == 0x00464C45) { // Checks if hdr is equal to magic , allowing to be read
	return true;
    }

    return false;

}

void dump_header (elf_hdr_t hdr)
{
    printf("%s", "00000000  ");
    for (int i = 0; i < 16; i++)
    {
	if (i == 15)
	{
	    printf("%02x", ((uint8_t*)&hdr)[i]);
	} 
	else 
	{
	    printf("%02x ", ((uint8_t*)&hdr)[i]);
	
	}

	if (i == 7) {
	    printf(" ");
	}
    }

    printf("\n");
    printf("%s %d %s", "Mini-ELF version", hdr.e_version, "\n");
    printf("%s %#x %s", "Entry point", hdr.e_entry, "\n");
    printf("There are %d program headers, starting at offset %d (%#x)       \n", hdr.e_num_phdr, hdr.e_phdr_start, hdr.e_phdr_start);
    if (hdr.e_symtab == 0)
    {
	printf("%s", "There is no symbol table present \n");
    }
    
    else 
    {
	printf("There is a symbol table starting at offset %d (%#x) \n", hdr.e_symtab, hdr.e_symtab);
    }
    
    if (hdr.e_strtab == 0) 
    {
	printf("%s", "There is no string table present \n");
    } 
    
    else 
    {
	printf("There is a string table starting at offset %d (%#x) \n", hdr.e_strtab, hdr.e_strtab);
    }
}

