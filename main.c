/*
 * CS 261 PA4: Main Driver Code
 *
 * Assignment : Project4
 * Date : 5/4
 * Author : Adam Lillard lillarar@jmu.edu
 * File name : main.c
 */


#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "p1-check.h"
#include "p2-load.h"
#include "p3-disas.h"
#include "p4-interp.h"
#include "y86.h"

int main (int argc, char **argv)
{
    bool header = false;
    bool segments = false;
    bool membrief = false;
    bool memfull = false; //Sets all attributes to false
    bool disas_code = false;
    bool disas_data = false;
    bool exec_normal = false;
    bool exec_debug = false;
    char *file;
    elf_hdr_t hdr; //Creates the elf header file
    
    if (!parse_command_line_p4(argc, argv, &header, &segments, &membrief, &memfull, &disas_code, &disas_data, &exec_normal, &exec_debug,  &file)) 
    {
	exit(EXIT_FAILURE);  //Determines if the parse command line is given valid paramters
    }

    if (file == NULL)
    {
	exit(EXIT_SUCCESS); //Default success
    }
    
    FILE *elfF = fopen(file, "r");
    if (!elfF) 
    {
	printf("Failed to open File\n");
	exit(EXIT_FAILURE);  //Opens elf File and checks for validity
    }

    if (!read_header(elfF, &hdr)) 
    {
	printf("Failed to Read ELF Header\n");
	exit(EXIT_FAILURE);  //Attempts to read header file
    }

    if (header) 
    {
	dump_header(hdr); //Dumps the header file into the hdr for use later
    }

    struct elf_phdr phdrs[hdr.e_num_phdr];
    uint16_t offset = hdr.e_phdr_start; //Creates the offset to start at

	for (int i = 0; i < hdr.e_num_phdr; i++, offset += 20)
	{
	    if (!read_phdr(elfF, offset, &phdrs[i])) //Attempts to read program header
	    {
		printf("Failed to Read Program Header\n");
		exit(EXIT_FAILURE); //Unable to read the header
	    }
	}	
	if (segments)
	{
	    dump_phdrs(hdr.e_num_phdr, phdrs); //Dumps the file into phdrs
	}

	memory_t memory = (memory_t)calloc(MEMSIZE, 1); //Creates memory and callocs it

	for (int i = 0; i < hdr.e_num_phdr; i++) //Attempts to load the segments
	{
	    if (!load_segment(elfF, memory, phdrs[i]))
	    {
		printf("Failed to Load Segment\n");
		exit(EXIT_FAILURE); //Unable to load the segment 
	    }
	    
	    if (membrief)
	    {
		dump_memory(memory, phdrs[i].p_vaddr, phdrs[i].p_vaddr + phdrs[i].p_filesz);
	    } // Dumps a brief description of the memory
	}

	if (memfull) 
	{
	    dump_memory(memory, 0, MEMSIZE); //Dumps the full memory contents
	}

	if (disas_code) //New parameter for P3, determines if disassemble code should be used
	{
	    printf("Disassembly of executable contents:\n");
	    for (int i = 0; i < hdr.e_num_phdr; i++) {
		if (phdrs[i].p_type == CODE) {
			disassemble_code(memory, &phdrs[i], &hdr);
			printf("\n");
		}

	    }


	
	}

	if (disas_data) 
	{
		printf("Disassembly of data contents:\n");
		for (int i = 0; i < hdr.e_num_phdr; i++) {
		    if(phdrs[i].p_type == DATA && phdrs[i].p_flag == 4) {
			disassemble_rodata(memory, &phdrs[i]);
			printf("\n");
		    } else if (phdrs[i].p_type == DATA && phdrs[i].p_flag == 6) {
			disassemble_data(memory, &phdrs[i]);
			printf("\n");
		    }
		}

	}

    y86_t cpu;
    memset(&cpu, 0, sizeof(cpu));
    int count = 0;
    y86_register_t valA = 0;
    y86_register_t valE = 0;
    bool cond = false;
    cpu.pc = hdr.e_entry;
    cpu.stat = AOK;
	y86_inst_t inst;
    if (exec_normal)
    {
	printf("Entry execution point at 0x%04x\n", hdr.e_entry);
	printf("Initial ");
	dump_cpu(&cpu);
	while(cpu.stat == AOK) {
	    inst = fetch(&cpu, memory);
	    valE = decode_execute(&cpu, &cond, &inst, &valA);
	    memory_wb_pc(&cpu, memory, cond, &inst, valE, valA);
	    count++;	

	    if (cpu.pc >= MEMSIZE) {
	        cpu.stat = ADR;
	    }

	    if (inst.type == INVALID) {
		printf("Corrupt Instruction (opcode 0x%02x) at address 0x%04lx\n", inst.opcode, cpu.pc);
	    }

        }

	if(count >= 1 && inst.type != INVALID) {
		printf("Post-Exec ");
	} else {
		printf("Post-Fetch ");
	}

  
	dump_cpu(&cpu);
	printf("Total execution count: %d instructions\n", count);
	printf("\n");
    }



    if (exec_debug) {
	printf("Entry execution point at 0x%04x\n", hdr.e_entry);
	printf("Initial ");
	while(cpu.stat == AOK)
	{
	    dump_cpu(&cpu);
	    y86_inst_t ins = fetch(&cpu, memory);
	    printf("Executing: ");
	    disassemble(ins);
	    printf("\n");
	    

	    valE = decode_execute(&cpu, &cond, &ins, &valA);
	    memory_wb_pc(&cpu, memory, cond, &ins, valE, valA);
	    if (count >= 0) {
		printf("Post-Exec ");
	    }

	    count++;

	    if(cpu.pc >= MEMSIZE) {
		cpu.stat = ADR;
		cpu.pc = 0xffffffffffffffff;
	    }
	}
	dump_cpu(&cpu);
	printf("Total execution count: %d instructions\n", count);
	printf("\n");
	dump_memory(memory, 0, MEMSIZE);

    }

    fclose(elfF);
    free(memory);

    return EXIT_SUCCESS;

}

