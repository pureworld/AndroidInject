#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>

#define LOGD(...) printf(__VA_ARGS__)

void showregs(pt_regs *regs) {
	for (int i = 0; i < 17; i++)
	{
		if (i == 13)
			LOGD("SP:%p\n", (void*)regs->uregs[i]);
		else if (i == 14)
			LOGD("LR:%p\n", (void*)regs->uregs[i]);
		else if (i == 15)
			LOGD("PC:%p\n", (void*)regs->uregs[i]);
		else if (i == 16)
			LOGD("CPSR:%p\n", (void*)regs->uregs[i]);
		else
			LOGD("R%d:%p\n", i, (void*)regs->uregs[i]);
	}
}

long myattach(pid_t pid) {
	return ptrace(PTRACE_ATTACH, pid, NULL, NULL);
}

long mydettach(pid_t pid) {
	return ptrace(PTRACE_DETACH, pid, NULL, NULL);
}

long mycont(pid_t pid) {
	return ptrace(PTRACE_CONT, pid, NULL, NULL);
}

long getregs(pid_t pid, pt_regs* pregs) {
	return ptrace(PTRACE_GETREGS, pid, NULL, pregs);
}

long setregs(pid_t pid, pt_regs* pregs) {
	return ptrace(PTRACE_SETREGS, pid, NULL, pregs);
}

//long mypokedata(pid_t pid, long addr, int length, void* data) {
//	return ptrace(PTRACE_POKEDATA, pid, (void*)addr, data);
//}

long mypokedata(pid_t pid, long addr, int length, void* data) {
    int res = -1;
    if (length % 4 != 0) {
        return res;
    }
    else {
        int cnt = length / 4;
        for (int i = 0; i < cnt; i++) {
            res = ptrace(PTRACE_POKEDATA, pid, (void*)(addr + i*4), (void*)(((int*)data)[i]));
            if (res < 0)
                return res;
        }
    }
    return res;
}

long get_module_base(pid_t pid, const char* libname);
long xfun(pid_t pid, const char* libname, long funaddr, int varcnt, int* pvars);

