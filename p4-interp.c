/*
 * CS 261 PA4: Interpreter Code
 *
 * Assignment : Project4
 * Date : 5/4
 * Author : Adam Lillard lillarar@jmu.edu
 * File name : p4-interp.c
 */
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "p4-interp.h"

//=======================================================================

void usage_p4 ()
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
	printf("  -e      Execute program\n");
	printf("  -E      Execute program (debug trace mode)\n");
	printf("Options must not be repeated neither explicitly nor implicitly.");
	
}

bool parse_command_line_p4 
    (   int argc         , char **argv      ,
        bool *header     , bool *segments   , bool *membrief , 
        bool *memfull    , bool *disas_code , bool *disas_data ,
        bool *exec_normal, bool *exec_debug , char **file 
    )
{
	
	if (argv == NULL || header == NULL || segments == NULL
            || membrief == NULL || memfull == NULL
            || disas_code == NULL || disas_data == NULL || file == NULL
			|| exec_normal == NULL || exec_debug == NULL) {
        return false;
    }
	
	int opt;
	char *optstr = "+hHafsmMdDeE";
	
	while ((opt = getopt(argc, argv, optstr)) != -1) {
		
		
		switch(opt) {
			case 'h':
				usage_p4(argc);
				return true;
			case 'H':
			
				if (*header == true) {
					usage_p4(argv);
					return false;
				}
				
				*header = true;
				break;
			case 'a':
			
				if (*header == true || *segments == true || *membrief == true) {
					usage_p4(argv);
					return false;
				}
				
				*header = true;
				*segments = true;
				*membrief = true;
				break;
			case 'f':
			
				if (*header == true || *segments == true || *memfull == true) {
					usage_p4(argv);
					return false;
				}
				
				*header = true;
				*segments = true;
				*memfull = true;
				break;
			case 's':
			
				if (*segments == true) {
					usage_p4(argv);
					return false;
				}
				
				*segments = true;
				break;
			case 'm':
			
				if (*membrief == true) {
					usage_p4(argv);
					return false;
				}
				
				*membrief = true;
				break;
			case 'M':
			
				if (*memfull == true) {
					usage_p4(argv);
					return false;
				}
				
				*memfull = true;
				break;
				
			case 'd':
				if (*disas_code == true) {
					usage_p4(argv);
					return false;
				}
				
				*disas_code = true;
				break;
				
			case 'D':
				if (*disas_data == true) {
					usage_p4(argv);
					return false;
				}
				
				*disas_data = true;
				break;
			
			case 'e':
				
				if (*exec_normal == true) {
					usage_p4(argv);
					return false;
				}
				
				*exec_normal = true;
				break;
				
			case 'E':
				
				if (*exec_debug == true) {
					usage_p4(argv);
					return false;
				}
				
				*exec_debug = true;
				break;
				
			default:
				usage_p4(argv);
				return false;
		}
	}
		
	if (optind != argc-1) {
		usage_p4(argv);
		return false;
	}
	
	*file = argv[optind];
    return true;
}

//=======================================================================

void dump_cpu( const y86_t *cpu ) 
{
    
	printf("dump of Y86 CPU:\n");
	printf("  %%rip: %016lx   flags: SF%d ZF%d OF%d  ", cpu->pc, cpu->sf, cpu->zf, cpu->of);
	
	//switch for cpu status
    switch(cpu->stat)
    {
        case(AOK): 
		
			printf("AOK\n");
			break;
			
        case(HLT):

			printf("HLT\n");
			break;
			
        case(ADR): 
		
			printf("ADR\n");
			break;
			
        case(INS):
		
			printf("INS\n");
			break;
			
			
    }
	
	
	printf("  %%rax: %016lx    %%rcx: %016lx \n",  cpu->rax,  cpu->rcx);
    printf("  %%rdx: %016lx    %%rbx: %016lx \n", cpu->rdx, cpu->rbx);
    printf("  %%rsp: %016lx    %%rbp: %016lx \n",   cpu->rsp,  cpu->rbp);
    printf("  %%rsi: %016lx    %%rdi: %016lx \n",  cpu->rsi, cpu->rdi);
	printf("   %%r8: %016lx     %%r9: %016lx \n",  cpu->r8,  cpu->r9);
    printf("  %%r10: %016lx    %%r11: %016lx \n", cpu->r10, cpu->r11);
    printf("  %%r12: %016lx    %%r13: %016lx \n",   cpu->r12,  cpu->r13);
    printf("  %%r14: %016lx\n\n",  cpu->r14);
	
	
}

//=======================================================================
y86_register_t op(y86_register_t *valA, y86_register_t valB, const y86_inst_t *inst, y86_t *cpu) {
	
	y86_register_t valE;
	
	switch(inst->op) {
	
	
		case(ADD):
			
			cpu->of = (valB < 0 && valA < 0 && valE > 0) || (valB > 0 && valA > 0 && valE < 0);
			valE = (int64_t) valA + valB;
			break;
			
		case(SUB):
			
			cpu->of = (valB < 0 && valA > 0 && valE > 0) || (valB > 0 && valA < 0 && valE < 0);
			valE = (int64_t) valB - (int64_t) valA;
			break;
			
		case(AND):
		
			valE = (int64_t) valA & valB;
			break;
			
		case(XOR): 
		
			valE = (int64_t) valA ^ valB;
			break;
		
		case(BADOP):
			cpu->stat = INS;
		default:
		
			cpu->stat = INS;
			
	}
	
	return valE;

}

bool getCmov(y86_cmov_t cmov, y86_t *cpu)
{
    bool cond = false;
    switch(cmov)
    {
        case(RRMOVQ):
            cond = true;
            break;
        case(CMOVLE):
            if(cpu->zf || ((cpu->sf && !cpu->of) || (!cpu->sf && cpu->of)))
            {
                cond = true;
            }
            break;
        case(CMOVL):
            if((cpu->sf && !cpu->of) || (!cpu->sf && cpu->of))
            {
                cond =  true;
            }
            break;
        case(CMOVE):
            if(cpu->zf)
            {
                cond =  true;
            }
            break;
        case(CMOVNE):
            if(!cpu->zf)
            {
                cond =  true;
            }
            break;
        case(CMOVGE):
            if(cpu->sf == cpu->of)
            {
                cond =  true;
            }
            break;
        case(CMOVG):
            if(!cpu->zf && cpu->sf == cpu->of)
            {
                cond =  true;
            }
            break;
        case(BADCMOV): cpu->stat = INS;
    }

    return cond;
}

bool getJump(y86_jump_t jmp, y86_t *cpu)
{
	bool cond = false;
	switch(jmp) 
	{
		case(JMP): 
            cond = true;
            break;
        case(JLE):
            if(cpu->zf || ((cpu->sf && !cpu->of) || (!cpu->sf && cpu->of)))
            {
                cond = true;
            }
           break;
        case(JL):
            if((cpu->sf && !cpu->of) || (!cpu->sf && cpu->of))
            {
                cond =  true;
            }
            break;
        case(JE):
            if(cpu->zf)
            {
                cond =  true;
            }
            break;
        case(JNE):
            if(!cpu->zf)
            {
                cond =  true;
            }
            break;
        case(JGE):
            if(cpu->sf == cpu->of)
            {
                cond =  true;
            }
            break;
        case(JG):
            if(!cpu->zf && cpu->sf == cpu->of) 
            {
                cond =  true;
            }
            break;
		case(BADJUMP) :
		cpu->stat = INS;
	}

	return cond;
}

void update_register(y86_rnum_t reg, y86_register_t valM, y86_t *cpu) {
	switch(reg) {
		case(0) :
		cpu->rax = valM;
		break;
		case(1) :
		cpu->rcx = valM;
		break;
		case(2) :
		cpu->rdx = valM;
		break;
		case(3) :
		cpu->rbx = valM;
		break;
		case(4) :
		cpu->rsp = valM;
		break;
		case(5) :
		cpu->rbp = valM;
		break;
		case(6) :
		cpu->rsi = valM;
		break;
		case(7) :
		cpu->rdi = valM;
		break;
		case(8) :
		cpu->r8 = valM;
		break;
		case(9) :
		cpu->r9 = valM;
		break;
		case(10) :
		cpu->r10 = valM;
		break;
		case(11) :
		cpu->r11 = valM;
		break;
		case(12) :
		cpu->r12 = valM;
		break;
		case(13) :
		cpu->r13 = valM;
		break;
		case(14) :
		cpu->r14 = valM;
		break;
		default:
		break;

	}
}

y86_register_t getRegister(y86_t *cpu, y86_rnum_t reg) {
	
	switch(reg)
    {
        case(0): return cpu->rax;
        case(1): return cpu->rcx;
        case(2): return cpu->rdx;
        case(3): return cpu->rbx;
        case(4): return cpu->rsp;
        case(5): return cpu->rbp;
        case(6): return cpu->rsi;
        case(7): return cpu->rdi;
		case(8): return cpu->r8;
		case(9): return cpu->r9;
		case(10): return cpu->r10;
		case(11): return cpu->r11;
		case(12): return cpu->r12;
		case(13): return cpu->r13;
		case(14): return cpu->r14;
        case(BADREG): cpu->stat = INS; return 0;
        default: return 0;
    }
}

y86_register_t decode_execute(  y86_t *cpu , bool *cond , const y86_inst_t *inst ,
                                y86_register_t *valA 
                             )
{
	y86_register_t valE = 0;
	y86_register_t valB = 0;
	
	if(valA == NULL) {
		cpu->stat = INS;
	}
	
	switch(inst->type) {
		
		case(HALT):
		
			cpu->stat = HLT;
			
			cpu->zf = false;
			cpu->sf = false;
			cpu->of = false;
			break;
			
		case(IRMOVQ):
		
			valE = inst->value;
			break;
			
		case(JUMP) :
			*cond = getJump(inst->jump, cpu);
			break;
		case(OPQ):
		
			valA = getRegister(cpu, inst->ra);
			valB = getRegister(cpu, inst->rb);
			valE = op(valA, valB, inst, cpu);
			break;
		
		case(POPQ):
			
			*valA = cpu->rsp;
			valB = cpu->rsp;
			valE = (int64_t) valB + 8;
			break;
			
		case(PUSHQ):
		
			*valA = getRegister(cpu, inst->ra);
			valB = cpu->rsp;
			valE = (int64_t) valB - 8;
			break;
			
		case(RMMOVQ):
		
			*valA = getRegister(cpu, inst->ra);
			valB = getRegister(cpu, inst->rb);
			valE = (uint64_t) inst->d + (uint64_t) valB;
			break;
		
		case(MRMOVQ):	
		
			valB = getRegister(cpu, inst->rb);
			valE = (uint64_t) inst->d + (uint64_t) valB;
			break;
		
		case(CALL):
			
			valB = cpu->rsp;
			valE = (uint64_t) valB - 8;
			break;
		
		case(CMOV) :
			*valA = getRegister(cpu, inst->ra);
			valE = *valA;
			*cond = getCmov(inst->cmov, cpu);
            break;

		case(RET):
			
			*valA = cpu->rsp;
			valB = cpu->rsp;
			valE = (int64_t) valB + 8;
			break;

		case(INVALID) :
			cpu->stat = INS;
			break;
	}
	
	
    return valE;
}

//=======================================================================

void memory_wb_pc(  y86_t *cpu , memory_t memory , bool cond , 
                    const y86_inst_t *inst , y86_register_t  valE , 
                    y86_register_t  valA 
                 )
{
    
	y86_register_t valM;
	uint64_t *point;
	
	switch(inst->type) {
			
		case (HALT):
			cpu->pc += inst->size; //Increments the size
			break;
		case(NOP):
		
			cpu->pc += inst->size;
			break;
			
		case(MRMOVQ):
			if(valE >= MEMSIZE) {
				cpu->stat = ADR;
				break;
			}
		
		case(CMOV) :
            cpu->pc += inst->size;
            break;
		case(IRMOVQ):
		
			update_register(inst->rb, valE, cpu); //Updates the register 
			cpu->pc += inst->size; //Increments the size again
			break;
			
		case(OPQ):
		
			update_register(inst->rb, valE, cpu);
			cpu->pc += inst->size;
			break;
			
		case(POPQ):
		
			point = (uint64_t *) &memory[valA];
			valM = *point;
			cpu->rsp = valE;
			update_register(inst->ra, valM, cpu);
			cpu->pc += inst->size;
			
			break;

		case (PUSHQ):
		
			point = (uint64_t *) &memory[valE];
			*point = valA;
			cpu->rsp = valE;
			cpu->pc += inst->size;
			printf("Memory write to 0x%04lx: 0x%lx \n",  valE, valA);
			break;
			
		case(CALL):
			
			point = (uint64_t *) &memory[valE];
			point = cpu->pc + inst->size;
			cpu->rsp = valE;
			cpu->pc = inst->dest;
			printf("Memory write to 0x%04lx: 0x%lx \n",  valE, point);
			
			break;
			
		case(JUMP) :
			if(cond) {
				cpu->pc = inst->dest;
				break;
			}

			cpu->pc += inst->size;
			break;
		case(RMMOVQ):
		
			if(valE >= MEMSIZE) {
				cpu->stat = ADR;
				break;
			}
			point = (uint64_t *) &memory[valE];
			*point = valA;
			cpu->pc += inst->size;
			printf("Memory write to 0x%04lx: 0x%lx \n",  valE, valA);
			break;
			
		
			
		case(RET):
			point = (uint64_t *) &memory[valA];
			valM = *point;
			cpu->rsp = valE;
			cpu->pc += valM;
			
			break;

		default :
			cpu->stat = INS;
			break;
			
	}
	
	
	
}


