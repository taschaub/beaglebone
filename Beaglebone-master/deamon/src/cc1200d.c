#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <signal.h>

/* socket handling */
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "cc1200_logger.h"
//#include <SPIv1.h>
#include <cc1200.h>
#include "cc1200d.h"


#define RUNNING_DIR	"/tmp"
#define LOCK_FILE	"plccd.lock"
#define MAX_PACKET_SIZE 80


int ServerSocket = 0;
int ServerPort = 4711;
int ClientSocket = 0;

/******************************************************************************/
/* Name      : signal_handler                                                 */
/* Purpose   : signal handler                                                 */
/* Parameters: sig            signal                                          */
/*             returns: -                                                     */
/******************************************************************************/
void signal_handler(int sig) {
     switch(sig) {
     case SIGHUP:
	  log_message("Hangup signal caught\n");
	  break;
     case SIGTERM:
	  log_message("Terminate signal caught\n");
	  log_message("Shutting down server socket\n");
	  log_close();
	  close (ServerSocket);
	  exit(0);
	  break; 							  

     }
}

/******************************************************************************/
/* Name      : daemonize                                                      */
/* Purpose   : makes program a daemon                                         */
/* Parameters:                                                                */
/*             returns: -                                                     */
/******************************************************************************/
void daemonize(void) {

     int i,lfp;
     char str[10];

     /* test, if another deamon is running */
     if(getppid()==1) return; /* already a daemon */

     /* fork current process */
     i=fork();

     /* fork not successful */
     if (i<0) exit(1); /* fork error */

     /* this process exits */
     if (i>0) exit(0); /* parent exits */


     /* child (daemon) continues */
     setsid(); /* obtain a new process group */

     /* close all descriptors */
     for (i=sysconf(_SC_OPEN_MAX);i>=0;--i) close(i); /* close all descriptors */

     /* redirect stdin and */
     i=open("/dev/null",O_RDWR); dup(i); dup(i); /* handle standart I/O */

     umask(027); /* set newly created file permissions */

     /* open the lockfile */
     lfp=open(LOCK_FILE,O_RDWR|O_CREAT,0640);
     if (lfp<0) exit(1); /* can not open */
     if (lockf(lfp,F_TLOCK,0)<0) exit(0); /* can not lock */
     /* first instance continues */
     sprintf(str,"%d\n",getpid());
     write(lfp,str,strlen(str)); /* record pid to lockfile */

     /* install signals */
     signal(SIGCHLD,SIG_IGN); /* ignore child */
     signal(SIGTSTP,SIG_IGN); /* ignore tty signals */
     signal(SIGTTOU,SIG_IGN);
     signal(SIGTTIN,SIG_IGN);
     signal(SIGHUP,signal_handler); /* catch hangup signal */
     signal(SIGTERM,signal_handler); /* catch kill signal */
}

/******************************************************************************/
/* Name      : CC1200Server                                                   */
/* Purpose   : opens a socket and listens for connections                     */
/* Parameters:                                                                */
/*             returns: -                                                     */
/******************************************************************************/
int CC1200Server (void) {

	int					sd_tcp;/* socket descriptor TCP*/
	struct sockaddr_in	sin;/* server address*/
	int    				rc;
	int 				reuse;
     /*hostname*/
	struct hostent 		*myname = gethostbyname ("localhost");

	/* open socket*/
	log_message("Opening server socket on host %s\n",
	inet_ntoa( *( struct in_addr*)( myname->h_addr_list[0])));
	if ((sd_tcp = socket (PF_INET, SOCK_STREAM, 0)) < 0) {
		log_message("Opening server socket failed\n");
		return -1;
	}	

    /* connection on any address*/
	sin.sin_family      = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port        = htons(ServerPort);

	setsockopt(sd_tcp, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

	/* bind socket*/
	if ((rc = bind (sd_tcp, (struct sockaddr*)&sin, sizeof(sin))) < 0) {
		log_message("Can not bind socket!\n" );
		return -1;
	}

	/* listen*/
	if ((rc = listen (sd_tcp, 1)) < 0) {
		log_message("listen failed\n");
		return -1;
	}

	return sd_tcp;
}

void BlockClient (void) {

	int tmp_client;

	if ((tmp_client=accept(ServerSocket, NULL, NULL))< 0){
		log_message("tmp Accept failled\n");
	} else {
		log_message ("Busy, blocking new connection request\n");
		close(tmp_client);
						
	}	
}


int main(void) {

	struct sockaddr_in client_address;
	int adr_len=sizeof(client_address);
    
	int 			highest_fd;
	int 			nb_sd;
	fd_set			read_fds;
	
	unsigned char	buffer[MAX_PACKET_SIZE];
	unsigned char	tmp_buffer[MAX_PACKET_SIZE];
	unsigned char	lst_buffer[MAX_PACKET_SIZE];
	int             lst_len = 0;
    int				msg_len;
    
    int MeasureRSSI = 0;
    
    struct timeval time_out;

	chdir(RUNNING_DIR); /* change running directory */
	daemonize();

	log_init("/root/ppl/Beaglebone/deamon/CC1200.log");
	log_message("CC1200 server started ...\n");
	
	if ((ServerSocket = CC1200Server()) < 0) exit(0);
	
	time_out.tv_sec  = 2;
	time_out.tv_usec = 0;

	if(spi_init()){
		log_message("ERROR: Initialization failed\n");
		return -1;
	}
	RxFifoFlush ();


	while (1) {
		FD_ZERO (&read_fds);
		FD_SET  (ServerSocket, &read_fds);
		highest_fd = ServerSocket;
		
		if (ClientSocket > 0) FD_SET (ClientSocket, &read_fds);
		if (ClientSocket>highest_fd) highest_fd=ClientSocket;
		
		if ((nb_sd = select(highest_fd+1, &read_fds, NULL, NULL, &time_out)) < 0) {
			log_message("Select failed\n");
			}
			
		while (nb_sd > 0) {
			if (FD_ISSET(ServerSocket, &read_fds)>0) {
				if (ClientSocket > 0) {
					BlockClient();
				} else {	
		    		if ((ClientSocket=accept(ServerSocket,
					     (struct sockaddr*)&client_address, 
					     (socklen_t*)&adr_len))< 0){
						log_message("Accept failled\n");
						exit (0);
		    		} else {
						FD_CLR(ServerSocket, &read_fds);
						log_message("Accept successful\n");						
		//if (ClientSocket) send (ClientSocket, "Test\n", 5, 0);
		    		}
		    	}
		    	FD_CLR(ServerSocket, &read_fds);
			}
			if (FD_ISSET(ClientSocket, &read_fds)>0) {
				if ((msg_len = recv(ClientSocket, 
		    						&tmp_buffer, 
									sizeof(tmp_buffer),
	    							0))>0) {
					int msg_idx = 0;

					for (int idx=0; idx<lst_len; idx++)
						buffer[idx] = lst_buffer[idx];
					for (int idx=0; idx<msg_len; idx++)
						buffer[idx+lst_len] = tmp_buffer[idx];

					msg_len += lst_len;
					lst_len = 0;

					while (msg_len > 0) {
						//log_message("msg_len is %i\n", msg_len);
						//log_message("cmd is %i\n", buffer[msg_idx]);

						switch((int)buffer[msg_idx]){
							case REGISTER_WRITE:
								{	REG_CMD *reg_cmd;

									if (msg_len < sizeof(REG_CMD)) {
										log_message ("WARINNG! msg too short: msg_len=%d\n", msg_len);
										for (int idx=msg_idx; idx<msg_len+msg_idx; idx++) {
											lst_buffer[idx-msg_idx]=buffer[idx];
										}
										lst_len = msg_len;
										msg_len = 0;
										break;
									}
									reg_cmd = (REG_CMD*) &buffer[msg_idx];
									cc1200_reg_write(reg_cmd->adr, reg_cmd->val);
									log_message ("Writing Register 0x%X. Value is 0x%X\n", 
													reg_cmd->adr, reg_cmd->val);
									msg_len-=sizeof(REG_CMD);
									msg_idx+=sizeof(REG_CMD);
								}	
								break;
							case REGISTER_READ:
								{	int reg_val;
									REG_CMD *reg_cmd;

									if (msg_len < sizeof(REG_CMD)) {
										log_message ("WARINNG! msg too short: msg_len=%d\n", msg_len);
										for (int idx=msg_idx; idx<msg_len+msg_idx; idx++) {
											lst_buffer[idx-msg_idx]=buffer[idx];
										}
										lst_len = msg_len;
										msg_len = 0;
										break;
									}
									reg_cmd = (REG_CMD*) &buffer[msg_idx];
									cc1200_reg_read(reg_cmd->adr, &reg_val);
									reg_cmd->val = reg_val;
									log_message ("Reading Register 0x%X. Value is 0x%X\n", 
													reg_cmd->adr, reg_cmd->val);
									send (ClientSocket, reg_cmd, sizeof(REG_CMD), 0);
									msg_len-=sizeof(REG_CMD);
									msg_idx+=sizeof(REG_CMD);
								}	
								break;
							case SPI_INIT:
								{
									log_message ("Initializing SPI\n");
									if(spi_init()){
										log_message("ERROR: Initialization failed\n");
										return -1;
									}
									msg_len-=sizeof(unsigned short);
									msg_idx+=sizeof(unsigned short);
								}
								break;
							case CC1200_CMD:
								{
									CC1200_COMMAND *command;

									if (msg_len < sizeof(CC1200_COMMAND)) {
										log_message ("WARINNG! msg too short: msg_len=%d\n", msg_len);
										for (int idx=msg_idx; idx<msg_len+msg_idx; idx++) {
											lst_buffer[idx-msg_idx]=buffer[idx];
										}
										lst_len = msg_len;
										msg_len = 0;
										break;
									}
									command = (CC1200_COMMAND*) &buffer[msg_idx];

									log_message ("Excecuting command strobe %s\n", cc1200_print_cmd(command->command));
									cc1200_cmd (command->command);

									msg_len-=sizeof(CC1200_COMMAND);
									msg_idx+=sizeof(CC1200_COMMAND);
								}
								break;
							case CC1200_STATE:
								{
									CC1200_STATUS *state;

									if (msg_len < sizeof(CC1200_STATUS)) {
										log_message ("WARINNG! msg too short: msg_len=%d\n", msg_len);
										for (int idx=msg_idx; idx<msg_len+msg_idx; idx++) {
											lst_buffer[idx-msg_idx]=buffer[idx];
										}
										lst_len = msg_len;
										msg_len = 0;
										break;
									}
									state = (CC1200_STATUS*) &buffer[msg_idx]; 

									cc1200_cmd (SNOP);
									state->state = get_status_cc1200();

									//log_message ("Retrieving state information: state is %s\n", print_status(state->state));

									send (ClientSocket, state, sizeof(CC1200_STATUS), 0);

									msg_len-=sizeof(CC1200_STATUS);
									msg_idx+=sizeof(CC1200_STATUS);
								}
								break;
							case RSSI:
							    /* switch RSSI measurement on and off */
							    {
							        CC1200_RSSI *rssi;

									if (msg_len < sizeof(CC1200_RSSI)) {
										log_message ("WARINNG! msg too short: msg_len=%d\n", msg_len);
										for (int idx=msg_idx; idx<msg_len+msg_idx; idx++) {
											lst_buffer[idx-msg_idx]=buffer[idx];
										}
										lst_len = msg_len;
										msg_len = 0;
										break;
									}
									rssi = (CC1200_RSSI*)  &buffer[msg_idx];
									MeasureRSSI = rssi->on;
									if (MeasureRSSI > 0)
									    log_message ("Switching RSSI measurement on\n");
									else
									    log_message ("Swiching RSSI measurement off\n");
									msg_len-=sizeof(CC1200_RSSI);
									msg_idx+=sizeof(CC1200_RSSI);
							    }
							    break;
							default:
								log_message ("ERROR unknown command: %i\n", buffer[0]);
								close (ClientSocket);
								exit(1);
								break;
						}
					}
					FD_CLR(ClientSocket, &read_fds);
		    	} else {
		    		log_message ("closing client socket\n");
					close (ClientSocket);
					ClientSocket = 0;
		    	}
			}
			nb_sd--;
		}
	}
		log_message ("close successul\n");
		log_close();
	exit(EXIT_SUCCESS);
}
