#include <pspkernel.h>
#include <pspsdk.h>
#include <pspwlan.h>
#include <psputility.h>
#include <psputility_netmodules.h>
#include <pspwlan.h>
#include <pspnet.h>
#include <pspnet_inet.h>
#include <pspnet_apctl.h>		//<-- STAS: included in net.h
#include <pspnet_resolver.h>
#include <psphttp.h>
#include <pspssl.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

// PUFF
#include <pspctrl.h>

#include "ftp.h" // Prototipos para el ftp struct, etc
//#include "ftpd.h" // Prototipo de Llamadas a las funciones ftpd
#include "sutils.h" // Prototipo a utilidades string!

// Variables de hilo y estado del server ftp
int ftpdServerState = 0; // 0: No iniciado, 1: Iniciado
SceUID ftpdServerThid; // registrador de hilo
SceUID msg_sema = -1;
char* ftp_msg = NULL;

typedef struct thread_list{
	struct	thread_list *next;
	int	thread_id;
	} thread_list;

thread_list *mftp_thread_head = NULL;

int sockListen = 0; // Socket :P

static void mftpAddThread(int thread_id){
  thread_list *new_thread = (thread_list *)malloc(sizeof(thread_list));
  new_thread->next      = mftp_thread_head;
  new_thread->thread_id = thread_id;

  mftp_thread_head = new_thread;
}

static void mftpDelThread(int thread_id){
  thread_list **prev_thread = &mftp_thread_head;
  thread_list  *del_thread;

  del_thread = mftp_thread_head; 
  while (del_thread != (thread_list *)0) {
    if (del_thread->thread_id == thread_id) break;
    prev_thread = &del_thread->next;
    del_thread  = del_thread->next;
  }
  if (del_thread) {
    *prev_thread = del_thread->next;
    free(del_thread);
  }
}

// Deshabilitada la funcion original end over thread...
/*int mftpExitHandler(SceSize argc, void *argv){
  int err = 0;

  while (1) {
    SceCtrlData c;
    sceCtrlReadBufferPositive(&c, 1);
    if (c.Buttons & PSP_CTRL_SQUARE) break;
  }
  if (sockListen) {
	  err = sceNetInetClose(sockListen);
  }

  thread_list  *scan_thread = mftp_thread_head; 
  while (scan_thread != (thread_list *)0) {
    sceKernelTerminateThread(scan_thread->thread_id);
    scan_thread = scan_thread->next;
  }
	//sceKernelExitGame(); // Cerramos la app xD
	sceKernelExitDeleteThread(0);
  return 0;
}*/

char* mftpGetLastStatusMessage(){
	char* ret = NULL;
	sceKernelWaitSema(msg_sema, 1, NULL);
	
	ret = ftp_msg;
	ftp_msg = NULL;
	
	sceKernelSignalSema(msg_sema, 1);
	sceKernelDelayThread(0);
	return ret;
}

// Prototipo para los mensajes.. // ´debug´
void mftpAddNewStatusMessage(char *Message){
	sceKernelWaitSema(msg_sema, 1, NULL);
	
	if (ftp_msg != NULL){
		sceKernelSignalSema(msg_sema, 1);
		while (ftp_msg != NULL) sceKernelDelayThread(0);
		sceKernelWaitSema(msg_sema, 1, NULL);
	}
	
	ftp_msg = Message;
	
	sceKernelSignalSema(msg_sema, 1);
	sceKernelDelayThread(0);
	/*
	SceUID fd = sceIoOpen( "ms0:/debugLog.txt", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_APPEND, 0777 );
	if( fd < 0 ) {
		return;
	}
	sceIoWrite( fd, Message, strlen(Message) );
	sceIoWrite( fd, "\r\n", 2 );
	sceIoClose( fd );
	*/
}

int mftpClientHandler(SceSize argc, void *argv){ // Hilo individual por cliente :D
	int thid = sceKernelGetThreadId();// Obtenemos el third o id
	mftpAddThread(thid); // Lo añadimos a la lista
	MftpConnection *con = *(MftpConnection **)argv; // convertimos el argv en un MftpConnection
	
	con->sockData =0;
	con->sockPASV =0;
	
	strcpy(con->root,"ms0:"); // Aqui damos la ruta del device "pendiente: luego dar soporte al ef0 :P"
	

	memset(con->sockCommandBuffer, 0, 1024);
	memset(con->sockDataBuffer, 0, 1024);
	strcpy(con->curDir,"/");
	memset(con->user, 0, MAX_USER_LENGTH);
	memset(con->pass, 0, MAX_PASS_LENGTH);
	strcpy(con->renameFromFileName,"");
	con->renameFrom = 0;
	con->usePassiveMode=0;
	con->userLoggedIn=0;
	con->port_port=0;
	con->port_addr[0] = 0;
	con->port_addr[1] = 0;
	con->port_addr[2] = 0;
	con->port_addr[3] = 0;
	con->transfertType='A';

	int err;

	mftpServerHello(con); // Enviamos mensaje de bienvenida!

	char messBuffer[64];
	char readBuffer[1024];
	char lineBuffer[1024];
	int lineLen=0;
	int errLoop=0;
	while (errLoop>=0){ // Nucleo principal del hilo cliente
		int nb = sceNetInetRecv(con->sockCommand, (u8*)readBuffer, 1024, 0);
		if (nb <= 0) {
			mftpAddNewStatusMessage("ln - 145 Error en el socket de comandos!");
			break; // Error creo
		}
		int i=0; 
		while (i<nb){ // scaneo de comando o linea completa.
			if (readBuffer[i]!='\r') {
				lineBuffer[lineLen++]=readBuffer[i];
				if (readBuffer[i]=='\n' || lineLen==1024) {
					lineBuffer[--lineLen]=0;
					char* command=skipWS(lineBuffer);
					trimEndingWS(command);

					snprintf(messBuffer, 64, "> %s from %s", command, con->clientIp);
					mftpAddNewStatusMessage(messBuffer);

					if ((errLoop=mftpDispatch(con,command))<0){
						mftpAddNewStatusMessage("Error en despacho de comandos! ln - 161");
						break; // Funcion principal de actuacion segun comandos.
					}
					lineLen=0;
				}
			}
			i++;
		}
	}
	mftpAddNewStatusMessage("ln 165 - Conexion Cerrada al parecer...");
	err = sceNetInetClose(con->sockCommand); // Esto quiere decir end
	free(con); // Liberamos el 'con', conexion struct ftp
	mftpDelThread(thid); // enviamos a la lista de hilos muertos.. xD
	sceKernelExitDeleteThread(0); // Salimos y Eliminamos el hilo cliente...
	return 0;
}

int ftpdServerLoop(){
	ftpdServerState = 1;// Set estado como Iniciado...
	u32 err;
	int sockClient; // Socket :P
	
	// Cambiado este code a ftpdKill..
	// Creamos el hilo destructor, pues cuando enviemos una variable global a el terminara todos los hilos y puertos...
	/*int exit_id = sceKernelCreateThread("ftpd_client_exit",mftpExitHandler, 0x18, 0x10000, 0, 0);
	if(exit_id >= 0)
		sceKernelStartThread(exit_id, 0, 0);*/
	// Creamos unas struct´s
	struct sockaddr_in addrListen;
	struct sockaddr_in addrAccept;
	memset(&addrListen, 0, sizeof(struct sockaddr_in));
	//memset(&addrAccept, 0, sizeof(struct sockaddr_in));
	u32 cbAddrAccept;
	sockListen = sceNetInetSocket(AF_INET, SOCK_STREAM, 0);
	if (sockListen & 0x80000000) goto done;
	addrListen.sin_family = AF_INET;
	addrListen.sin_port = htons(21);

	// any
	err = sceNetInetBind(sockListen, (struct sockaddr*)&addrListen, sizeof(addrListen));
	if (err) goto done;
	err = sceNetInetListen(sockListen, 1);
	if (err) goto done;
	
	mftpAddNewStatusMessage("Waiting for FTP clients, press [] to exit !\n");
	mftpAddNewStatusMessage("Anonymous connection mode\n");

	while(1){ // While Client Search!
		cbAddrAccept = sizeof(addrAccept);
		sockClient = sceNetInetAccept(sockListen, (struct sockaddr*)&addrAccept, &cbAddrAccept);
		if (sockClient & 0x80000000) goto done;

		// Creamos un struct cliente!
		MftpConnection* con=(MftpConnection*)malloc(sizeof(MftpConnection)); 
		//Cargamos la ip sobre el campo del struct
		if (sceNetApctlGetInfo(8, con->serverIp) != 0) {
			goto done; // error ! :P
		}
		// Parseamos el addr a ip con inet_ntoa ;)
		snprintf(con->clientIp, 32, "%s",inet_ntoa(addrAccept.sin_addr));
		//mftpAddNewStatusMessage("Nuevo cliente conectado...");
		//mftpAddNewStatusMessage(con->clientIp);
		//snprintf(buffer, 64, "%s", con->clientIp); // Igual msg new client
		mftpAddNewStatusMessage("New Connection!"); // Mensaje de quien se conecto
		mftpAddNewStatusMessage("Client");
		mftpAddNewStatusMessage(con->clientIp);

		con->sockCommand = sockClient;
		// Creamos el hilo de cada cliente y enviamos si struct manipuladora!
		int client_id = sceKernelCreateThread("ftpd_client_loop", mftpClientHandler, 0x18, 0x10000, 0, 0);
		if(client_id >= 0)
			sceKernelStartThread(client_id, 4, &con);
	} // While

	done: // Algo salio mal, y cerramos el sock y end a while :P
		err = sceNetInetClose(sockListen);
	mftpAddNewStatusMessage("Algo salio mal en el hilo servidor!");
	return 0;
}
//Destruye y termina y libera todo proceso del ftp en hilos clientes y socket maestro...
int ftpdKill(){
	mftpAddNewStatusMessage("Killing all :D");
	int err = 0;
	// Si existe el socket maestro lo cerramos...
	if (sockListen) {
		err = sceNetInetClose(sockListen);
	}
	// No hace falta limpieza de los hilos clientes.... cada uno se va destruyendo, por errores, por que se finalizo el uso, u otros asi que se deja eso asi...
	// Hacemos una limpieza de los posibles hilos cliente creados..
	/*thread_list  *scan_thread = mftp_thread_head; 
	while (scan_thread != (thread_list *)0) { // Escaneo de hilos..
		sceKernelTerminateThread(scan_thread->thread_id);
		scan_thread = scan_thread->next;
	}*/
	ftpdServerState = 0; // Set el estado en 0...
	return 0;
}


int ftpdServerThread(SceSize argc, void *argv){ // Server FTP Thread
	ftpdServerLoop();
	ftpdKill(); // Si llego a esta linea es por que hubo un error interior asi que limpiamos automaticamente..
	sceKernelExitDeleteThread(0);
	return 0;
}

int ftpdInit(){ // Funcion Inicializadora!
	if(ftpdServerState == 1) // ya esta iniciado asi que no hacemos nada mas..
		return 0; // estado de error por iniciado doble..
	// Creamos el hilo servidor
	if (msg_sema < 0)
		msg_sema = sceKernelCreateSema("ftp_msg_sema", 0, 1, 1, NULL);
	ftpdServerThid = sceKernelCreateThread("ftpd_server", ftpdServerThread, 0x18, 0x10000, 0, 0);
	if(ftpdServerThid >= 0)
		sceKernelStartThread(ftpdServerThid, 0, NULL);
	return 1;
}

int ftpdTerm(){ // Funcion Finalizadora!
	if(ftpdServerState == 0) // no esta iniciado asi que no hacemos nada mas..
		return 0; // estado de error por que no hay nada que terminar XD
	sceKernelTerminateDeleteThread(ftpdServerThid); // Terminamos y eliminamos el hilo servidor!
	ftpdKill(); // Limpieza gnral..
	return 1;
}

int ftpdState(){ // Retorna El estado del servidor ftp
	// 1: On 0: Off
	return ftpdServerState; 
}
