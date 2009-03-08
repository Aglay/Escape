/**
 * @version		$Id$
 * @author		Nils Asmussen <nils@script-solution.de>
 * @copyright	2008 Nils Asmussen
 */

#include <common.h>
#include <debug.h>
#include <proc.h>
#include <io.h>
#include <dir.h>
#include <string.h>
#include <service.h>
#include <mem.h>
#include <heap.h>

#include <test.h>
#include <theap.h>
#include <sllist.h>

#define ID_OUT		0
#define ID_IN		1
#define ID_CLEAR	2

static void printProcess(sProc *p) {
	tPid pid = getpid();
	debugf("(%d) Process:\n",pid);
	debugf("(%d) \tpid=%d\n",pid,p->pid);
	debugf("(%d) \tparentPid=%d\n",pid,p->parentPid);
	debugf("(%d) \tstate=%d\n",pid,p->state);
	debugf("(%d) \ttextPages=%d\n",pid,p->textPages);
	debugf("(%d) \tdataPages=%d\n",pid,p->dataPages);
	debugf("(%d) \tstackPages=%d\n",pid,p->stackPages);
}

#include <messages.h>

void logChar(char c) {
	outb(0xe9,c);
	outb(0x3f8,c);
	while((inb(0x3fd) & 0x20) == 0);
}

s32 main(void) {
	s32 fd1,fd;
	do {
		fd1 = open("services:/video",IO_READ | IO_WRITE);
		if(fd1 < 0)
			yield();
	}
	while(fd1 < 0);

	fd = dupFd(fd1);
	debugf("fd=%d, fd1=%d\n",fd,fd1);

#if 0
	if(fork() == 0) {
		s32 id = regService("test");

		static sMsgConRequest header;
		while(1) {
			s32 cfd = waitForClient(id);
			if(cfd < 0)
				printLastError();
			else {
				u32 x = 0;
				s32 c = 0;
				do {
					if((c = read(cfd,&header,sizeof(sMsgConRequest))) < 0)
						printLastError();
					else if(c > 0) {
						sMsgConRequest *msg;
						u8 *data = NULL;
						/* read data */
						if(header.length > 0) {
							data = malloc(header.length * sizeof(u8));
							if(read(cfd,data,header.length) < 0)
								printLastError();
						}

						/* build msg and send */
						msg = createConsoleMsg(header.id,header.length,data);
						if(write(fd,msg,sizeof(sMsgConRequest) + header.length) < 0)
							printLastError();

						if(header.length > 0)
							free(data);
						x++;
					}
				}
				while(c > 0);
				close(cfd);
			}
		}
		unregService(id);
	}

	s32 fd2;
	do {
		fd2 = open("services:/test",IO_READ | IO_WRITE);
		if(fd2 < 0)
			yield();
	}
	while(fd2 < 0);
	debugf("fd2=%d\n",fd2);

	if(redirFd(fd,fd2) < 0)
		printLastError();
	else
		debugf("Redirected %d to %d\n",fd,fd2);
#endif

	s8 str[] = "Test: \e[30mblack\e[0m, \e[31mred\e[0m, \e[32mgreen\e[0m"
			", \e[33morange\e[0m, \e[34mblue\e[0m, \e[35mmargenta\e[0m, \e[36mcyan\e[0m"
			", \e[37mgray\e[0m\n"
			"\e[30;44mwithbg\e[0m, \e[34;43mwithbg\e[0m\n";
	sMsgConRequest *msg = createConsoleMsg(MSG_VIDEO_PUTS,strlen(str) + 1,str);
	u32 i;
	s32 err;
	for(i = 0; i < 1 ;i++) {
		if((err = write(fd,msg,sizeof(sMsgConRequest) + msg->length)) < 0) {
			if(err == ERR_NOT_ENOUGH_MEM)
				yield();
			else
				printLastError();
		}
		else {
			yield();
		}
	}

	sMsgDataVidGoto vidgoto;
	vidgoto.col = 0;
	vidgoto.row = 10;
	msg = createConsoleMsg(MSG_VIDEO_GOTO,sizeof(sMsgDataVidGoto),&vidgoto);
	write(fd,msg,sizeof(sMsgConRequest) + msg->length);

	msg = createConsoleMsg(MSG_VIDEO_PUTS,5,"Test");
	write(fd,msg,sizeof(sMsgConRequest) + msg->length);

	vidgoto.col = 0;
	vidgoto.row = 13;
	msg = createConsoleMsg(MSG_VIDEO_GOTO,sizeof(sMsgDataVidGoto),&vidgoto);
	write(fd,msg,sizeof(sMsgConRequest) + msg->length);

	msg = createConsoleMsg(MSG_VIDEO_PUTS,5,"Test");
	write(fd,msg,sizeof(sMsgConRequest) + msg->length);

#if 0
	/* read from keyboard */
	s32 fd3;
	do {
		fd3 = open("services:/keyboard",IO_READ | IO_WRITE);
		if(fd3 < 0)
			yield();
	}
	while(fd3 < 0);

	static sMsgKbRequest kbReq;
	static sMsgKbResponse kbRes;

	kbReq.id = MSG_KEYBOARD_READ;
	while(true) {
		if(write(fd3,&kbReq,sizeof(sMsgKbRequest)) > 0) {
			if(read(fd3,&kbRes,sizeof(kbRes)) <= 0)
				yield();
			else {
				if(kbRes.isBreak)
					debugf("Key %d released\n",kbRes.keycode);
				else
					debugf("Key %d pressed\n",kbRes.keycode);
			}
		}
	}
	close(fd3);
#endif

	freeConsoleMsg(msg);
	close(fd);


#if 0
	s32 id = regService("console");
	if(id < 0)
		printLastError();
	else {
		if(fork() == 0) {
			s32 sfd = open("services:/console",IO_READ | IO_WRITE);
			if(sfd < 0)
				printLastError();
			else {
				s8 str[] = "Test";
				sMsgConRequest *msg = createMsg(ID_OUT,5,str);
				do {
					if(write(sfd,msg,sizeof(sMsgConRequest) + msg->length) < 0)
						printLastError();
					yield();
				}
				while(1);
				freeMsg(msg);
			}
			close(sfd);
			while(1);
		}

		if(fork() == 0) {
			s32 sfd = open("services:/console",IO_READ | IO_WRITE);
			if(sfd < 0)
				printLastError();
			else {
				sMsgConRequest *msg = createMsg(ID_CLEAR,0,NULL);
				do {
					if(write(sfd,msg,sizeof(sMsgConRequest)) < 0)
						printLastError();
					yield();
				}
				while(1);
				freeMsg(msg);
			}
			close(sfd);
			while(1);
		}

		videoBase = mapPhysical(0xB8000,COLS * ROWS * 2);
		video = videoBase;
		vid_clearScreen();
		vid_setFGColor(WHITE);
		vid_setBGColor(BLACK);

		static sMsgConRequest msg;
		do {
			s32 fd = waitForClient(id);
			if(fd < 0)
				printLastError();
			else {
				read(fd,&msg,sizeof(sMsgConRequest));
				if(msg.id == ID_OUT) {
					s8 *readBuf = malloc(msg.length * sizeof(s8));
					read(fd,readBuf,msg.length);
					debugf("Read: __%s__\n",readBuf);
					free(readBuf);
				}
				else if(msg.id == ID_IN) {

				}
				else if(msg.id == ID_CLEAR) {
					/*vid_clearScreen();*/
					debugf("CLEAR\n");
				}
				close(fd);
			}
		}
		while(1);

		unregService();
	}

#else

#if 0
	s32 id = regService("console");
	if(id < 0)
		printLastError();
	else {
		s8 buf[] = "Das ist ein Teststring\n";
		s32 fd = open("services:/console",IO_READ | IO_WRITE);
		if(fd < 0)
			printLastError();
		else {
			if(fork() == 0) {
				s8 *readBuf = malloc(strlen(buf) * sizeof(s8));
				while(1) {
					if(readBuf == NULL)
						printLastError();
					else {
						if(read(fd,readBuf,strlen(buf)) < 0)
							printLastError();
						else {
							debugf("Read: %s\n",readBuf);
							/* reset buffer */
							*readBuf = '\0';
						}
					}
					yield();
				}
				close(fd);
			}
			else {
				while(1) {
					if(write(fd,buf,strlen(buf)) < 0)
						printLastError();
					else {
						debugf("Wrote: %s\n",buf);
					}
					yield();
				}
			}
		}

		unregService();
	}


	s32 id = regService("console");
	if(id < 0)
		printLastError();
	else {
		debugf("Registered service\n");

		if(fork() == 0) {
			debugf("mypid=%d, ppid=%d\n",getpid(),getppid());
			s8 buf[100];
			s32 sfd = open("services:/console",IO_READ | IO_WRITE);
			debugf("Opened service with fd %d\n",sfd);
			yield();
			if(read(sfd,buf,100) < 0)
				printLastError();
			else
				debugf("Read: %s\n",buf);
			close(sfd);
			while(1);
		}

		yield();
		if(unregService(id) < 0)
			printLastError();
		else
			debugf("Unregistered service\n");
	}
#endif
#endif

	/*videoBase = mapPhysical(0xB8000,COLS * ROWS * 2);
	debugf("videoBase=0x%x\n",videoBase);
	if(videoBase == NULL) {
		printLastError();
	}
	else {
		video = videoBase;
		vid_clearScreen();
		vid_setFGColor(WHITE);
		vid_setBGColor(BLACK);

		vid_printf("Hallo, ich bins :P\n");
	}

	sSLList *l = sll_create();
	sll_append(l,1);
	sll_append(l,2);
	sll_append(l,3);

	sll_dbg_print(l);

	sll_destroy(l,false);*/

	/*test_register(&tModHeap);
	test_start();*/

	/*s32 count;

	count = changeSize(3);
	if(count < 0)
		printLastError();
	else
		debugf("dataPages = %d\n",count);

	count = changeSize(-3);
	if(count < 0)
		printLastError();
	else
		debugf("dataPages = %d\n",count);

	u32 *ptr[10],*p,*first;
	u32 c,i,x = 0;

	c = 10;
	ptr[x] = (u32*)malloc(sizeof(u32) * c);
	first = ptr[x];
	debugf("ptr=0x%x\n",ptr[x]);
	p = ptr[x];
	for(i = 0;i < c;i++)
		*p++ = x + 1;

	x++;
	c = 3;
	ptr[x] = (u32*)malloc(sizeof(u32) * c);
	debugf("ptr=0x%x\n",ptr[x]);
	p = ptr[x];
	for(i = 0;i < c;i++)
		*p++ = x + 1;

	x++;
	c = 2;
	ptr[x] = (u32*)malloc(sizeof(u32) * c);
	debugf("ptr=0x%x\n",ptr[x]);
	p = ptr[x];
	for(i = 0;i < c;i++)
		*p++ = x + 1;

	x++;
	c = 10;
	ptr[x] = (u32*)malloc(sizeof(u32) * c);
	debugf("ptr=0x%x\n",ptr[x]);
	p = ptr[x];
	for(i = 0;i < c;i++)
		*p++ = x + 1;

	free(ptr[2]);
	free(ptr[0]);
	free(ptr[1]);

	/*for(i = 0;i < 25;i++) {
		debugf("0x%x = 0x%x\n",first,*first);
		first++;
	}

	ptr = (u32*)malloc(sizeof(u32) * 1025);
	debugf("ptr=0x%x\n",ptr);

	debugf("ptr=0x%x\n",malloc(4092));
	debugf("ptr=0x%x\n",malloc(3996));
	debugf("ptr=0x%x\n",malloc(10));

	printHeap();*/

	/*u16 pid = fork();
	if(pid == 0) {
		debugf("I am the child with pid %d\n",getpid());
	}
	else if(pid == -1) {
		debugf("Fork failed\n");
	}
	else {
		debugf("I am the parent, created child %d\n",pid);
	}

	while(true) {
		debugf("Hi, my pid is %d, parent is %d\n",getpid(),getppid());
	}*/

	/*if(regService("test") < 0)
		printLastError();
	else
		unregService();*/

	/*s32 dd;
	sDir *entry;
	debugf("Listing directory '/':\n");
	if((dd = opendir("/")) >= 0) {
		while((entry = readdir(dd)) != NULL) {
			debugf("- 0x%x : %s\n",entry->nodeNo,entry->name);
		}
		closedir(dd);
	}

	if(fork() == 0) {
		tFD fd;
		sProc proc;
		s8 path[] = "system:/processes/";
		s8 ppath[255];
		debugf("(%d) Listing all processes:\n",getpid());
		if((dd = opendir(path)) >= 0) {
			while((entry = readdir(dd)) != NULL) {
				strncpy(ppath,path,strlen(path));
				strncat(ppath,entry->name,strlen(entry->name));
				if((fd = open(ppath,IO_READ)) >= 0) {
					read(fd,&proc,sizeof(sProc));
					printProcess(&proc);
					close(fd);
				}
			}
			closedir(dd);
		}
		return 0;
	}*/

	while(1);
	return 0;
}
