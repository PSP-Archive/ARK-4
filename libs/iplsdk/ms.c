#include <pspkernel.h>

#define IO_MEM_STICK_CMD *((volatile int*)(0xBD200030))
#define IO_MEM_STICK_DATA *((volatile int*)(0xBD200034))
#define IO_MEM_STICK_STATUS *((volatile int*)(0xBD200038))
#define IO_MEM_STICK_SYS *((volatile int*)(0xBD20003C))

#define MSRST 0x8000

#define MS_FIFO_RW   0x4000
#define MS_RDY       0x1000
#define MS_TIME_OUT  0x0100
#define MS_CRC_ERROR 0x0200

#define READ_PAGE_DATA  0x2000
#define READ_REG        0x4000
#define GET_INT         0x7000
#define SET_RW_REG_ADRS 0x8000
#define EX_SET_CMD      0x9000

#define INT_REG_CED   0x80
#define INT_REG_ERR   0x40
#define INT_REG_BREQ  0x20
#define INT_REG_CMDNK 0x01
/*
;NORM_COMP       = (ced && !err)
;CMD_ERR_TER     = (ced && err)
;NORM_DATA_TRANS = (!err && breq)
;DATA_REQ_ERR    = (err && breq)
;CMD_EXE         = (!ced && !breq)
;CMD_NOT_EXE     = cmdnk
*/

static u8 sts_buf[8];

void ms_wait_unk1(void);
int _ms_init(void);
int send_data_and_sync(int arg1, int arg2);
int read_data(int addr, int count);
int ms_wait_ready(void);
int ms_check_unk2(void);
int ms_get_reg(int buffer, int reg);
int ms_get_reg_int(void);

void pspMsInit(void){
    _ms_init();
}


void ms_wait_unk1(void){
    while(!(IO_MEM_STICK_STATUS & 0x2000)){};
}

int parse_init_registers()
{
    *((volatile int*)(0xBC100054)) |= 0x00000100;
    *((volatile int*)(0xBC100050)) |= 0x00000400;
    *((volatile int*)(0xBC100078)) |= 0x00000010;
    *((volatile int*)(0xBC10004C)) &= ~0x100;

  return 0;
}

int _ms_init(void)
{
    //initialize the hardware
    parse_init_registers();

    //reset the controller
    IO_MEM_STICK_SYS = MSRST;
    while(IO_MEM_STICK_SYS & MSRST){}

    ms_check_unk2();

    ms_wait_ready();

    int ret;
    do{
        ret = ms_get_reg_int();
    }while((ret < 0) || ( (ret & INT_REG_CED) == 0));

    return 0;
}

int send_data_and_sync(int arg1, int arg2){
  int ret;
  IO_MEM_STICK_DATA = arg1;
  IO_MEM_STICK_DATA = arg2;
  ret = ms_wait_ready();
  return ret;
}

int read_data(int addr, int count){
  int i;
  int status;
  for(i = 0; i<count; i+= 4){
    do{
      status = IO_MEM_STICK_STATUS;
      if (status & MS_TIME_OUT) return -1;
    }while(!(status & MS_FIFO_RW));
    *((volatile int*)(addr + i)) = IO_MEM_STICK_DATA;

//Kprintf("%08X ",*((volatile int*)(addr + i)));
if( (i%0x20) ==0x1c) ; //Kprintf("\n");

  }
  return 0;
}

int ms_wait_ready(void){
  int status;
  do{
    status = IO_MEM_STICK_STATUS;
  }while(!(status & MS_RDY));

  if (status & (MS_CRC_ERROR|MS_TIME_OUT))
  {
//Kprintf("ms_wait_ready:err %08X\n",status);
    return -1;
  }
  return 0;
}

/*
    read status register
*/
int ms_check_unk2(void)
{
    int ret, val_a;
    //set rw reg addrs of type reg (?)
    IO_MEM_STICK_CMD = SET_RW_REG_ADRS | 0x4;
    IO_MEM_STICK_DATA = 0x06100800;
    IO_MEM_STICK_DATA = 0x00000000;


    ret = ms_wait_ready();
    if (ret != 0) return -1;

    //  ms_get_reg(0x80010A1C, 0x8);
    ms_get_reg((int)sts_buf, 8);
    if(sts_buf[4] != 0x01)
    {
        return -1;
    }

    val_a = *((volatile int*)(&sts_buf[0]));

    if (((val_a >> 16) & 0x15) != 0)
        return -1;

    return 0;
}

int ms_get_reg(int buffer, int reg){
  int ret;

//Kprintf("READ_REG\n");
  IO_MEM_STICK_CMD = READ_REG | reg;
  ret = read_data(buffer, reg);

  return ret;
}

int ms_get_reg_int(void)
{
  int ret, status;

  IO_MEM_STICK_CMD = GET_INT | 0x1;

  do{
    status = IO_MEM_STICK_STATUS;
    if(status & MS_TIME_OUT)
    {
//Kprintf("get_reg_int timeout\n");
     return -1;
    }
  }while(!(status & MS_FIFO_RW));

  ret = IO_MEM_STICK_DATA;
  (void) IO_MEM_STICK_DATA;

  do{
    status = IO_MEM_STICK_STATUS;
    if(status & MS_TIME_OUT)
    {
//Kprintf("get_reg_int timeout\n");
     return -1;
    }
  }while(!(status & MS_RDY));

  return ret & 0xff;
}
