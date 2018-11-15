#include "myinject.h"

#define T_MASK (1 << 5)

#define MYCHECK(x) {if (res < 0) { \
		perror(x); \
		return 0; \
	} else { \
		printf("%s ok\n", x); \
	}}

int main(int argc, char* argv[]) {
	pid_t pid = 76;

	long res = myattach(pid);
	MYCHECK("myattach")

	waitpid(pid, NULL, WUNTRACED);

	// 调用mmap申请内存
	int vars_mmap[6];
	vars_mmap[0] = 0;	//addr
	vars_mmap[1] = 0x1000;	//len
	vars_mmap[2] = PROT_READ | PROT_WRITE;	//prot
	vars_mmap[3] = MAP_PRIVATE | MAP_ANONYMOUS;	//flags
	vars_mmap[4] = 0;	//fildes
	vars_mmap[5] = 0;	//off
	long addr = xfun(pid, "libc.so", (long)mmap, 6, vars_mmap);
	//printf("%x", addr);

	// 写入路径
	char path[260] = {"/data/local/tmp/libhello.so"};
	mypokedata(pid, addr, 260, path);

	// 调用dlopen
	int vars_dlopen[2];
	vars_dlopen[0] = addr;	//addr
	vars_dlopen[1] = RTLD_NOW;	//len
	long handle = xfun(pid, "linker", (long)dlopen, 2, vars_dlopen);
	//printf("%x", handle);

	return 0;
}

long get_module_base(pid_t pid, const char* libname) {
	char buffer[300];
	if (pid == 0)
		sprintf(buffer, "/proc/self/maps");
	else
		sprintf(buffer, "/proc/%d/maps", pid);

	FILE *fp = fopen(buffer, "r");
	if (fp != NULL)
	{
		while(!feof(fp))
		{
			fgets(buffer, sizeof(buffer), fp);
			if (strstr(buffer, libname) != NULL)
			{
				int addr;
				sscanf(buffer, "%x", &addr);
				//printf("addr = %x\n", addr);
				return addr;
				break;
			}
		}
		fclose(fp);
	}
	return 0;
}

long xfun(pid_t pid, const char* libname, long funaddr, int varcnt, int* pvars) {
	pt_regs old_regs;
	long res = getregs(pid, &old_regs);
	MYCHECK("getregs")

	long funaddr_remote = funaddr +
			get_module_base(pid, libname) - get_module_base(0, libname);

//	printf("funaddr = %p remote_base = %p self_base = %p remote_funaddr = %p\n", funaddr,
//			get_module_base(pid, libname),
//			get_module_base(0, libname),
//			funaddr_remote);

	pt_regs new_regs = old_regs;

	//showregs(&new_regs);

	switch(varcnt) {
	default:
		new_regs.ARM_sp -= (varcnt - 4) * 4;
//		for (int i = 0; i < (varcnt - 4); i++) {
//			res = mypokedata(pid, new_regs.ARM_sp + i*4, (void*)pvars[i+4]);
//			MYCHECK("mypokedata")
//		}
		res = mypokedata(pid, new_regs.ARM_sp, 8, pvars + 4);
		MYCHECK("mypokedata");
		/* no break */
	case 4:
		new_regs.ARM_r3 = pvars[3];
		/* no break */
	case 3:
		new_regs.ARM_r2 = pvars[2];
		/* no break */
	case 2:
		new_regs.ARM_r1 = pvars[1];
		/* no break */
	case 1:
		new_regs.ARM_r0 = pvars[0];
		/* no break */
	case 0:
		break;
	}

	new_regs.ARM_lr = 0;	// 制造函数返回时异常
	new_regs.ARM_pc = funaddr_remote & ~1;
	//设置T位
	if (funaddr_remote & 1)
	{
		//thumb
		new_regs.ARM_cpsr |= T_MASK;
	}
	else
	{
		//arm
		new_regs.ARM_cpsr &= ~T_MASK;
	}

	//showregs(&new_regs);
	res = setregs(pid, &new_regs);
	MYCHECK("setregs")

	res = mycont(pid);
	MYCHECK("mycont")

	waitpid(pid, NULL, WUNTRACED);

	// 获取结果
	res = getregs(pid, &new_regs);
	MYCHECK("getregs")
	//showregs(&new_regs);
	// 恢复环境
	res = setregs(pid, &old_regs);
	MYCHECK("setregs")

	return new_regs.ARM_r0;
}
