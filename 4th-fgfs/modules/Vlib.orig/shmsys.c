#include "/usr/include/sys/types.h"
#include "/usr/include/sys/ipc.h"
#include "/usr/include/sys/shm.h"
#include "/usr/include/sys.s"

#define	SHMSYS	SYS_shmsys

#define	SHMAT	0
#define	SHMCTL	1
#define	SHMDT	2
#define	SHMGET	3

char *
shmat(shmid, shmaddr, shmflg)
int shmid;
char *shmaddr;
int shmflg;
{
	return (char *) syscall(SHMSYS, SHMAT, shmid, shmaddr, shmflg);
}

int
shmctl(shmid, cmd, buf)
int shmid, cmd;
struct shmid_ds *buf;
{
	return syscall(SHMSYS, SHMCTL, shmid, cmd, buf);
}

int
shmdt(shmaddr)
char *shmaddr;
{
	return syscall(SHMSYS, SHMDT, shmaddr);
}

int
shmget(key, size, shmflg)
key_t key;
int size, shmflg;
{
	return syscall(SHMSYS, SHMGET, key, size, shmflg);
}
