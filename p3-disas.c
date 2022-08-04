/*
 * CS 261 PA3: Disassembler Code
 *
 * Assignment : Project3
 * Date : 4/13
 * Author : Adam Lillard lillarar@jmu.edu
 * File name : p3-disas.c
 */


#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "p3-disas.h"

//============================================================================
void usage_p3 ()
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
    printf("  -d      Disassemble code contents\n");
    printf("  -D      Disassemble data contents\n");
    printf("Options must not be repeated neither explicitly nor implicitly.\n");
}

//============================================================================
bool parse_command_line_p3 (int argc, char **argv,
        bool *header, bool *segments, bool *membrief, bool *memfull,
        bool *disas_code, bool *disas_data, char **file)
{
    char *optStr = "+hHafsmMDd";
    int opt;
    opterr = 0;

    while((opt = getopt(argc, argv, optStr)) != -1)  //Switch statement to use >
    {
        switch(opt)
        {
            case 'h' : //Displays the usage
                usage_p3(argv);
                return true;
            case 'H' : // Shows ELF header
                if (*header == true)
                {
		     usage_p3(argv);
                        return false;
                }

                *header = true;
                break;
            case 'm' : //Shows brief memory 
                if (membrief == true)
                {
                    usage_p3(argv);
                    return false;
                }
                *membrief = true;
                break;
            case 'M' : //Shows full memory
                if (*memfull == true)
                {
                    usage_p3(argv);
                    return false;
                }

		*memfull = true;
                break;
            case 's' : //Shows the headers

                if (*segments == true)
                {
                    usage_p3(argv);
                    return false;
                }
                *segments = true;
                break;
            case 'a' : //Shows all the brief memory

                if (*header == true || *segments == true || *membrief == true)
                {
                    usage_p3(argv);
                    return false;
                }
                
		*header = true;
		 *membrief = true;
                *segments = true;
                break;
            case 'f' : //Shows all with full memory

                if (*header == true || *segments == true || *memfull == true)
                {
                    usage_p3(argv);
                    return false;
                }
                *header = true;
                *memfull = true;
                *segments = true;
                break;

	    case 'd' : //New command line which disassembles data
		if (*disas_code == true) 
		{
		    usage_p3(argv);
		    return false;
		}

		*disas_code = true;
		break;
	    case 'D' : //New command line which disassembles code
		if (*disas_data == true)
		{
		    usage_p3(argv);
		    return false;
		}
		
		*disas_data = true;
		break;
            default : //Default case
                usage_p3(argv);
                return false;
        }

    }

    if (*memfull && *membrief)
    {
        usage_p3(argv); // Sees if memfull and brief were checked
        return false;
    }

    if(optind != argc - 1) {
        usage_p3();
        return false;
    }

    *file = argv[optind]; // Gets the file

    return true;


}

//============================================================================
y86_inst_t fetch (y86_t *cpu, memory_t memory)
{
    y86_inst_t ins;

    cpu->stat = AOK;
    // Initialize the instruction
    memset( &ins , 0 , sizeof(y86_inst_t) );  // Clear all fields i=on instr.
    ins.type = INVALID;   // Invalid instruction until proven otherwise

    if( memory == NULL || cpu->pc >= MEMSIZE || cpu ->pc  < 0 )    // Is it outside the  Address Space?
    {
        return ins ;        // an INVALID instruction
    }
    ins.opcode = memory[ cpu->pc ] ; // fetch 1st byte of instr.
    uint64_t *p;
    uint8_t byte0;
    switch( ins.opcode  )       // Inspect the opcode
    {
        case 0x00:
            ins.type = HALT ;
            ins.size = 1 ;
	    if (cpu->pc + ins.size >= MEMSIZE)
	    {
		ins.type = INVALID;
		cpu->stat = INS;
	    }
	    ins.dest = cpu->pc + 1;
	    break;

        case 0x10:
            ins.type = NOP ;
	    cpu->stat = INS;
            ins.size = 1 ;
            if (cpu->pc + ins.size >= MEMSIZE)
            {
                ins.type = INVALID;
		cpu->stat = INS;
            }
            break;
        case (0x20): case (0x22): case (0x23): case (0x21): case (0x24): case (0x25): case (0x26):
	    ins.type = CMOV;
	    ins.size = 2;
	    ins.cmov = memory[cpu->pc] & 0x0F;
	    if(cpu->pc + ins.size >= MEMSIZE)
	    {
		ins.type = INVALID;
		cpu->stat = INS;
		break;
	    }

	    ins.ra = ((memory[cpu->pc + 1] & 0xF0) >> 4);
	    ins.rb = (memory[cpu->pc + 1] & 0x0F);

	    if (ins.ra > 0x07 || ins.rb > 0x07 || ins.cmov > 0x06)
	    {
		ins.type = INVALID;
		cpu->stat = INS;
	    }


	    ins.dest = cpu->pc + 2;
	    break;

	case 0x30://IRMOVQ
	    ins.type = IRMOVQ; 
	    ins.size = 10;
	    if(cpu->pc + ins.size >= MEMSIZE)
            {
                ins.type = INVALID;
		cpu->stat = INS;
                break;
            }

	    ins.rb = memory[cpu->pc + 1] & 0x0F;
	
	    p = &memory[cpu->pc + 2];
	    ins.dest = *p;
	    break;

	case 0x40://RMMOVQ
	    ins.type = RMMOVQ;
            ins.size = 10;
            if(cpu->pc + ins.size >= MEMSIZE)
            {
                ins.type = INVALID;
		cpu->stat = INS;
                break;
            }

	    uint8_t byte1 = memory[cpu->pc + 1];
	    ins.ra = ((byte1 & 0xF0) >> 4);
	    ins.rb = (byte1 & 0x0F);


            p = (uint64_t *)&memory[cpu->pc + 2];
            ins.dest = *p;
            break;

	case 0x50://MrMovq
            ins.type = MRMOVQ;
            ins.size = 10;
            if(cpu->pc + ins.size >= MEMSIZE)
            {
                ins.type = INVALID;
		cpu->stat = INS;
                break;
            }

            byte0 = memory[cpu->pc + 1];
            ins.ra = ((byte0 & 0xF0) >> 4);
            ins.rb = (byte0 & 0x0F);
            if (ins.ra > 0x07 || ins.rb > 0x07)
            {
                ins.type = INVALID;
                break;
            }

            p = (uint64_t *)&memory[cpu->pc + 2];
            ins.dest = *p;
            break;
	case 0x60: case 0x61: case 0x62: case 0x63: //Opq code is selected
	    ins.type = OPQ;
            ins.size = 2;
	    ins.op = memory[cpu->pc] & 0x0F;
            if(cpu->pc + ins.size >= MEMSIZE)
            {
                ins.type = INVALID; //Invalid Type
                break;
            }
            ins.ra = ((memory[cpu->pc + 1] & 0xF0) >> 4); //Sets register values
            ins.rb = (memory[cpu->pc + 1] & 0x0F);
            if (ins.op > 0x03 || ins.rb > 0x07 || ins.ra > 0x07)
            {
                ins.type = INVALID;
            }

	    break;
	
	case (0x70): case(0x71): case(0x72): case(0x73): case(0x74): case(0x75): case(0x76): //Jump Switch Statements
	    ins.type = JUMP;
	    ins.size = 9;
	    ins.jump = memory[cpu->pc] & 0x0F;
	    if( ( cpu->pc + ins.size ) >= MEMSIZE )
            {
                ins.type = INVALID;   // an invalid fetch add // INVALID instruction
		break;
            }

	    p = (uint64_t *)&memory[cpu->pc + 1];
	    ins.dest = *p;
	    break;

	case 0x80:
	    ins.type = CALL;
	    ins.size = 9;
	    if( ( cpu->pc + ins.size ) >= MEMSIZE )
            {
                ins.type = INVALID;   // an invalid fetch add // INVALID instruction
		break;
            }

	    p = (uint64_t *)&memory[cpu->pc + 1];
	    ins.dest = *p;
	    break;



        case 0x90:  // ret
	    ins.type = RET;
	    ins.size = 1;
	    if( ( cpu->pc + ins.size ) >= MEMSIZE )
            {
                ins.type = INVALID;   // an invalid fetch add // INVALID instruction
            }


            break;

	case 0xA0:
	    ins.type = PUSHQ;
	    ins.size = 2;
	    if( ( cpu->pc + ins.size ) >= MEMSIZE )
            {
                ins.type = INVALID;   // an invalid fetch add // INVALID instruction
		cpu->stat = INS;
		break;
            }

	    ins.ra = (((memory[cpu->pc + 1] & 0xF0)) >> 4);
	    break;
	case 0xB0:
	    ins.type = POPQ;
	    ins.size = 2;
	    if( ( cpu->pc + ins.size ) >= MEMSIZE )
            {
                ins.type = INVALID;   // an invalid fetch add // INVALID instruction
		cpu->stat = INS;
		break;
            }

	    ins.ra = (((memory[cpu->pc + 1] & 0xF0)) >> 4);
	    break;



        default:
            ins.type  = INVALID ; // INVALID instruction
            cpu->stat = INS ;
            break ;

    }

    return ins;


 
}

//============================================================================
void disassemble (y86_inst_t inst)
{
    switch(inst.type)
    {
	case HALT :
	    printf("halt"); //Halt Instruction
	    break;
	case NOP :
	    printf("nop");
	    break;
	case RET :
	    printf("ret");
	    break;

	case CMOV: //CMOV WITH DIFFERENT METHODS OF CMOV
	    switch(inst.cmov)
	    {
	    case RRMOVQ:
		printf("rrmovq ");
		break;
	    case CMOVLE:
                printf("cmovle ");
                break;
	    case CMOVL:
                printf("cmovl ");
                break;
	    case CMOVE:
                printf("cmove ");
                break;
	    case CMOVNE:
                printf("cmovne ");
                break;
	    case CMOVGE:
                printf("cmovge ");
                break;
	    case CMOVG:
                printf("cmovg ");
                break;
	    case BADCMOV:
                return;
	}

	print_register(inst.ra);
	printf(", ");
	print_register(inst.rb);
	break;

    case IRMOVQ://Mov Code is selected
	printf("irmovq ");
	printf("0x%lx, ", inst.dest);
	print_register(inst.rb);
	break;
    case RMMOVQ:
        printf("rmmovq ");
	print_register(inst.ra);
        printf(", %#lx(", inst.dest);
        print_register(inst.rb);
	printf(")");
        break;
    case MRMOVQ:
        printf("mrmovq ");
	printf("%#lx(", inst.dest);
	print_register(inst.rb);
	printf("), ");
        print_register(inst.ra);
        break;
    case OPQ: //OPQ SWITCH STATEMENT 
	switch(inst.op)
	{
	    case ADD: 
		printf("addq ");
		break;
	    case SUB:
	    	printf("subq ");
		break;
	    case AND:
		printf("andq ");
		break;
	    case XOR:
		printf("xorq ");
		break;
	    case BADOP:
		return;
	}

	print_register(inst.ra);
	printf(", ");
	print_register(inst.rb);
	break;
    case JUMP: //Jump statement is selected and multiple are displayed
	switch(inst.jump)
	{
	    case JMP:
		printf("jmp ");
		break;
	    case JLE:
                printf("jle ");
                break;
	    case JL:
                printf("jl ");
                break;
	    case JE:
                printf("je ");
                break;
	    case JNE:
                printf("jne ");
                break;
	    case JGE:
                printf("jge ");
                break;
	    case JG:
                printf("jg ");
                break;
	    case BADJUMP:
                return;
	}
	printf("%#lx", (uint64_t) inst.dest);
	break;

    case CALL:
	printf("call ");
	printf("%#lx", (uint64_t) inst.dest);
	break;
    case PUSHQ:
	printf("pushq ");
	print_register(inst.ra);
	break;
    case POPQ:
	printf("popq ");
	print_register(inst.ra);
	break;
    case INVALID:
	break;
  }

}

//============================================================================
void disassemble_code (memory_t memory, elf_phdr_t *phdr, elf_hdr_t *hdr)
{
    y86_t cpu; //Creates virtual cpu
    y86_inst_t ins;

    cpu.pc = phdr->p_vaddr; //Sets the CPU.PC value

    printf("  0x%03lx:                      | .pos 0x%03lx code\n", cpu.pc, cpu.pc);

    while(cpu.pc < phdr->p_vaddr + phdr->p_filesz)
    {
	if (cpu.pc == hdr->e_entry)
	{
	    printf("  0x%03lx:                      | _start:\n", (uint64_t) cpu.pc);
	}

	ins = fetch(&cpu, memory); //Fetches the opcode
	if (ins.type == INVALID)
	{
	    printf("Invalid opcode: %#02x\n", ins.opcode); //Invalid OpCode is given
	    cpu.pc += ins.size;
	    return;
	}

	printf("  0x%03lx: ", cpu.pc);
	for (int i = cpu.pc; i < cpu.pc + ins.size; i++)
	{
	    printf("%02x", memory[i]);
	}

	if (ins.type == CMOV || ins.type == OPQ || ins.type == PUSHQ || ins.type == POPQ) 
	{
	    printf("                 |   "); //Formatting according to size of opcodes
	}

	if (ins.type == IRMOVQ || ins.type == RMMOVQ || ins.type == MRMOVQ)
	{
	    printf(" |   ");
	}

	if (ins.type == HALT || ins.type == RET || ins.type == NOP)
	{
	    printf("                   |   ");

	}

	if (ins.type == JUMP || ins.type == CALL)
	{
	    printf("   |   ");
	}
	disassemble(ins);
	cpu.pc += ins.size;
	printf("\n");

	}


}

//============================================================================
void disassemble_data (memory_t memory, elf_phdr_t *phdr)
{
    if (memory == NULL || phdr == NULL)
    {
	return;
    }
    y86_t cpu;
    cpu.pc = phdr ->p_vaddr; //Sets the cpu.pc variablefor future use

    printf("  0x%lx:              | .pos 0x%lx\n", cpu.pc, cpu.pc);
    while (cpu.pc < phdr->p_vaddr + phdr->p_filesz) 
    {
	printf("  0x%lx: ", cpu.pc);
	for (int i = 0; i < 8; i++)
	{
	    printf("%02x", memory[cpu.pc + 1]);
	}
	
	printf("     |   .quad 0x%lx", *(uint64_t*)(&(memory[cpu.pc])));
	printf("\n");
	cpu.pc += 0x8;
    }

}

//============================================================================
void disassemble_rodata (memory_t memory, elf_phdr_t *phdr)
{
    y86_t cpu;
    cpu.pc = phdr->p_vaddr;
    printf("  0x%lx:                      | .pos 0x%lx rodata\n", cpu.pc, cpu.pc);
    char words[MEMSIZE];
    int index;
    int temp_pc;
    while (cpu.pc < phdr->p_vaddr + phdr->p_filesz) 
    {
	for (index = cpu.pc; memory[index] != '\0'; index++)
	{
	    words[index - cpu.pc] = memory[index];
	}

	words[index - cpu.pc] = 0;

	printf("  0x%lx: ", cpu.pc);
	for(temp_pc = cpu.pc; cpu.pc <= index && cpu.pc < temp_pc + 10; cpu.pc++)
	{
	    printf("%02x", memory[cpu.pc]);
	}

	printf(" |   .string \"%s\"\n", words);

	while (cpu.pc <= index) 
	{
	    printf("  0x%lx: ", cpu.pc);
	    int i;
	    for (i = cpu.pc + 0xa; i > cpu.pc && cpu.pc <= index; cpu.pc++)
	    {
		printf("%02x", memory[cpu.pc]);
	    }

	    for (int j = cpu.pc; i > j; j++) 
	    {
		printf("  ");
	    }

	    printf(" | \n");

	}

    }

    printf("\n");
}
//============================================================================

void print_register(y86_regname_t reg)
{
    switch(reg) //Private helper method to print the registers
    {
	case RAX:
	    printf("%%rax");
	    break;
	 case RCX:
            printf("%%rcx");
            break;
	 case RDX:
            printf("%%rdx");
            break;
	 case RBX:
            printf("%%rbx");
            break;
	 case RSP:
            printf("%%rsp");
            break;
	 case RBP:
            printf("%%rbp");
            break;
	 case RSI:
            printf("%%rsi");
            break;
	 case RDI:
            printf("%%rdi");
            break;
	 case R8:
            printf("%%r8");
            break;
	 case R9:
            printf("%%r9");
            break;
	 case R10:
            printf("%%r10");
            break;
	 case R11:
            printf("%%r11");
            break;
	 case R12:
            printf("%%r12");
            break;
	 case R13:
            printf("%%r13");
            break;
	 case R14:
            printf("%%r14");
            break;

	}
}
