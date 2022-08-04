/*
 * CS 261 PA2: Mini-ELF loader
 *
 * Name: Adam Lillard
 */

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "p2-load.h"

void usage_p2 ()
{
    printf("Usage: y86 <option(s)> mini-elf-file\n");
    printf(" Options are:\n");
    printf("  -h      Display usage\n");
    printf("  -H      Show the Mini-ELF header\n");
    printf("  -a      Show all with brief memory\n");
    printf("  -f      Show all with full memory\n");
    printf("  -s      Show the program headers\n");
    printf("  -m      Show the memory contents (brief)\n");
    printf("  -M      Show the memory contents (full)\n");    
    printf("Options must not be repeated neither explicitly nor implicitly.\n");
}

bool parse_command_line_p2 (int argc, char **argv,
        bool *header, bool *segments, bool *membrief, bool *memfull,
        char **file)
{
    
    char *optStr = "+hHafsmM";
    int opt;
    opterr = 0;
    
    while((opt = getopt(argc, argv, optStr)) != -1)  //Switch statement to use proper Usage case and set attributes
    {
	switch(opt)
	{
	    case 'h' : 
		usage_p2(argv);
		return true;
	    case 'H' :
		if (*header == true)
		{
			usage_p2(argv);
			return false;
		}

		*header = true;
		break;
	    case 'm' :
		if (membrief == true)
		{
		    usage_p2(argv);
		    return false;
		}
		*membrief = true;
		break;
	    case 'M' :
		if (*memfull == true)
		{
		    usage_p2(argv);
		    return false;
		}
		
		*memfull = true;
		break;
	    case 's' : 
		
		if (*segments == true)
		{
		    usage_p2(argv);
		    return false;
		}
		*segments = true;
		break;
	    case 'a' : 
		
		if (*header == true || *segments == true || *membrief == true)
		{
		    usage_p2(argv);
		    return false;
		}
		*header = true;
		*membrief = true;
		*segments = true;
		break;
	    case 'f' : 
		
		if (*header == true || *segments == true || *memfull == true)
		{
		    usage_p2(argv);
		    return false;
		}
		*header = true; 
		*memfull = true;
		*segments = true;
		break;
	    default : 
		usage_p2(argv);
		return false;
	}

    }

    if (*memfull && *membrief)
    {
	usage_p2(argv); // Sees if memfull and brief were checked
	return false;
    }

    if(optind != argc - 1) {
	usage_p2();
	return false;
    }

    *file = argv[optind]; // Gets the file

    return true;

}

bool read_phdr (FILE *file, uint16_t offset, elf_phdr_t *phdr)
{

    if (!file || !offset || !phdr)  //Chekcs for null values
    {
	return false;
    }

    if (fseek(file, offset, SEEK_SET) != 0)
    {
	return false;
    }

    if (fread(phdr, sizeof(elf_phdr_t), 1, file) != 1) 
    {
	return false;
    }

    if (phdr -> magic != 0xDEADBEEF)  //Sees if value is the magic number
    {
	return false;
    }

    return true;
}

void dump_phdrs (uint16_t numphdrs, elf_phdr_t phdr[])
{
    printf("Segment   Offset    VirtAddr  FileSize  Type      Flag\n");
    for (int i = 0; i < numphdrs; i++)
    {
	printf("  %02d      0x%04x    0x%04x    0x%04x", i, phdr[i].p_offset, phdr[i].p_vaddr, phdr[i].p_filesz);
	elf_segtype_t segtype = phdr[i].p_type; // Determines the type
	printf("    ");	
	switch(segtype)
	{
	    case 0:
		printf("DATA      ");
		break;
	    case 1:
		printf("CODE      ");
		break;
	    case 2: 
		printf("STACK     ");
		break;
	    case 3:
		printf("HEAP      ");
		break;
	    default:
		printf("UNKNOWN   ");
		break;
	}

	uint16_t flag = phdr[i].p_flag;
	switch(flag)  //Determines the flag associated
	{
	    case 1 :
		printf("  X");
		break;
	    case 2 :
		printf(" W ");
		break;
	    case 3 :
		printf(" WX");
		break;
	    case 4 :
		printf("R  ");
		break;
	    case 5 :
		printf("R X");
		break;
	    case 6 :
		printf("RW ");
		break;
	    case 7 :
		printf("RWX");
		break;
	    default :
		printf("   ");
		break;
	}

	printf("\n");	
    }

    printf("\n");
}
bool load_segment (FILE *file, memory_t memory, elf_phdr_t phdr)
{
    if (!file || !memory || phdr.p_offset < 0)  //Check for null values
    {
	return false;
    }

    if (fseek(file, phdr.p_offset, SEEK_SET) != 0)
    {
	return false;
    }

    if (phdr.p_vaddr > 4096 || phdr.p_vaddr + phdr.p_filesz > 4096 || phdr.p_vaddr < 0) //Checks if it exceeds MEMVal
    {
	return false;
    }

    if (fread(&memory[phdr.p_vaddr], phdr.p_filesz, 1, file) != 1 && phdr.p_filesz)
    {
	return false;
    }

    if (phdr.p_filesz == 0)
    {
	return true;
    }

    return true;
}

void dump_memory (memory_t memory, uint16_t start, uint16_t end)
{
    printf("Contents of memory from %04x to %04x:", start, end);
    for (int i = start; i < end; i++)
    {
	if (i % 16 == 0)
	{
	    printf("\n  %04x  ", i);
	} else if (i % 8 == 0) {
	    printf(" ");
	}

	if (i % 16 == 0) 
	{
	    printf("%02x", memory[i]);
	} else {
	    printf(" %02x", memory[i]);
	}

    }
    //Prints the memory values according to specifications
    printf("\n\n");
}
