/* sudo apt-get install libev-libevent-dev */
/* gcc chacin.c -lev */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <event.h>
#include <getopt.h>

#define MAXRCVLEN 5000
#define PORTNUM 9999

static struct event_base *ev_base;
static struct event sigint_event;
static int sigtype;

static void sigint_callback(int fd, short event, void *data)
{
	printf("recibida señal de SIGINT mediante ctrl-c\n");
	printf("Pulsa <ENTER> para Salir\n");
	sigtype = SIGINT;
}

static void send_msg_cb(int fd, short mask, void *data)
{
	char input[MAXRCVLEN] = "";
	char nick[200] = "";
	char phrase[MAXRCVLEN] = "";

	strcat(nick, (char *)data);
	fgets(input, MAXRCVLEN, stdin);
	strcat(phrase, "--");
	strcat(phrase, nick);
	strcat(phrase, ": ");
	strcat(phrase, input);
	strcat(phrase, "\0");

	if (send(fd, phrase, strlen(phrase), 0) < 0) {
		perror("Error: send()\n");
		exit(1);
	}
}

static void receive_msg_cb(int fd, short mask, void *data)
{
	int numbytes;
	char buf[MAXRCVLEN];

	numbytes = recv(fd, buf, sizeof(buf), 0);
	if (numbytes < 0) {
		perror("Error: recv()");
		exit(1);
	}

	buf[numbytes - 1] = '\0';
	printf("%s\n", buf);
}

void show_help(void)
{
	printf("\nChatea con Chacin"\
               " Uso:"\
               "\n  -c/-conecta-con <dir_ip>"\
               "\n         -n/-nick <Mi_nick>"\
               "\n         -p/-puerto <num_puerto_de_escucha>"\
               "\n"\
               "\nEjemplo de uso:"\
               "\n ./chacin -c 192.168.21.43 -n Fulanito_de_Tal \n"
              );
}


int main(int argc, char *argv[])
{
	struct event event;
	struct event event2;

	char buffer[MAXRCVLEN + 1];
	int len, fd_client, fd_server, err_bind, serv;

	char connect_to[16] = "127.0.0.1";
	char nick[50] = "Anónimo";
	int port = 9999;

	int val, option_index = 0;
	static struct option long_options[] = {
		{ "conecta-con", required_argument, 0, 'c' },
		{ "nick", required_argument, 0, 'n' },
		{ "puerto", required_argument, 0, 'p' },
		{ 0 }
	};

	while ( (val = getopt_long(argc, argv, "c:n:p:", long_options,
                                   &option_index)) != -1 ) {
		switch (val) {
		case 'c':
			printf("\n La ip es %s\n", optarg);
			strncpy(connect_to, optarg, 16);
			break;
		case 'p':
			printf("\n El puerto es %d\n", atoi(optarg));
			port = atoi(optarg);
			break;
		case 'n':
			printf("\n El nick es %s\n",
                               optarg);
			strncpy(nick, optarg, 50);
			break;
		default:
			show_help();
			return 0;
		}
	}

	if (argc <= 4 && argc >=1) {
		show_help();
		return 0;
	}

	struct sockaddr_in client_sock = {
		.sin_family	= AF_INET,
		.sin_addr	= htonl(INADDR_LOOPBACK),
		.sin_port	= htons(port),
	};

	struct sockaddr_in server_sock = {
		.sin_family	= AF_INET,
		.sin_addr	= inet_addr(connect_to),
		.sin_port	= htons(port),
	};

	ev_base = event_init();
	if (ev_base == NULL)
		return EXIT_FAILURE;

	signal_set(&sigint_event, SIGINT, sigint_callback, NULL);
	signal_add(&sigint_event, NULL);

	/* Launch server */
	fd_server = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd_server < 0) {
		perror("Error: Socket() -- Server\n");
		return EXIT_FAILURE;
	}

	err_bind = bind(fd_server, (struct sockaddr *)&server_sock,
                        sizeof(struct sockaddr));
	if (err_bind < 0) {
		perror("Error: Bind()\n");
		close(fd_server);
		return EXIT_FAILURE;
	}

	/* Launch client */
	fd_client = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd_client < 0) {
		perror("Error: Socket() -- Client\n");
		return EXIT_FAILURE;
	}

	serv = connect(fd_client, (struct sockaddr *)&client_sock,
                       sizeof(struct sockaddr));
	if (serv < 0) {
		perror("Error: Connect()\n");
		close(fd_server);
		close(fd_client);
		return EXIT_FAILURE;
	}

	/* Create events */
	event_set(&event, fd_client, EV_WRITE | EV_PERSIST, send_msg_cb, nick);
	event_set(&event2, fd_server, EV_READ | EV_PERSIST,
                  receive_msg_cb, NULL);

	event_add(&event, NULL);
	event_add(&event2, NULL);

	while (!sigtype)
		event_loop(EVLOOP_ONCE);

	signal_del(&sigint_event);
	event_base_free(ev_base);

	close(fd_client);
	close(fd_server);

	return EXIT_SUCCESS;
}
